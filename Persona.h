#pragma once
#include <iostream>
#include <string>
#include <regex>
#include <stdexcept>

class persona {
protected:
    std::string nombres;
    std::string apellidos;
    std::string direccion;
    std::string telefono;
    int genero;
    std::string fecha_nacimiento;
    std::string fecha_inicio_labores;

public:
    persona() : genero(0) {}

    persona(std::string nom, std::string ape, std::string dir, std::string tel, int gen,
        std::string fnac, std::string fini) {
        setNombres(nom);
        setApellidos(ape);
        setDireccion(dir);
        setTelefono(tel);
        setGenero(gen);
        setFechaNacimiento(fnac);
        setFechaInicioLabores(fini);
    }

    // ---- VALIDACIONES GENERALES ----
    void setNombres(const std::string& nom) {
        if (nom.empty() || nom.length() > 60) {
            throw std::invalid_argument("El nombre es obligatorio y no debe superar los 60 caracteres.");
        }
        std::regex patron_nom("^[a-zA-Z ]+$");
        if (!std::regex_match(nom, patron_nom)) {
            throw std::invalid_argument("El nombre solo debe contener letras y espacios.");
        }
        nombres = nom;
    }

    void setApellidos(const std::string& ape) {
        if (ape.empty() || ape.length() > 60) {
            throw std::invalid_argument("El apellido es obligatorio y no debe superar los 60 caracteres.");
        }
        std::regex patron_ape("^[a-zA-Z ]+$");
        if (!std::regex_match(ape, patron_ape)) {
            throw std::invalid_argument("El apellido solo debe contener letras y espacios.");
        }
        apellidos = ape;
    }

    void setDireccion(const std::string& dir) {
        if (dir.empty() || dir.length() > 80) { // Limitado según el VARCHAR(80) de tu base de datos
            throw std::invalid_argument("La direccion es obligatoria y no debe superar los 80 caracteres.");
        }
        direccion = dir;
    }

    void setTelefono(const std::string& tel) {
        std::regex patron_tel("^\\d{8}$");
        if (!std::regex_match(tel, patron_tel)) {
            throw std::invalid_argument("El telefono debe contener exactamente 8 digitos numericos.");
        }
        telefono = tel;
    }

    void setGenero(int gen) {
        if (gen != 0 && gen != 1) {
            throw std::invalid_argument("El genero debe ser 0 (Femenino) o 1 (Masculino).");
        }
        genero = gen;
    }

    void setFechaNacimiento(const std::string& fnac) {
        std::regex patron_fecha("^\\d{4}-\\d{2}-\\d{2}$");
        if (!std::regex_match(fnac, patron_fecha)) {
            throw std::invalid_argument("Formato de fecha de nacimiento invalido (Use AAAA-MM-DD).");
        }
        int anio = std::stoi(fnac.substr(0, 4));
        if (anio < 1920 || anio > 2026) {
            throw std::invalid_argument("La fecha de nacimiento no es valida.");
        }
        fecha_nacimiento = fnac;
    }

    void setFechaInicioLabores(const std::string& fini) {
        std::regex patron_fecha("^\\d{4}-\\d{2}-\\d{2}$");
        if (!std::regex_match(fini, patron_fecha)) {
            throw std::invalid_argument("Formato de fecha de inicio de labores invalido (Use AAAA-MM-DD).");
        }
        fecha_inicio_labores = fini;
    }

    // ---- GETTERS ----
    std::string getNombres() const { return nombres; }
    std::string getApellidos() const { return apellidos; }
    std::string getDireccion() const { return direccion; }
    std::string getTelefono() const { return telefono; }
    int getGenero() const { return genero; }
    std::string getFechaNacimiento() const { return fecha_nacimiento; }
    std::string getFechaInicioLabores() const { return fecha_inicio_labores; }

    virtual void leer() = 0;
    virtual ~persona() {}
};