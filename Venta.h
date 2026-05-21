#pragma once
#include <mysql.h>
#include <vector>
#include <string>
#include <iostream>
#include <cstring>
#include "ConexionBD.h"

struct Carrito {
    int id_producto;
    int cantidad;
    float precio_unitario;
};

class Venta {
private:
    int obtenerExistencia(MYSQL* conector, int id_prod) {
        std::string query = "SELECT existencia FROM productos WHERE id_producto = " + std::to_string(id_prod);
        if (mysql_query(conector, query.c_str()) == 0) {
            MYSQL_RES* res = mysql_store_result(conector);
            if (res) {
                MYSQL_ROW fila = mysql_fetch_row(res);
                int stock = fila ? std::stoi(fila[0]) : 0;
                mysql_free_result(res);
                return stock;
            }
        }
        return 0;
    }

public:
    void ejecutarVenta(int id_c, int id_e, char serie, int no, std::vector<Carrito> det) {
        if (det.empty() || id_c <= 0 || id_e <= 0 || no <= 0) {
            std::cerr << "Parametros transaccionales invalidos o Carrito vacio." << std::endl;
            return;
        }

        ConexionBD cn; cn.abrir_conexion();
        MYSQL* conn = cn.getConector();

        if (conn) {
            for (auto const& item : det) {
                int stock_actual = obtenerExistencia(conn, item.id_producto);
                if (stock_actual < item.cantidad) {
                    std::cerr << "Venta cancelada. Stock insuficiente para el ID: " << item.id_producto << std::endl;
                    cn.cerrar_conexion(); return;
                }
            }

            mysql_query(conn, "START TRANSACTION");

            std::string q_venta = "INSERT INTO ventas(no_factura, serie, fecha_factura, id_cliente, id_empleado, fecha_ingreso) VALUES (?, ?, NOW(), ?, ?, NOW())";
            MYSQL_STMT* stmt_venta = mysql_stmt_init(conn);

            if (!stmt_venta || mysql_stmt_prepare(stmt_venta, q_venta.c_str(), q_venta.length()) != 0) {
                mysql_query(conn, "ROLLBACK"); cn.cerrar_conexion(); return;
            }

            MYSQL_BIND bind_v[4]; std::memset(bind_v, 0, sizeof(bind_v));
            std::string serie_str(1, serie);
            bind_v[0].buffer_type = MYSQL_TYPE_LONG;   bind_v[0].buffer = &no;
            bind_v[1].buffer_type = MYSQL_TYPE_STRING; bind_v[1].buffer = (char*)serie_str.c_str(); bind_v[1].buffer_length = 1;
            bind_v[2].buffer_type = MYSQL_TYPE_LONG;   bind_v[2].buffer = &id_c;
            bind_v[3].buffer_type = MYSQL_TYPE_LONG;   bind_v[3].buffer = &id_e;

            mysql_stmt_bind_param(stmt_venta, bind_v);
            if (mysql_stmt_execute(stmt_venta) != 0) {
                mysql_stmt_close(stmt_venta); mysql_query(conn, "ROLLBACK"); cn.cerrar_conexion(); return;
            }

            int id_v = (int)mysql_insert_id(conn);
            mysql_stmt_close(stmt_venta);

            std::string q_detalle = "INSERT INTO ventas_detalle(id_venta, id_producto, cantidad, precio_unitario) VALUES (?, ?, ?, ?)";
            std::string q_stock = "UPDATE productos SET existencia = existencia - ? WHERE id_producto = ?";
            MYSQL_STMT* stmt_det = mysql_stmt_init(conn);
            MYSQL_STMT* stmt_stk = mysql_stmt_init(conn);

            if (mysql_stmt_prepare(stmt_det, q_detalle.c_str(), q_detalle.length()) != 0 || mysql_stmt_prepare(stmt_stk, q_stock.c_str(), q_stock.length()) != 0) {
                mysql_query(conn, "ROLLBACK"); cn.cerrar_conexion(); return;
            }

            bool error = false;
            for (auto const& i : det) {
                MYSQL_BIND bind_d[4]; std::memset(bind_d, 0, sizeof(bind_d));
                float precio = i.precio_unitario; int cant = i.cantidad; int prod_id = i.id_producto;

                bind_d[0].buffer_type = MYSQL_TYPE_LONG;  bind_d[0].buffer = &id_v;
                bind_d[1].buffer_type = MYSQL_TYPE_LONG;  bind_d[1].buffer = &prod_id;
                bind_d[2].buffer_type = MYSQL_TYPE_LONG;  bind_d[2].buffer = &cant;
                bind_d[3].buffer_type = MYSQL_TYPE_FLOAT; bind_d[3].buffer = &precio;

                mysql_stmt_bind_param(stmt_det, bind_d);
                if (mysql_stmt_execute(stmt_det) != 0) { error = true; break; }

                MYSQL_BIND bind_s[2]; std::memset(bind_s, 0, sizeof(bind_s));
                bind_s[0].buffer_type = MYSQL_TYPE_LONG;  bind_s[0].buffer = &cant;
                bind_s[1].buffer_type = MYSQL_TYPE_LONG;  bind_s[1].buffer = &prod_id;

                mysql_stmt_bind_param(stmt_stk, bind_s);
                if (mysql_stmt_execute(stmt_stk) != 0) { error = true; break; }
            }

            mysql_stmt_close(stmt_det); mysql_stmt_close(stmt_stk);

            if (!error) {
                mysql_query(conn, "COMMIT");
                std::cout << "\n>> [EXITO] Transaccion guardada de manera conforme. Factura No: " << no << std::endl;
            }
            else {
                mysql_query(conn, "ROLLBACK");
                std::cerr << "Error critico de procesamiento. Rollback aplicado." << std::endl;
            }
        }
        cn.cerrar_conexion();
    }
};