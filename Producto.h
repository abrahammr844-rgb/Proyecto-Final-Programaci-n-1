#pragma once
#include <mysql.h>
#include <iostream>
#include <string>
#include <stdexcept>
#include <cstring>
#include "ConexionBD.h"

class Producto {
private:
    int id_producto;
    std::string nombre_producto;
    int id_marca;
    std::string descripcion;
    std::string imagen;
    double precio_costo;
    double precio_venta;
    int existencia;

public:
    Producto() : id_producto(0), id_marca(0), precio_costo(0.0), precio_venta(0.0), existencia(0) {}

    Producto(std::string prod, int marca, std::string desc, std::string img, double costo, double venta, int stock) {
        setProducto(prod);
        setIdMarca(marca);
        setDescripcion(desc);
        setImagen(img);
        setPrecios(costo, venta);
        setExistencia(stock);
    }

    void setProducto(const std::string& prod) {
        if (prod.empty() || prod.length() > 50) { throw std::invalid_argument("Nombre de producto no valido (Max 50)."); }
        nombre_producto = prod;
    }

    void setIdMarca(int marca) {
        if (marca <= 0) { throw std::invalid_argument("Marca no valida."); }
        id_marca = marca;
    }

    void setDescripcion(const std::string& desc) {
        if (desc.length() > 100) { throw std::invalid_argument("Descripcion demasiado larga (Max 100)."); }
        descripcion = desc;
    }

    void setImagen(const std::string& img) {
        if (img.length() > 30) { throw std::invalid_argument("Nombre de ruta de imagen excedido (Max 30)."); }
        imagen = img;
    }

    void setPrecios(double costo, double venta) {
        if (costo <= 0.0 || venta <= 0.0) { throw std::invalid_argument("Los precios deben ser mayores a cero."); }
        if (venta < costo) { throw std::invalid_argument("Alerta de Negocio: El precio de venta genera perdidas respecto al costo."); }
        precio_costo = costo;
        precio_venta = venta;
    }

    void setExistencia(int stock) {
        if (stock < 0) { throw std::invalid_argument("La existencia no puede ser negativa."); }
        existencia = stock;
    }

    std::string getProducto() const { return nombre_producto; }
    double getPrecioVenta() const { return precio_venta; }
    int getExistencia() const { return existencia; }

    bool crear() {
        ConexionBD cn; cn.abrir_conexion();
        MYSQL* conn = cn.getConector();
        if (conn) {
            MYSQL_STMT* stmt = mysql_stmt_init(conn);
            if (!stmt) { cn.cerrar_conexion(); return false; }

            std::string q = "INSERT INTO productos (producto, id_marca, descripcion, imagen, precio_costo, precio_venta, existencia, fecha_ingreso) VALUES (?, ?, ?, ?, ?, ?, ?, NOW())";
            if (mysql_stmt_prepare(stmt, q.c_str(), q.length()) != 0) { mysql_stmt_close(stmt); cn.cerrar_conexion(); return false; }

            MYSQL_BIND bind[7]; std::memset(bind, 0, sizeof(bind));
            bind[0].buffer_type = MYSQL_TYPE_STRING; bind[0].buffer = (char*)nombre_producto.c_str(); bind[0].buffer_length = nombre_producto.length();
            bind[1].buffer_type = MYSQL_TYPE_LONG;   bind[1].buffer = &id_marca;
            bind[2].buffer_type = MYSQL_TYPE_STRING; bind[2].buffer = (char*)descripcion.c_str();   bind[2].buffer_length = descripcion.length();
            bind[3].buffer_type = MYSQL_TYPE_STRING; bind[3].buffer = (char*)imagen.c_str();        bind[3].buffer_length = imagen.length();
            bind[4].buffer_type = MYSQL_TYPE_DOUBLE; bind[4].buffer = &precio_costo;
            bind[5].buffer_type = MYSQL_TYPE_DOUBLE; bind[5].buffer = &precio_venta;
            bind[6].buffer_type = MYSQL_TYPE_LONG;   bind[6].buffer = &existencia;

            mysql_stmt_bind_param(stmt, bind);
            int status = mysql_stmt_execute(stmt);
            mysql_stmt_close(stmt); cn.cerrar_conexion();
            return (status == 0);
        }
        return false;
    }

    void leer() {
        ConexionBD cn; cn.abrir_conexion();
        if (cn.getConector()) {
            std::string q = "SELECT p.id_producto, p.producto, m.marca, p.precio_venta, p.existencia FROM productos p INNER JOIN marcas m ON p.id_marca = m.id_marca ORDER BY p.id_producto ASC";
            mysql_query(cn.getConector(), q.c_str());
            MYSQL_RES* res = mysql_store_result(cn.getConector());
            MYSQL_ROW fila;
            std::cout << "\n=== INVENTARIO DE PRODUCTOS DISPONIBLES ===" << std::endl;
            while ((fila = mysql_fetch_row(res))) {
                std::cout << fila[0] << " | " << fila[1] << " (" << fila[2] << ") | Precio: Q" << fila[3] << " | Stock: " << fila[4] << " un." << std::endl;
            }
            mysql_free_result(res);
        }
        cn.cerrar_conexion();
    }

    bool verificarExistencia(int id, int cant) {
        ConexionBD cn; cn.abrir_conexion();
        bool hay = false; MYSQL* conn = cn.getConector();
        if (conn) {
            std::string q = "SELECT existencia FROM productos WHERE id_producto = " + std::to_string(id);
            if (mysql_query(conn, q.c_str()) == 0) {
                MYSQL_RES* res = mysql_store_result(conn);
                if (res) {
                    MYSQL_ROW fila = mysql_fetch_row(res);
                    if (fila && std::stoi(fila[0]) >= cant) hay = true;
                    mysql_free_result(res);
                }
            }
        }
        cn.cerrar_conexion();
        return hay;
    }
};