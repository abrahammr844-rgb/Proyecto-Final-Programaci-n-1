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
    // ==== CONSTRUCTORES ====

    // Constructor vacio
    Empleado() : persona() {
        id_empleado = 0;
        id_puesto = 0;
        cui = "";
        fecha_ingreso = "";
    }

    // Constructor con parametros 
    Empleado(std::string nom, std::string ape, std::string dir, std::string tel, std::string c, int gen, std::string fn,  
        int id_emp, int puesto, std::string f_inicio, std::string f_ingreso)
        : persona(nom, ape, dir, tel, gen, fn, f_inicio) {
        // ALINEACION CORRECTA: Ahora si pasamos los parametros en el orden exacto que persona.h pide.

        id_empleado = id_emp;
        id_puesto = puesto;
        cui = c; // Guardamos el CUI de 13 digitos
        fecha_ingreso = f_ingreso;
    }

    // ==== ENCAPSULAMIENTO (GETTERS Y SETTERS) ====

    void setIdEmpleado(int id) { id_empleado = id; }
    int getIdEmpleado() const { return id_empleado; }

    void setIdPuesto(int puesto) { id_puesto = puesto; }
    int getIdPuesto() const { return id_puesto; }

    void setCui(std::string c) { cui = c; }
    std::string getCui() const { return cui; }

    void setFechaIngreso(std::string f) { fecha_ingreso = f; }
    std::string getFechaIngreso() const { return fecha_ingreso; }



    bool buscarPorId(int id) {
        ConexionBD cn; cn.abrir_conexion();
        MYSQL* conn = cn.getConector();
        bool existe = false;

        if (conn) {
            std::string query = "SELECT id_empleado, nombres, apellidos FROM empleados WHERE id_empleado = ?";
            MYSQL_STMT* stmt = mysql_stmt_init(conn);

            if (stmt && mysql_stmt_prepare(stmt, query.c_str(), query.length()) == 0) {
                MYSQL_BIND bind_in[1]; std::memset(bind_in, 0, sizeof(bind_in));
                bind_in[0].buffer_type = MYSQL_TYPE_LONG;
                bind_in[0].buffer = &id;
                mysql_stmt_bind_param(stmt, bind_in);

                if (mysql_stmt_execute(stmt) == 0) {
                    mysql_stmt_store_result(stmt);
                    if (mysql_stmt_num_rows(stmt) > 0) {
                        int res_id; char res_nom[61]; char res_ape[61]; unsigned long l_n, l_a;
                        MYSQL_BIND bind_out[3]; std::memset(bind_out, 0, sizeof(bind_out));

                        bind_out[0].buffer_type = MYSQL_TYPE_LONG;   bind_out[0].buffer = &res_id;
                        bind_out[1].buffer_type = MYSQL_TYPE_STRING; bind_out[1].buffer = res_nom; bind_out[1].buffer_length = 60; bind_out[1].length = &l_n;
                        bind_out[2].buffer_type = MYSQL_TYPE_STRING; bind_out[2].buffer = res_ape; bind_out[2].buffer_length = 60; bind_out[2].length = &l_a;

                        mysql_stmt_bind_result(stmt, bind_out);
                        if (mysql_stmt_fetch(stmt) == 0) {
                            id_empleado = res_id; res_nom[l_n] = '\0'; res_ape[l_a] = '\0';
                            nombres = std::string(res_nom);
                            apellidos = std::string(res_ape);
                            existe = true;
                        }
                    }
                }
            }
            if (stmt) mysql_stmt_close(stmt);
        }
        cn.cerrar_conexion();
        return existe;
    }

    bool autenticar(int id, const std::string& token_cui) {
        ConexionBD cn;
        cn.abrir_conexion();
        MYSQL* conn = cn.getConector();

        bool acceso_concedido = false;

        if (!conn) {
            std::cout << "\n Error: No se pudo conectar a la BD." << std::endl;
            cn.cerrar_conexion();
            return false;
        }

        std::cout << "\n========== DIAGNOSTICO DE ACCESO ==========" << std::endl;
        std::cout << "-> Buscando ID ingresado: " << id << std::endl;
        std::cout << "-> CUI en teclado: '" << token_cui << "' (Longitud: " << token_cui.length() << " caracteres)" << std::endl;

        // 1. Intento de inicio de sesion seguro 
        std::string q = "SELECT id_empleado FROM empleados WHERE id_empleado = ? AND cui = ?";
        MYSQL_STMT* stmt = mysql_stmt_init(conn);

        if (stmt && mysql_stmt_prepare(stmt, q.c_str(), q.length()) == 0) {
            MYSQL_BIND bind[2];
            std::memset(bind, 0, sizeof(bind));

            int id_temp = id;
            unsigned long cui_len = token_cui.length();

            bind[0].buffer_type = MYSQL_TYPE_LONG;
            bind[0].buffer = &id_temp;

            bind[1].buffer_type = MYSQL_TYPE_STRING;
            bind[1].buffer = (char*)token_cui.c_str();
            bind[1].buffer_length = token_cui.length();
            bind[1].length = &cui_len;

            mysql_stmt_bind_param(stmt, bind);

            if (mysql_stmt_execute(stmt) == 0) {
                mysql_stmt_store_result(stmt);
                if (mysql_stmt_num_rows(stmt) > 0) {
                    acceso_concedido = true;
                }
            }
            mysql_stmt_close(stmt);
        }

        
        if (!acceso_concedido) {
            std::cout << "\n El login fallo. Verificando existencia en MySQL..." << std::endl;
            std::string q_check = "SELECT id_empleado, nombres, cui FROM empleados WHERE id_empleado = " + std::to_string(id);

            if (mysql_query(conn, q_check.c_str()) == 0) {
                MYSQL_RES* res = mysql_store_result(conn);
                if (res) {
                    MYSQL_ROW fila = mysql_fetch_row(res);
                    if (fila) {
                        std::string db_nom = fila[1];
                        std::string db_cui = fila[2];
                        std::cout << ">> [¡EL ID SI EXISTE!] El empleado es: " << db_nom << std::endl;
                        std::cout << "   - CUI en Base de Datos: '" << db_cui << "' (Longitud: " << db_cui.length() << ")" << std::endl;
                        std::cout << "   - CUI que tu escribiste: '" << token_cui << "' (Longitud: " << token_cui.length() << ")" << std::endl;
                    }
                    else {
                        std::cout << ">> [CAUSA] No existe ningun empleado con el ID " << id << " en la base de datos." << std::endl;
                    }
                    mysql_free_result(res);
                }
            }
        }

        std::cout << "===========================================\n" << std::endl;
        system("pause");

        cn.cerrar_conexion();
        return acceso_concedido;
    }

    void leer() override {
      
    }
};

#endif