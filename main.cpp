#ifndef _HAS_STD_BYTE
#define _HAS_STD_BYTE 0
#endif

#include <mysql.h>
#include <iostream>
#include <string>
#include <regex>
#include <vector>

#include "Cliente.h"
#include "Empleado.h"
#include "Producto.h"
#include "Venta.h"
#include "Compra.h"

using namespace std;

// --- FUNCIONES DE VALIDACIÓN ---
bool v_nit(string n) {
    return regex_match(n, regex("^[0-9]+-[0-9kK]{1}$")) || n == "C/F" || n == "c/f";
}

bool v_letras(string s) {
    return regex_match(s, regex("^[a-zA-Z ]+$"));
}

// --- MÓDULO DE VENTAS (SALIDA) ---
void menu_ventas() {
    Venta v;
    Cliente c;
    Producto p;
    vector<Carrito> mi_carrito;
    string nit_buscado;
    int id_emp, id_prod, cant, no_fact;
    char serie, agregar_mas;
    float precio_p;

    system("cls");
    cout << "===== MODULO DE FACTURACION (VENTA) =====" << endl;
    cout << "Ingrese NIT del Cliente: ";
    cin >> nit_buscado;

    if (!c.buscarNit(nit_buscado)) {
        cout << "xx Cliente no registrado. Por favor, creelo en el menu de clientes. xx" << endl;
        system("pause");
        return;
    }

    cout << "ID del Empleado: "; cin >> id_emp;
    cout << "Serie de Factura: "; cin >> serie;
    cout << "No. Factura: "; cin >> no_fact;

    do {
        p.leer();
        cout << "\nID del Producto: "; cin >> id_prod;
        cout << "Cantidad: "; cin >> cant;

        if (p.verificarExistencia(id_prod, cant)) {
            cout << "Precio Venta Q: "; cin >> precio_p;
            mi_carrito.push_back({ id_prod, cant, precio_p });
            cout << ">> Agregado al carrito." << endl;
        }
        else {
            cout << "xx Error: Stock insuficiente. xx" << endl;
        }
        cout << "¿Desea agregar otro? (s/n): "; cin >> agregar_mas;
    } while (agregar_mas == 's' || agregar_mas == 'S');

    if (!mi_carrito.empty()) {
        v.ejecutarVenta(c.getId(), id_emp, serie, no_fact, mi_carrito);
    }
    system("pause");
}

// --- MÓDULO DE COMPRAS (ENTRADA/ABASTECIMIENTO) ---
void menu_compras() {
    Compra comp;
    Producto p;
    vector<LoteCompra> carrito_c;
    int id_prov, no_ord, id_prod, cant;
    float costo;
    char mas;

    system("cls");
    cout << "===== MODULO DE COMPRAS (ABASTECIMIENTO) =====" << endl;
    cout << "ID Proveedor: "; cin >> id_prov;
    cout << "No. Orden Compra: "; cin >> no_ord;

    do {
        p.leer();
        cout << "\nID Producto: "; cin >> id_prod;
        cout << "Cantidad recibida: "; cin >> cant;
        cout << "Precio Costo Q: "; cin >> costo;

        carrito_c.push_back({ id_prod, cant, costo });

        cout << "¿Agregar otro producto a la orden? (s/n): "; cin >> mas;
    } while (mas == 's' || mas == 'S');

    if (!carrito_c.empty()) {
        comp.ejecutarCompra(no_ord, id_prov, carrito_c);
    }
    system("pause");
}

// --- MÓDULO DE CLIENTES ---
void menu_clientes() {
    Cliente c;
    int op;
    string n, a, ni;

    system("cls");
    cout << "--- GESTION DE CLIENTES ---\n1. Ver Listado\n2. Nuevo Cliente\n3. Regresar\nOpcion: ";
    cin >> op; cin.ignore();

    if (op == 1) {
        c.leer();
        system("pause");
    }
    if (op == 2) {
        cout << "NIT: "; getline(cin, ni);
        if (c.buscarNit(ni)) {
            cout << "El NIT ya existe." << endl;
        }
        else {
            cout << "Nombres: "; getline(cin, n);
            cout << "Apellidos: "; getline(cin, a);
            c = Cliente(n, a, "Ciudad", "00000000", 1, "", ni, "correo@ejemplo.com");
            c.crear();
        }
        system("pause");
    }
}

// --- MENÚ PRINCIPAL ---
int main() {
    int m_op;
    do {
        system("cls");
        cout << "========================================" << endl;
        cout << "   SISTEMA DE GESTION SUPERMERCADO" << endl;
        cout << "========================================" << endl;
        cout << "1. CLIENTES" << endl;
        cout << "2. PRODUCTOS (STOCK)" << endl;
        cout << "3. FACTURACION (VENTA)" << endl;
        cout << "4. COMPRAS (ABASTECIMIENTO)" << endl;
        cout << "5. SALIR" << endl;
        cout << "----------------------------------------" << endl;
        cout << "Seleccione una opcion: ";
        cin >> m_op;

        switch (m_op) {
        case 1: menu_clientes(); break;
        case 2: { Producto p; p.leer(); system("pause"); } break;
        case 3: menu_ventas(); break;
        case 4: menu_compras(); break;
        case 5: cout << "Cerrando sistema..." << endl; break;
        default: cout << "Opcion invalida." << endl; system("pause");
        }
    } while (m_op != 5);

    return 0;
}