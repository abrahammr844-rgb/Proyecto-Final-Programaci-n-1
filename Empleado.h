#ifndef EMPLEADO_H
#define EMPLEADO_H
#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <mysql.h>
#include "Persona.h"
#include "ConexionBD.h"

class Empleado : public persona {
private:
    int id_empleado;
    int id_puesto;
    std::string cui;
    std::string fecha_ingreso;

public:
    Empleado() : persona() { id_empleado = 0; id_puesto = 0; cui = ""; fecha_ingreso = ""; }

    Empleado(std::string nom, std::string ape, std::string dir, std::string tel, std::string c, int gen, std::string fn,
        int id_emp, int puesto, std::string f_inicio, std::string f_ingreso)
        : persona(nom, ape, dir, tel, gen, fn, f_inicio) {
        id_empleado = id_emp; id_puesto = puesto; cui = c; fecha_ingreso = f_ingreso;
    }

    // ==== 1. MÉTODO BUSCAR (Corrigiendo el C2039) ====
    bool buscarPorId(int id) {
        ConexionBD cn; cn.abrir_conexion();
        MYSQL* conn = cn.getConector();
        bool existe = false;
        if (conn) {
            std::string q = "SELECT nombres, apellidos FROM empleados WHERE id_empleado = " + std::to_string(id);
            if (mysql_query(conn, q.c_str()) == 0) {
                MYSQL_RES* res = mysql_store_result(conn);
                if (MYSQL_ROW fila = mysql_fetch_row(res)) {
                    nombres = fila[0]; apellidos = fila[1]; existe = true;
                }
                mysql_free_result(res);
            }
        }
        cn.cerrar_conexion();
        return existe;
    }

    // ==== 2. MÉTODO DE AUTENTICACIÓN (Lógica directa para evitar errores) ====
    bool autenticar(int id, std::string token_cui) {
        ConexionBD cn;
        cn.abrir_conexion();
        MYSQL* conn = cn.getConector();
        bool acceso_concedido = false;

        if (!conn) return false;

        // Consulta directa usando TRIM para ignorar espacios invisibles
        std::string q = "SELECT id_empleado FROM empleados WHERE id_empleado = " + std::to_string(id) +
            " AND TRIM(cui) = '" + token_cui + "' AND id_puesto = 1";

        if (mysql_query(conn, q.c_str()) == 0) {
            MYSQL_RES* res = mysql_store_result(conn);
            if (res) {
                if (mysql_num_rows(res) > 0) acceso_concedido = true;
                mysql_free_result(res);
            }
        }

        // Si falla, mostramos el diagnóstico para ver qué está pasando
        if (!acceso_concedido) {
            std::cout << "\n[!] Fallo de acceso. Diagnóstico:" << std::endl;
            std::string q_check = "SELECT nombres, cui FROM empleados WHERE id_empleado = " + std::to_string(id);
            if (mysql_query(conn, q_check.c_str()) == 0) {
                MYSQL_RES* res = mysql_store_result(conn);
                if (MYSQL_ROW fila = mysql_fetch_row(res)) {
                    std::cout << "Nombre BD: " << fila[0] << "\nCUI en BD: '" << fila[1] << "' (Long: " << strlen(fila[1]) << ")" << std::endl;
                    std::cout << "Tu CUI:    '" << token_cui << "' (Long: " << token_cui.length() << ")" << std::endl;
                }
                mysql_free_result(res);
            }
            system("pause");
        }

        cn.cerrar_conexion();
        return acceso_concedido;
    }

    void leer() override {}
};
#endif
