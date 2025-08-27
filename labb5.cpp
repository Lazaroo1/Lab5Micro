#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <stdexcept>
#include <cstdint>
#include <pthread.h>
#include <cstdlib>
#include <ctime>
#include <sys/time.h>
using namespace std;

// ====================== FUNCIONES COMUNES =========================
int suma(int a, int b) { return a + b; }
int resta(int a, int b) { return a - b; }
int multiplicacion(int a, int b) { return a * b; }
int division_entera(int a, int b) {
    if (b == 0) throw runtime_error("Division por cero");
    return a / b;
}
int potencia(int a, int b) {
    int res = 1;
    for (int i = 0; i < b; i++) res *= a;
    return res;
}
int modulo(int a, int b) {
    if (b == 0) throw runtime_error("Modulo por cero");
    return a % b;
}

// =================== CALCULO PARALELO =============================
struct DatosCalc {
    int inicio;
    int fin;
    int *arreglo;
    int max_local;
};

void* buscarMaximo(void* arg) {
    DatosCalc* d = (DatosCalc*) arg;
    struct timeval t1, t2;
    gettimeofday(&t1, NULL);

    int maximo = d->arreglo[d->inicio];
    for (int i = d->inicio; i < d->fin; i++) {
        if (d->arreglo[i] > maximo) maximo = d->arreglo[i];
    }
    d->max_local = maximo;

    gettimeofday(&t2, NULL);
    long tiempo = (t2.tv_sec - t1.tv_sec) * 1000000L + (t2.tv_usec - t1.tv_usec);

    cout << "Hilo " << pthread_self()
         << " procesando de " << d->inicio << " a " << d->fin
         << " -> Max local: " << maximo
         << " Tiempo: " << tiempo << " us\n";
    pthread_exit(NULL);
}

void calculoParalelo() {
    int n, numHilos;
    cout << "Ingrese tamaño del arreglo: ";
    cin >> n;
    cout << "Ingrese cantidad de hilos: ";
    cin >> numHilos;
    while (numHilos >= n) {
        cout << "Error: el tamaño del arreglo debe ser mayor que la cantidad de hilos.\n";
        cout << "Ingrese tamaño del arreglo: ";
        cin >> n;
        cout << "Ingrese cantidad de hilos: ";
        cin >> numHilos;
    }

    int* arreglo = new int[n];
    srand(time(NULL));
    for (int i = 0; i < n; i++) arreglo[i] = rand() % 100000 + 1;

    int base = n / numHilos;
    int resto = n % numHilos;

    pthread_t hilos[numHilos];
    DatosCalc datos[numHilos];

    int inicio = 0;
    for (int i = 0; i < numHilos; i++) {
        datos[i].inicio = inicio;
        datos[i].fin = inicio + base;
        if (i == numHilos - 1) datos[i].fin += resto;
        datos[i].arreglo = arreglo;
        pthread_create(&hilos[i], NULL, buscarMaximo, (void*)&datos[i]);
        inicio = datos[i].fin;
    }

    for (int i = 0; i < numHilos; i++) pthread_join(hilos[i], NULL);

    int max_global = datos[0].max_local;
    for (int i = 1; i < numHilos; i++)
        if (datos[i].max_local > max_global) max_global = datos[i].max_local;

    cout << "Maximo global: " << max_global << "\n";
    delete[] arreglo;
}

// =================== COMPILADOR PARALELO ===========================
struct DatosComp {
    string instr;
    int indice;
};

void decodificarInstruccion(const string& binario, int indice) {
    if (binario.size() != 8) {
        cout << "Error: instruccion debe tener 8 bits.\n";
        return;
    }
    for (char c : binario)
        if (c != '0' && c != '1') {
            cout << "Error: caracteres invalidos.\n";
            return;
        }

    uint8_t num = 0;
    for (char c : binario) num = (num << 1) | (c - '0');

    uint8_t opcode = (num >> 5) & 0x07;
    uint8_t mod    = (num >> 4) & 0x01;
    uint8_t a_orig = (num >> 2) & 0x03;
    uint8_t b_orig = num & 0x03;
    uint8_t a_val = a_orig;
    uint8_t b_val = b_orig;
    if (mod) swap(a_val, b_val);

    cout << "Instruccion " << indice << ": " << binario << "\n";
    
    //  detalles de la decodificación
    cout << "OPCODE: " << (int)opcode;
    
    string operacion_nombre;
    string simbolo_op;
    switch (opcode) {
        case 0: operacion_nombre = " (Suma)"; simbolo_op = "+"; break;
        case 1: operacion_nombre = " (Resta)"; simbolo_op = "-"; break;
        case 2: operacion_nombre = " (Multiplicacion)"; simbolo_op = "*"; break;
        case 3: operacion_nombre = " (Division)"; simbolo_op = "/"; break;
        case 5: operacion_nombre = " (Potencia)"; simbolo_op = "^"; break;
        case 6: operacion_nombre = " (Modulo)"; simbolo_op = "%"; break;
        default: operacion_nombre = " (Invalido)"; simbolo_op = "?"; break;
    }
    cout << operacion_nombre << "\n";
    
    //  MOD
    cout << "MOD: " << (int)mod;
    if (mod == 0) {
        cout << " (Orden directo)\n";
    } else {
        cout << " (Invertir operandos)\n";
    }
    
    //  valores originales y finales
    cout << "A: " << (int)a_val << "\n";
    cout << "B: " << (int)b_val << "\n";
    
    try {
        int resultado;
        switch (opcode) {
            case 0: resultado = suma(a_val, b_val); break;
            case 1: resultado = resta(a_val, b_val); break;
            case 2: resultado = multiplicacion(a_val, b_val); break;
            case 3: resultado = division_entera(a_val, b_val); break;
            case 5: resultado = potencia(a_val, b_val); break;
            case 6: resultado = modulo(a_val, b_val); break;
            default:
                cout << "OPCODE invalido\n"; return;
        }
        cout << "Resultado: " << (int)a_val << " " << simbolo_op << " " << (int)b_val
             << " = " << resultado << "\n";
    } catch (const exception& e) {
        cout << "Resultado: Error. " << e.what() << "\n";
    }
}

void* ejecutar(void* arg) {
    DatosComp* d = (DatosComp*)arg;
    struct timeval t1, t2;
    gettimeofday(&t1, NULL);

    cout << "Hilo " << pthread_self() << " ejecutando instruccion "
         << d->indice << "\n";
    decodificarInstruccion(d->instr, d->indice);

    gettimeofday(&t2, NULL);
    long tiempo = (t2.tv_sec - t1.tv_sec) * 1000000L + (t2.tv_usec - t1.tv_usec);
    cout << "TID: " << pthread_self()
         << " - Tiempo de ejecucion: " << tiempo << " us\n\n";

    pthread_exit(NULL);
}

void compiladorParalelo() {
    cin.ignore();
    cout << "Ingrese instrucciones binarias separadas por espacio: ";
    string entrada;
    getline(cin, entrada);

    stringstream ss(entrada);
    vector<string> tokens;
    string token;
    while (ss >> token) tokens.push_back(token);

    if (tokens.size() < 3) {
        cout << "Se necesitan al menos 3 instrucciones.\n";
        return;
    }

    // Primera instrucción en hilo principal
    struct timeval t1, t2;
    gettimeofday(&t1, NULL);
    cout << "Hilo principal ejecutando instruccion 1\n";
    decodificarInstruccion(tokens[0], 1);
    gettimeofday(&t2, NULL);
    long tiempo = (t2.tv_sec - t1.tv_sec) * 1000000L + (t2.tv_usec - t1.tv_usec);
    cout << "TID: " << pthread_self()
         << " - Tiempo de ejecucion: " << tiempo << " us\n\n";

    // Resto de instrucciones en hilos
    int n = tokens.size();
    pthread_t hilos[n-1];
    DatosComp datos[n-1];
    for (int i = 1; i < n; i++) {
        datos[i-1].instr = tokens[i];
        datos[i-1].indice = i + 1;
        pthread_create(&hilos[i-1], NULL, ejecutar, (void*)&datos[i-1]);
    }
    for (int i = 0; i < n-1; i++) pthread_join(hilos[i], NULL);
}

// ========================== MENU ================================
int main() {
    int opcion;
    cout << "MENU:\n1. Calculo Paralelo\n2. Compilador Paralelo\nSeleccione: ";
    cin >> opcion;
    if (opcion == 1) calculoParalelo();
    else if (opcion == 2) compiladorParalelo();
    else cout << "Opcion invalida.\n";
    return 0;
}
