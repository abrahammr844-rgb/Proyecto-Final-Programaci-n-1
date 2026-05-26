#pragma once

#include <mysql.h>
#include <vector>
#include <string>
#include <iostream>
#include <cstring>
#include <iomanip> // Para dar formato estético a los números decimales
#include "ConexionBD.h"

// Struct actualizado con soporte para nombres y consistencia en Double
struct Carrito {
    int id_producto;
    std::string nombre_producto;
    int cantidad;
    double precio_unitario;
};

class Venta {
private:
    // Método privado para validar existencias en el servidor antes de la transacción
    int obtenerExistencia(MYSQL* conector, int id_prod) {
        std::string query = "SELECT existencia FROM productos WHERE id_producto = " + std::to_string(id_prod);
        if (mysql_query(conector, query.c_str()) == 0) {
            MYSQL_RES* res = mysql_store_result(conector);
            if (res) {
                MYSQL_ROW fila = mysql_fetch_row(res);
                int stock = (fila && fila[0] != NULL) ? std::stoi(fila[0]) : 0;
                mysql_free_result(res);
                return stock;
            }
        }
        return 0;
    }

public:
    // ==== REQUERIMIENTO 3: AUTOMATIZACIÓN DE CORRELATIVOS ====
    int obtenerSiguienteNumeroFactura(char serie) {
        ConexionBD cn;
        cn.abrir_conexion();
        MYSQL* conn = cn.getConector();
        int siguiente_no = 1; // Si la tabla está vacía, empieza en 1

        if (conn) {
            // Buscamos el número más alto registrado en esa serie específica
            std::string query = "SELECT MAX(no_factura) FROM ventas WHERE serie = '" + std::string(1, serie) + "'";
            if (mysql_query(conn, query.c_str()) == 0) {
                MYSQL_RES* res = mysql_store_result(conn);
                if (res) {
                    MYSQL_ROW fila = mysql_fetch_row(res);
                    if (fila && fila[0] != NULL) {
                        siguiente_no = std::stoi(fila[0]) + 1; // Sumamos 1 al correlativo anterior
                    }
                    mysql_free_result(res);
                }
            }
        }
        cn.cerrar_conexion();
        return siguiente_no;
    }

    // ==== PROCESAMIENTO TRANSACCIONAL DE LA VENTA ====
    void ejecutarVenta(int id_c, int id_e, char serie, int no, std::vector<Carrito> det) {
        if (det.empty() || id_c <= 0 || id_e <= 0 || no <= 0) {
            std::cerr << "xx Error: Parametros transaccionales invalidos o Carrito vacio. xx" << std::endl;
            return;
        }

        ConexionBD cn;
        cn.abrir_conexion();
        MYSQL* conn = cn.getConector();

        if (conn) {
            // 1. Verificación de última hora del Stock (Seguridad Concurrente)
            for (auto const& item : det) {
                int stock_actual = obtenerExistencia(conn, item.id_producto);
                if (stock_actual < item.cantidad) {
                    std::cerr << "\nxx Venta cancelada de forma automatica: Stock insuficiente en servidor para el ID: "
                        << item.id_producto << " (" << item.nombre_producto << ") xx" << std::endl;
                    cn.cerrar_conexion();
                    return;
                }
            }

            // 2. Iniciamos bloque transaccional ACID
            mysql_query(conn, "START TRANSACTION");

            // 3. Insertar el maestro de la venta (La Factura)
            std::string q_venta = "INSERT INTO ventas(no_factura, serie, fecha_factura, id_cliente, id_empleado, fecha_ingreso) VALUES (?, ?, NOW(), ?, ?, NOW())";
            MYSQL_STMT* stmt_venta = mysql_stmt_init(conn);

            if (!stmt_venta || mysql_stmt_prepare(stmt_venta, q_venta.c_str(), q_venta.length()) != 0) {
                if (stmt_venta) mysql_stmt_close(stmt_venta);
                mysql_query(conn, "ROLLBACK");
                cn.cerrar_conexion();
                return;
            }

            MYSQL_BIND bind_v[4];
            std::memset(bind_v, 0, sizeof(bind_v));
            std::string serie_str(1, serie);

            bind_v[0].buffer_type = MYSQL_TYPE_LONG;   bind_v[0].buffer = &no;
            bind_v[1].buffer_type = MYSQL_TYPE_STRING; bind_v[1].buffer = (char*)serie_str.c_str(); bind_v[1].buffer_length = 1;
            bind_v[2].buffer_type = MYSQL_TYPE_LONG;   bind_v[2].buffer = &id_c;
            bind_v[3].buffer_type = MYSQL_TYPE_LONG;   bind_v[3].buffer = &id_e;

            mysql_stmt_bind_param(stmt_venta, bind_v);
            if (mysql_stmt_execute(stmt_venta) != 0) {
                mysql_stmt_close(stmt_venta);
                mysql_query(conn, "ROLLBACK");
                cn.cerrar_conexion();
                return;
            }

            // Recuperamos el ID autogenerado de la factura para enlazar los detalles
            int id_v = (int)mysql_insert_id(conn);
            mysql_stmt_close(stmt_venta);

            // 4. Preparar statements para el Detalle y el Descuento de Stock
            std::string q_detalle = "INSERT INTO ventas_detalle(id_venta, id_producto, cantidad, precio_unitario) VALUES (?, ?, ?, ?)";
            std::string q_stock = "UPDATE productos SET existencia = existencia - ? WHERE id_producto = ?";

            MYSQL_STMT* stmt_det = mysql_stmt_init(conn);
            MYSQL_STMT* stmt_stk = mysql_stmt_init(conn);

            if (!stmt_det || !stmt_stk ||
                mysql_stmt_prepare(stmt_det, q_detalle.c_str(), q_detalle.length()) != 0 ||
                mysql_stmt_prepare(stmt_stk, q_stock.c_str(), q_stock.length()) != 0) {

                if (stmt_det) mysql_stmt_close(stmt_det);
                if (stmt_stk) mysql_stmt_close(stmt_stk);
                mysql_query(conn, "ROLLBACK");
                cn.cerrar_conexion();
                return;
            }

            // 5. Procesar el lote de productos
            bool error = false;
            for (auto const& i : det) {
                MYSQL_BIND bind_d[4];
                std::memset(bind_d, 0, sizeof(bind_d));
                double precio = i.precio_unitario;
                int cant = i.cantidad;
                int prod_id = i.id_producto;

                bind_d[0].buffer_type = MYSQL_TYPE_LONG;   bind_d[0].buffer = &id_v;
                bind_d[1].buffer_type = MYSQL_TYPE_LONG;   bind_d[1].buffer = &prod_id;
                bind_d[2].buffer_type = MYSQL_TYPE_LONG;   bind_d[2].buffer = &cant;
                bind_d[3].buffer_type = MYSQL_TYPE_DOUBLE; bind_d[3].buffer = &precio; // Cambiado a DOUBLE

                mysql_stmt_bind_param(stmt_det, bind_d);
                if (mysql_stmt_execute(stmt_det) != 0) { error = true; break; }

                MYSQL_BIND bind_s[2];
                std::memset(bind_s, 0, sizeof(bind_s));
                bind_s[0].buffer_type = MYSQL_TYPE_LONG;   bind_s[0].buffer = &cant;
                bind_s[1].buffer_type = MYSQL_TYPE_LONG;   bind_s[1].buffer = &prod_id;

                mysql_stmt_bind_param(stmt_stk, bind_s);
                if (mysql_stmt_execute(stmt_stk) != 0) { error = true; break; }
            }

            mysql_stmt_close(stmt_det);
            mysql_stmt_close(stmt_stk);

            // 6. Cierre de la operación (Consolidación o Aborto)
            if (!error) {
                mysql_query(conn, "COMMIT");
                std::cout << ">> [BASE DE DATOS] Datos guardados y stock actualizado exitosamente." << endl;
                std::cout << "==========================================================" << endl;
            }
            else {
                mysql_query(conn, "ROLLBACK");
                std::cerr << "xx [ERROR CRITICO] Fallo el procesamiento del detalle. Rollback aplicado. xx" << std::endl;
            }
        }
        cn.cerrar_conexion();
    }
};