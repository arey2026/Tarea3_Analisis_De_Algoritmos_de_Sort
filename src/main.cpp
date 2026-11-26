// main.cpp
// Programa de DEMOSTRACIÓN: muestra con un arreglo pequeño cómo
// funciona cada algoritmo. (Las mediciones de tiempo NO van aquí,
// van en benchmarks/benchmark.cpp.)
#include "sorting_algorithms.h"
#include <iostream>

int main() {
    std::vector<int> ejemplo = {29, 10, 14, 37, 13, 5, 88, 1};

    std::cout << "=== Tarea 3: Analisis de Algoritmos de Sort ===" << std::endl;
    std::cout << "Arreglo de ejemplo: ";
    imprimirArreglo(ejemplo);
    std::cout << "Esta ordenado? " << (estaOrdenado(ejemplo) ? "si" : "no") << std::endl;

    // Cada algoritmo recibe su PROPIA COPIA del arreglo original.
    std::vector<int> copia = ejemplo;
    insertionSort(copia);
    std::cout << "\nInsertion Sort: ";
    imprimirArreglo(copia);
    std::cout << "Esta ordenado? " << (estaOrdenado(copia) ? "si" : "no") << std::endl;

    // En las fases 4 a 8 agregaremos aquí los demás algoritmos.

    return 0;
}
