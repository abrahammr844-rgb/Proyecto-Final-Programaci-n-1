#pragma once
#include "Persona.h"
#include "ConexionBD.h"
#include <mysql.h>

class Empleado : public persona {
private:
    string cui;
    int id_puesto;
public:
    Empleado() {}
    Empleado(string nom, string ape, string dir, string tel, int gen, string fi, string c, int p)
        : persona(nom, ape, dir, tel, gen, fi) {
        cui = c; id_puesto = p;
    }

    void leer() override {
        ConexionBD cn; cn.abrir_conexion();
        if (cn.getConector()) {
            string q = "SELECT e.id_empleado, e.nombres, p.puesto FROM empleados e INNER JOIN puestos p ON e.id_puesto = p.id_puesto";
            mysql_query(cn.getConector(), q.c_str());
            MYSQL_RES* res = mysql_store_result(cn.getConector());
            MYSQL_ROW fila;
            while (fila = mysql_fetch_row(res)) { cout << fila[0] << " | " << fila[1] << " | " << fila[2] << endl; }
            mysql_free_result(res);
        }
        cn.cerrar_conexion();
    }
};