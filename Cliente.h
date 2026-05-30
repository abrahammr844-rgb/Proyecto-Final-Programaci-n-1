#pragma once
#include <mysql.h>
#include <iostream>
#include <string>
#include <regex>
#include <stdexcept>
#include <cstring>
#include "Persona.h"
#include "ConexionBD.h"

class Cliente : public persona {
private:
    std::string nit;
    std::string correo;
    int id_cliente;

public:
    Cliente() : persona(), id_cliente(0) {}

    Cliente(std::string nom, std::string ape, std::string dir, std::string tel, int gen,
        std::string n, std::string mail, int id = 0)
        : persona(nom, ape, dir, tel, gen, "2000-01-01", "2000-01-01") {
        setNit(n);
        setCorreo(mail);
        id_cliente = id;
    }

    void setNit(const std::string& n) {
        std::regex patron_nit("^([0-9]{3,10}-?[0-9kK]|cf|CF|c/f|C/F)$");
        if (!std::regex_match(n, patron_nit)) {
            throw std::invalid_argument("El formato del NIT es incorrecto. Use un numero valido o 'CF'.");
        }
        nit = (n == "cf" || n == "c/f" || n == "c/f") ? "CF" : n;
    }

    void setCorreo(const std::string& mail) {
        if (mail.empty()) { correo = ""; return; }
        if (mail.length() > 45) { throw std::invalid_argument("El correo no debe superar los 45 caracteres."); }
        std::regex patron_correo("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
        if (!std::regex_match(mail, patron_correo)) {
            throw std::invalid_argument("El formato del correo electrónico es incorrecto.");
        }
        correo = mail;
    }

    int getId() const { return id_cliente; }
    std::string getNit() const { return nit; }
    std::string getCorreo() const { return correo; }

    bool crear() {
        ConexionBD cn; cn.abrir_conexion();
        MYSQL* conn = cn.getConector();
        if (conn) {
            MYSQL_STMT* stmt = mysql_stmt_init(conn);
            if (!stmt) { cn.cerrar_conexion(); return false; }

            std::string query = "INSERT INTO clientes(nombres, apellidos, nit, genero, telefono, correo_electronico, fecha_ingreso) VALUES (?, ?, ?, ?, ?, ?, NOW())";
            if (mysql_stmt_prepare(stmt, query.c_str(), query.length()) != 0) { mysql_stmt_close(stmt); cn.cerrar_conexion(); return false; }

            MYSQL_BIND bind[6]; std::memset(bind, 0, sizeof(bind));
            std::string nom = getNombres(); std::string ape = getApellidos(); int gen = getGenero(); std::string tel = getTelefono();

            bind[0].buffer_type = MYSQL_TYPE_STRING; bind[0].buffer = (char*)nom.c_str();  bind[0].buffer_length = nom.length();
            bind[1].buffer_type = MYSQL_TYPE_STRING; bind[1].buffer = (char*)ape.c_str();  bind[1].buffer_length = ape.length();
            bind[2].buffer_type = MYSQL_TYPE_STRING; bind[2].buffer = (char*)nit.c_str();  bind[2].buffer_length = nit.length();
            bind[3].buffer_type = MYSQL_TYPE_LONG;   bind[3].buffer = &gen;
            bind[4].buffer_type = MYSQL_TYPE_STRING; bind[4].buffer = (char*)tel.c_str();  bind[4].buffer_length = tel.length();
            bind[5].buffer_type = MYSQL_TYPE_STRING; bind[5].buffer = (char*)correo.c_str(); bind[5].buffer_length = correo.length();

            mysql_stmt_bind_param(stmt, bind);
            int status = mysql_stmt_execute(stmt);
            mysql_stmt_close(stmt);
            cn.cerrar_conexion();
            return (status == 0);
        }
        return false;
    }

    bool buscarNit(std::string n) {
        ConexionBD cn; cn.abrir_conexion();
        MYSQL* conn = cn.getConector();
        bool existe = false;

        if (conn) {
            std::string query = "SELECT id_cliente, nombres, apellidos FROM clientes WHERE nit = ?";
            MYSQL_STMT* stmt = mysql_stmt_init(conn);

            if (stmt && mysql_stmt_prepare(stmt, query.c_str(), query.length()) == 0) {
                MYSQL_BIND bind_in[1]; std::memset(bind_in, 0, sizeof(bind_in));
                bind_in[0].buffer_type = MYSQL_TYPE_STRING; bind_in[0].buffer = (char*)n.c_str(); bind_in[0].buffer_length = n.length();
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
                            id_cliente = res_id; res_nom[l_n] = '\0'; res_ape[l_a] = '\0';
                            nombres = std::string(res_nom); apellidos = std::string(res_ape); nit = n;
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

    void leer() override {
        ConexionBD cn; cn.abrir_conexion();
        if (cn.getConector()) {
            std::string query = "SELECT id_cliente, nit, nombres, apellidos FROM clientes ORDER BY id_cliente ASC";
            mysql_query(cn.getConector(), query.c_str());
            MYSQL_RES* res = mysql_store_result(cn.getConector());
            MYSQL_ROW fila;
            std::cout << "\n--- LISTA DE CLIENTES EN SISTEMA ---" << std::endl;
            while ((fila = mysql_fetch_row(res))) {
                std::cout << "ID: " << fila[0] << " | NIT: " << fila[1] << " | " << fila[2] << " " << fila[3] << std::endl;
            }
            mysql_free_result(res);
        }
        cn.cerrar_conexion();
    }
};
