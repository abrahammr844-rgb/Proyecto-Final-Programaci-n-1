#pragma once
#ifndef _HAS_STD_BYTE
#define _HAS_STD_BYTE 0
#endif
#include <mysql.h>
#include <iostream>

using namespace std;

class ConexionBD {
private:
    MYSQL* conector;
public:
    void abrir_conexion() {
        conector = mysql_init(0);
       
        conector = mysql_real_connect(conector, "localhost", "root", "/Dexitotexito1234*", "sistema_supermercado", 3306, NULL, 0);
    }
    MYSQL* getConector() { return conector; }
    void cerrar_conexion() {
        if (conector) mysql_close(conector);
    }
};