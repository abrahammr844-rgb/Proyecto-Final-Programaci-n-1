#pragma once
#include <mysql.h>
#include <iostream>
#include <string>
#include "ConexionBD.h"
using namespace std;

class Producto {
public:
    void leer() {
        ConexionBD cn; cn.abrir_conexion();
        if (cn.getConector()) {
            string q = "SELECT id_producto, producto, precio_venta, existencia FROM productos";
            mysql_query(cn.getConector(), q.c_str());
            MYSQL_RES* res = mysql_store_result(cn.getConector());
            MYSQL_ROW fila;
            cout << "\nID | PRODUCTO | PRECIO | STOCK" << endl;
            while (fila = mysql_fetch_row(res)) { cout << fila[0] << " | " << fila[1] << " | " << fila[2] << " | " << fila[3] << endl; }
            mysql_free_result(res);
        }
        cn.cerrar_conexion();
    }

    bool verificarExistencia(int id, int cant) {
        ConexionBD cn; cn.abrir_conexion();
        bool hay = false;
        string q = "SELECT existencia FROM productos WHERE id_producto = " + to_string(id);
        mysql_query(cn.getConector(), q.c_str());
        MYSQL_RES* res = mysql_store_result(cn.getConector());
        if (res) {
            MYSQL_ROW fila = mysql_fetch_row(res);
            if (fila && stoi(fila[0]) >= cant) hay = true;
        }
        mysql_free_result(res); cn.cerrar_conexion();
        return hay;
    }
};