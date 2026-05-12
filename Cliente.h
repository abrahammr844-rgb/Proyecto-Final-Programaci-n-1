#pragma once
#include <mysql.h>
#include <iostream>
#include <string>
#include <iomanip>
#include "Persona.h"
#include "ConexionBD.h"

class Cliente : public persona {
private:
    string nit, correo;
    int id_cliente = 0;
public:
    Cliente() {}
    Cliente(string nom, string ape, string dir, string tel, int gen, string fi, string n, string mail, int id = 0)
        : persona(nom, ape, dir, tel, gen, fi) {
        nit = n; correo = mail; id_cliente = id;
    }

    int getId() { return id_cliente; }

    void crear() {
        ConexionBD cn;
        cn.abrir_conexion();
        if (cn.getConector()) {
            string query = "INSERT INTO clientes(nombres,apellidos,nit,genero,telefono,correo_electronico,fecha_ingreso) VALUES ('" + nombres + "','" + apellidos + "','" + nit + "'," + to_string(genero) + ",'" + telefono + "','" + correo + "',NOW())";
            mysql_query(cn.getConector(), query.c_str());
        }
        cn.cerrar_conexion();
    }

    bool buscarNit(string n) {
        ConexionBD cn; cn.abrir_conexion();
        bool existe = false;
        string query = "SELECT id_cliente, nombres, apellidos FROM clientes WHERE nit = '" + n + "'";
        mysql_query(cn.getConector(), query.c_str());
        MYSQL_RES* res = mysql_store_result(cn.getConector());
        if (res && mysql_num_rows(res) > 0) {
            MYSQL_ROW fila = mysql_fetch_row(res);
            id_cliente = stoi(fila[0]);
            nombres = fila[1];
            apellidos = fila[2];
            existe = true;
        }
        mysql_free_result(res); cn.cerrar_conexion();
        return existe;
    }

    void leer() override {
        ConexionBD cn; cn.abrir_conexion();
        if (cn.getConector()) {
            string query = "SELECT id_cliente, nit, nombres, apellidos FROM clientes";
            mysql_query(cn.getConector(), query.c_str());
            MYSQL_RES* res = mysql_store_result(cn.getConector());
            MYSQL_ROW fila;
            cout << "\n--- LISTA DE CLIENTES ---" << endl;
            while (fila = mysql_fetch_row(res)) { cout << fila[0] << " | " << fila[1] << " | " << fila[2] << " " << fila[3] << endl; }
            mysql_free_result(res);
        }
        cn.cerrar_conexion();
    }
};