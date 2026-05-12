#pragma once
#include <mysql.h>
#include <vector>
#include <string>
#include "ConexionBD.h"
using namespace std;

struct Carrito {
    int id_producto;
    int cantidad;
    float precio_unitario;
};

class Venta {
public:
    void ejecutarVenta(int id_c, int id_e, char serie, int no, vector<Carrito> det) {
        ConexionBD cn; cn.abrir_conexion();
        if (cn.getConector()) {
            mysql_query(cn.getConector(), "BEGIN");
            string q = "INSERT INTO ventas(no_factura, serie, fecha_factura, id_cliente, id_empleado, fecha_ingreso) VALUES (" + to_string(no) + ",'" + serie + "', NOW(), " + to_string(id_c) + ", " + to_string(id_e) + ", NOW())";
            if (mysql_query(cn.getConector(), q.c_str()) == 0) {
                int id_v = (int)mysql_insert_id(cn.getConector());
                for (auto const& i : det) {
                    string d = "INSERT INTO ventas_detalle(id_venta, id_producto, cantidad, precio_unitario) VALUES (" + to_string(id_v) + "," + to_string(i.id_producto) + "," + to_string(i.cantidad) + "," + to_string(i.precio_unitario) + ")";
                    string u = "UPDATE productos SET existencia = existencia - " + to_string(i.cantidad) + " WHERE id_producto = " + to_string(i.id_producto);
                    mysql_query(cn.getConector(), d.c_str());
                    mysql_query(cn.getConector(), u.c_str());
                }
                mysql_query(cn.getConector(), "COMMIT");
                cout << "VENTA EXITOSA" << endl;
            }
            else { mysql_query(cn.getConector(), "ROLLBACK"); }
        }
        cn.cerrar_conexion();
    }
    void leerVentas() { /* Similar a leer clientes */ }
};