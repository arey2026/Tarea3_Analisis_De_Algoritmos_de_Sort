// test_sorting.cpp
// ---------------------------------------------------------------
// Pruebas de CORRECTITUD: comprueban que cada algoritmo ordena bien.
//
// Un resultado es correcto si cumple DOS condiciones:
//   1. Está ordenado de menor a mayor (estaOrdenado).
//   2. Contiene exactamente los mismos elementos que el original
//      (no se perdió, duplicó ni inventó ningún número).
// ---------------------------------------------------------------
#include "sorting_algorithms.h"
#include <iostream>
#include <map>
#include <random>
#include <string>

// Un algoritmo a probar: su nombre y la función que lo implementa.
struct Algoritmo {
    std::string nombre;
    FuncionSort funcion;
};

// Lista de los seis algoritmos a probar (todos en la clase SortingAlgorithms).
const std::vector<Algoritmo> ALGORITMOS = {
    {"Insertion Sort", SortingAlgorithms::insertionSort},
    {"Selection Sort", SortingAlgorithms::selectionSort},
    {"Bubble Sort",    SortingAlgorithms::bubbleSort},
    {"Merge Sort",     SortingAlgorithms::mergeSort},
    {"Heap Sort",      SortingAlgorithms::heapSort},
    {"Quick Sort",     SortingAlgorithms::quickSort},
};

// Un caso de prueba: una descripción y el arreglo de entrada.
struct CasoPrueba {
    std::string descripcion;
    std::vector<int> datos;
};

// Comprueba que 'a' y 'b' tienen los mismos elementos con la misma
// cantidad de repeticiones, contando cuántas veces aparece cada valor.
bool mismosElementos(const std::vector<int>& a, const std::vector<int>& b) {
    if (a.size() != b.size()) return false;
    std::map<int, int> conteo;
    for (int x : a) conteo[x]++;
    for (int x : b) conteo[x]--;
    for (const auto& par : conteo) {
        if (par.second != 0) return false;
    }
    return true;
}

// Genera n enteros aleatorios en [minimo, maximo] con una semilla fija,
// para que las pruebas den siempre el mismo resultado.
std::vector<int> generarAleatorio(int n, int minimo, int maximo, unsigned semilla) {
    std::mt19937 generador(semilla);
    std::uniform_int_distribution<int> distribucion(minimo, maximo);
    std::vector<int> datos(n);
    for (int i = 0; i < n; i++) {
        datos[i] = distribucion(generador);
    }
    return datos;
}

std::vector<CasoPrueba> crearCasos() {
    std::vector<CasoPrueba> casos;
    casos.push_back({"Arreglo vacio", {}});
    casos.push_back({"Un elemento", {42}});
    casos.push_back({"Dos elementos desordenados", {2, 1}});
    casos.push_back({"Ya ordenado", {1, 2, 3, 4, 5, 6, 7, 8}});
    casos.push_back({"Orden inverso", {9, 8, 7, 6, 5, 4, 3, 2, 1}});
    casos.push_back({"Todos iguales", {7, 7, 7, 7, 7}});
    casos.push_back({"Con duplicados", {5, 1, 3, 5, 1, 3, 2, 2}});
    casos.push_back({"Con negativos", {-3, 10, 0, -50, 7, -1}});
    casos.push_back({"Aleatorio 1,000 (rango pequeno, muchos duplicados)",
                     generarAleatorio(1000, 0, 20, 12345)});
    casos.push_back({"Aleatorio 5,000 (rango amplio)",
                     generarAleatorio(5000, -1000000, 1000000, 2026)});

    // Arreglos grandes ordenados e inversos: son el peor caso de varios
    // algoritmos (por ejemplo, Quick Sort con un mal pivote).
    std::vector<int> ordenadoGrande(3000), inversoGrande(3000);
    for (int i = 0; i < 3000; i++) {
        ordenadoGrande[i] = i;
        inversoGrande[i] = 3000 - i;
    }
    casos.push_back({"Ordenado 3,000", ordenadoGrande});
    casos.push_back({"Inverso 3,000", inversoGrande});

    // Tamaños impares/pares pequeños: revisan los límites de los índices.
    for (int n = 3; n <= 17; n++) {
        casos.push_back({"Aleatorio n=" + std::to_string(n),
                         generarAleatorio(n, -10, 10, 100 + n)});
    }
    return casos;
}

int main() {
    const std::vector<CasoPrueba> casos = crearCasos();
    int aprobadas = 0;
    int fallidas = 0;

    for (const Algoritmo& algoritmo : ALGORITMOS) {
        std::cout << "=== " << algoritmo.nombre << " ===" << std::endl;

        for (const CasoPrueba& caso : casos) {
            std::vector<int> copia = caso.datos;   // copia propia
            algoritmo.funcion(copia);

            bool correcto = estaOrdenado(copia) && mismosElementos(copia, caso.datos);
            std::cout << (correcto ? "  [OK]    " : "  [FALLO] ") << caso.descripcion << std::endl;

            if (correcto) aprobadas++;
            else fallidas++;
        }
    }

    std::cout << "\nResultado: " << aprobadas << " pruebas aprobadas, "
              << fallidas << " fallidas." << std::endl;

    // Código de salida distinto de 0 si algo falló (útil para el Makefile).
    return fallidas == 0 ? 0 : 1;
}
