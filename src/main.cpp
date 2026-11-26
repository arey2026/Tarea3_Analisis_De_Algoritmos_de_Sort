// main.cpp
// Programa de DEMOSTRACIÓN: muestra con un arreglo pequeño que cada
// algoritmo de la clase SortingAlgorithms ordena correctamente.
// (Las mediciones de tiempo NO van aquí, van en benchmarks/benchmark.cpp.)
#include "sorting_algorithms.h"
#include <iostream>
#include <string>

struct Algoritmo {
    std::string nombre;
    FuncionSort funcion;
};

int main() {
    const std::vector<int> ejemplo = {29, 10, 14, 37, 13, 5, 88, 1, 14, -7};

    const std::vector<Algoritmo> algoritmos = {
        {"Insertion Sort", SortingAlgorithms::insertionSort},
        {"Selection Sort", SortingAlgorithms::selectionSort},
        {"Bubble Sort   ", SortingAlgorithms::bubbleSort},
        {"Merge Sort    ", SortingAlgorithms::mergeSort},
        {"Heap Sort     ", SortingAlgorithms::heapSort},
        {"Quick Sort    ", SortingAlgorithms::quickSort},
    };

    std::cout << "=== Tarea 3: Analisis de Algoritmos de Sort ===" << std::endl;
    std::cout << "Original      : ";
    imprimirArreglo(ejemplo);
    std::cout << std::endl;

    for (const Algoritmo& a : algoritmos) {
        std::vector<int> copia = ejemplo; // cada algoritmo recibe su PROPIA copia
        a.funcion(copia);
        std::cout << a.nombre << ": ";
        imprimirArreglo(copia);
        std::cout << "                ordenado? " << (estaOrdenado(copia) ? "si" : "NO") << std::endl;
    }

    return 0;
}
