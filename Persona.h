#pragma once
#include <iostream>
#include <string>
using namespace std;

class persona {
protected:
    string nombres, apellidos, direccion, fecha_ingreso, telefono;
    int genero = 0;
public:
    persona() {}
    persona(string nom, string ape, string dir, string tel, int gen, string fi) {
        nombres = nom; apellidos = ape; direccion = dir; telefono = tel; genero = gen; fecha_ingreso = fi;
    }
    virtual void leer() = 0;
};