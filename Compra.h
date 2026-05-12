#pragma once
#include <mysql.h>
#include <vector>
#include <string>
#include <iostream>
#include "ConexionBD.h"

using namespace std;

struct LoteCompra {
    int id_producto;
    int cantidad;
    float precio_costo;
};

class Compra {
public:
    void ejecutarCompra(int no, int id_p, vector<LoteCompra> det) {
        ConexionBD cn;
        cn.abrir_conexion();

        if (cn.getConector()) {
            // 1. Iniciamos la transacción
            mysql_query(cn.getConector(), "BEGIN");

            string q = "INSERT INTO compras(no_orden_compra, id_proveedor, fecha_orden, fecha_ingreso) VALUES ("
                + to_string(no) + "," + to_string(id_p) + ", NOW(), NOW())";

            if (mysql_query(cn.getConector(), q.c_str()) == 0) {
                // Obtenemos el ID de la compra recién creada
                int id_c = (int)mysql_insert_id(cn.getConector());
                bool error_en_detalle = false;

                for (auto const& i : det) {
                    // Insertar en compras_detalle
                    string d = "INSERT INTO compras_detalle(id_compra, id_producto, cantidad, precio_costo_unitario) VALUES ("
                        + to_string(id_c) + "," + to_string(i.id_producto) + "," + to_string(i.cantidad) + "," + to_string(i.precio_costo) + ")";

                    // Actualizar existencia sumando lo comprado
                    string u = "UPDATE productos SET existencia = existencia + " + to_string(i.cantidad) +
                        " WHERE id_producto = " + to_string(i.id_producto);

                    // Validar si alguna de las dos consultas falla
                    if (mysql_query(cn.getConector(), d.c_str()) != 0 || mysql_query(cn.getConector(), u.c_str()) != 0) {
                        error_en_detalle = true;
                        cout << "xxxx Error en producto ID " << i.id_producto << ": " << mysql_error(cn.getConector()) << " xxxx" << endl;
                        break;
                    }
                }

                if (!error_en_detalle) {
                    mysql_query(cn.getConector(), "COMMIT");
                    cout << ">>>> COMPRA EXITOSA Y STOCK ACTUALIZADO <<<<" << endl;
                }
                else {
                    mysql_query(cn.getConector(), "ROLLBACK");
                    cout << "xxxx Venta cancelada debido a un error en el detalle xxxx" << endl;
                }
            }
            else {
                cout << "xxxx Error en encabezado: " << mysql_error(cn.getConector()) << " xxxx" << endl;
                mysql_query(cn.getConector(), "ROLLBACK");
            }
        }
        else {
            cout << "xxxx Error de conexion xxxx" << endl;
        }
        cn.cerrar_conexion();
    }
};