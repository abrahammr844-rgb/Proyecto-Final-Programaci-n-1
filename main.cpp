#ifndef _HAS_STD_BYTE
#define _HAS_STD_BYTE 0
#endif
#include <mysql.h>
#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include "Cliente.h"
#include "Empleado.h"
#include "Producto.h"
#include "Venta.h"

using namespace std;

// --- CONTROLADOR DE FLUJO DE FACTURACIÓN ---
void menu_ventas() {
    Venta v; Cliente c; Producto p; Empleado emp; vector<Carrito> mi_carrito;
    string nit_buscado; int id_emp, id_prod, cant, no_fact; char serie, agregar_mas;

    system("cls");
    cout << "===== MODULO DE FACTURACION (PUNTO DE VENTA) =====" << endl;

    // REQUERIMIENTO 2: Visualizar información del cliente
    cout << "Ingrese NIT del Cliente: "; cin >> nit_buscado;
    if (!c.buscarNit(nit_buscado)) {
        cout << "\n[Aviso] El cliente no existe. Registrelo en la gestion de clientes." << endl;
        system("pause"); return;
    }
    cout << ">> Cliente: " << c.getNombres() << " " << c.getApellidos() << " | ID: " << c.getId() << "\n" << endl;

    // REQUERIMIENTO 1: Visualizar nombre del cajero/empleado
    cout << "ID del Empleado (Cajero): "; cin >> id_emp;
    if (!emp.buscarPorId(id_emp)) {
        cout << "\nxx Error: El ID de empleado no existe. xx" << endl;
        system("pause"); return;
    }
    cout << ">> Cajero Activo: " << emp.getNombres() << " " << emp.getApellidos() << "\n" << endl;

    // REQUERIMIENTO 3: Serie y número de factura automáticos
    cout << "Serie de Factura a emitir (Ej: A, B, C): "; cin >> serie;
    no_fact = v.obtenerSiguienteNumeroFactura(serie);
    cout << ">> Generando de forma automatica -> Factura No: " << no_fact << " (Serie " << serie << ")\n" << endl;
    system("pause");

    do {
        system("cls");
        cout << "--- AGREGANDO PRODUCTOS AL CARRITO ---" << endl;
        p.leer(); // Muestra el inventario disponible

        cout << "\nID del Producto: "; cin >> id_prod;

        // REQUERIMIENTO 4: Mostrar nombre y precio automáticamente al ingresar ID
        if (p.buscarPorId(id_prod)) {
            cout << ">> Producto Seleccionado: " << p.getProducto() << endl;
            cout << ">> Precio Unitario:       Q" << p.getPrecioVenta() << endl;
            cout << ">> Stock Disponible:      " << p.getExistencia() << " unidades." << endl;

            cout << "\nCantidad a comprar: "; cin >> cant;

            if (cant <= p.getExistencia()) {
                // REQUERIMIENTO 5: Multiplicar precio * cantidad y generar total del producto
                double subtotal_producto = p.getPrecioVenta() * cant;
                cout << ">> TOTAL DE ESTE PRODUCTO: Q" << subtotal_producto << endl;

                // Guardamos en el vector incluyendo el nombre del producto
                mi_carrito.push_back({ id_prod, p.getProducto(), cant, p.getPrecioVenta() });
                cout << "\n[OK] Agregado exitosamente al carrito." << endl;
            }
            else {
                cout << "\nxx Error: Inventario insuficiente. El stock es de: " << p.getExistencia() << " xx" << endl;
            }
        }
        else {
            cout << "\nxx Error: El ID de producto ingresado no existe. xx" << endl;
        }

        cout << "\n¿Desea agregar otro producto? (s/n): "; cin >> agregar_mas;
    } while (agregar_mas == 's' || agregar_mas == 'S');

    // GENERACIÓN E IMPRESIÓN DE LA FACTURA FINAL EN PANTALLA Y EN BASE DE DATOS
    if (!mi_carrito.empty()) {
        system("cls");
        cout << "==========================================================" << endl;
        cout << "                   S U P E R M E R C A D O                 " << endl;
        cout << "==========================================================" << endl;
        cout << "Factura Serie: " << serie << "   No. Factura: " << no_fact << endl;
        cout << "Cliente:       " << c.getNombres() << " " << c.getApellidos() << " | NIT: " << nit_buscado << endl;
        cout << "Cajero ID:     " << id_emp << " - " << emp.getNombres() << endl;
        cout << "----------------------------------------------------------" << endl;
        cout << "Cant. | Descripcion                 | P. Unit | Subtotal  " << endl;
        cout << "----------------------------------------------------------" << endl;

        double total_general = 0.0;
        for (auto const& item : mi_carrito) {
            double sub = item.cantidad * item.precio_unitario;
            total_general += sub;

            // Ajustamos espacios para simular una impresión estética de ticket
            string desc = item.nombre_producto;
            if (desc.length() > 25) desc = desc.substr(0, 22) + "...";
            else desc.append(25 - desc.length(), ' ');

            cout << item.cantidad << "      " << desc << "   Q" << item.precio_unitario << "    Q" << sub << endl;
        }

        cout << "----------------------------------------------------------" << endl;
        cout << "                                    TOTAL FINAL: Q" << total_general << endl;
        cout << "==========================================================\n" << endl;

        cout << "Procesando transaccion en Base de Datos..." << endl;
        v.ejecutarVenta(c.getId(), id_emp, serie, no_fact, mi_carrito);
    }
    else {
        cout << "\n[Aviso] Venta cancelada. El carrito de compras esta vacio." << endl;
    }
    system("pause");
}

// --- GESTIÓN DE CLIENTES ---
void menu_clientes() {
    Cliente c; int op; string n, a, ni, dir, tel, mail; int gen;
    system("cls");
    cout << "--- GESTION DE CLIENTES ---\n1. Ver Listado\n2. Nuevo Cliente\n3. Regresar\nOpcion: ";
    cin >> op; cin.ignore();

    if (op == 1) { c.leer(); system("pause"); }
    if (op == 2) {
        cout << "NIT: "; getline(cin, ni);
        if (c.buscarNit(ni)) { cout << "El NIT ya existe." << endl; }
        else {
            cout << "Nombres: "; getline(cin, n);
            cout << "Apellidos: "; getline(cin, a);
            cout << "Direccion: "; getline(cin, dir);
            cout << "Telefono: "; getline(cin, tel);
            cout << "Genero (1=M, 0=F): "; cin >> gen; cin.ignore();
            cout << "Correo Electronico: "; getline(cin, mail);

            try {
                c = Cliente(n, a, dir, tel, gen, ni, mail);
                if (c.crear()) cout << ">> Cliente guardado exitosamente." << endl;
            }
            catch (const std::invalid_argument& e) {
                cout << "\nxx Registro cancelado: " << e.what() << " xx" << endl;
            }
        }
        system("pause");
    }
}

// --- SUB-MENÚ EXCLUSIVO PARA EMPLEADOS (ROL 1) ---
void sub_menu_empleados() {
    Empleado emp; int id_emp; string pass_cui;
    system("cls");
    cout << "========================================" << endl;
    cout << "     LOGIN CONTROL ADMINISTRATIVO" << endl;
    cout << "========================================" << endl;
    cout << "ID de Empleado: "; cin >> id_emp;
    cin.ignore(10000, '\n');
    cout << "Contraseña (CUI): "; getline(cin, pass_cui);

    if (!emp.autenticar(id_emp, pass_cui)) {
        cout << "\n[RECHAZADO] No posee permisos para modificar inventario.\n" << endl;
        system("pause"); return;
    }

    int op;
    do {
        system("cls");
        cout << "==== PANEL DE CONTROL DE INVENTARIOS ====" << endl;
        cout << "1. Ver Stock en Bodegas" << endl;
        cout << "2. Modificar/Ingresar Producto Nuevo" << endl;
        cout << "3. Administrar Clientes" << endl;
        cout << "4. Cerrar Sesion Administrativa" << endl;
        cout << "----------------------------------------" << endl;
        cout << "Seleccione: "; cin >> op; cin.ignore();

        if (op == 2) {
            string pr, ds, img; int mr, st; double cst, vnt;
            cout << "Producto: "; getline(cin, pr);
            cout << "ID Marca: "; cin >> mr; cin.ignore();
            cout << "Descripcion: "; getline(cin, ds);
            cout << "Imagen (nombre archivo): "; getline(cin, img);
            cout << "Precio Costo: "; cin >> cst;
            cout << "Precio Venta: "; cin >> vnt;
            cout << "Existencia Inicial: "; cin >> st;

            try {
                Producto nuevoP(pr, mr, ds, img, cst, vnt, st);
                if (nuevoP.crear()) cout << ">> Producto agregado al inventario." << endl;
            }
            catch (const std::invalid_argument& e) {
                cout << "Error de ingreso: " << e.what() << endl;
            }
            system("pause");
        }
        else if (op == 1) { Producto p; p.leer(); system("pause"); }
        else if (op == 3) { menu_clientes(); }
    } while (op != 4);
}

// --- SUB-MENÚ EXCLUSIVO PARA CLIENTES (ROL 2) ---
void sub_menu_clientes() {
    int op;
    do {
        system("cls");
        cout << "========================================" << endl;
        cout << "          PORTAL DE ATENCION" << endl;
        cout << "========================================" << endl;
        cout << "1. Catalogo de Precios" << endl;
        cout << "2. Proceder al Cobro/Facturacion" << endl;
        cout << "3. Regresar" << endl;
        cout << "----------------------------------------" << endl;
        cout << "Seleccione una opcion: "; cin >> op;
        switch (op) {
        case 1: { Producto p; p.leer(); system("pause"); break; }
        case 2: menu_ventas(); break;
        }
    } while (op != 3);
}

// --- ENTRADA PRINCIPAL ---
int main() {
    int m_op;
    do {
        system("cls");
        cout << "========================================" << endl;
        cout << "       SUPERMERCADO - TERMINAL" << endl;
        cout << "========================================" << endl;
        cout << "1. Entrar como EMPLEADO" << endl;
        cout << "2. Entrar como CLIENTE" << endl;
        cout << "3. Apagar terminal" << endl;
        cout << "----------------------------------------" << endl;
        cout << "Seleccione un perfil: ";
        if (!(cin >> m_op)) { cin.clear(); cin.ignore(10000, '\n'); continue; }

        switch (m_op) {
        case 1: sub_menu_empleados(); break;
        case 2: sub_menu_clientes(); break;
        }
    } while (m_op != 3);
    return 0;
}