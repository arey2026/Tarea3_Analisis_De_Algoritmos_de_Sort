// sorting_algorithms.h
// ---------------------------------------------------------------
// Declaración de la clase SortingAlgorithms, que reúne los SEIS
// algoritmos de ordenamiento del proyecto (implementados en
// src/sorting_algorithms.cpp), y de las funciones auxiliares.
//
// Todos los algoritmos reciben el arreglo POR REFERENCIA
// (std::vector<int>&) y lo ordenan de menor a mayor en el mismo arreglo.
// ---------------------------------------------------------------
#ifndef SORTING_ALGORITHMS_H
#define SORTING_ALGORITHMS_H

#include <vector>

// Tipo "puntero a función de ordenamiento": permite guardar los seis
// algoritmos en una lista y recorrerlos en las pruebas y el benchmark.
using FuncionSort = void (*)(std::vector<int>&);

class SortingAlgorithms {
public:
    // Los métodos son 'static': no hace falta crear un objeto para usarlos,
    // se llaman así:  SortingAlgorithms::insertionSort(arreglo);
    // Además, un método static puede guardarse en un FuncionSort.

    // ----- Algoritmos O(n^2) -----
    static void insertionSort(std::vector<int>& arr);
    static void selectionSort(std::vector<int>& arr);
    static void bubbleSort(std::vector<int>& arr);

    // ----- Algoritmos O(n log n) -----
    static void mergeSort(std::vector<int>& arr);
    static void heapSort(std::vector<int>& arr);
    static void quickSort(std::vector<int>& arr);

private:
    // Funciones de apoyo internas (el usuario de la clase no las necesita).

    // Merge Sort
    static void mergeSortRecursivo(std::vector<int>& arr, std::vector<int>& aux, int izq, int der);
    static void mezclar(std::vector<int>& arr, std::vector<int>& aux, int izq, int medio, int der);

    // Heap Sort
    static void hundir(std::vector<int>& arr, int n, int i);

    // Quick Sort
    static void quickSortRecursivo(std::vector<int>& arr, int izq, int der);
    static int particion(std::vector<int>& arr, int izq, int der);
};

// ===== Funciones auxiliares (src/utils.cpp) =====

// Devuelve true si el arreglo está ordenado de menor a mayor.
bool estaOrdenado(const std::vector<int>& arr);

// Imprime el arreglo en consola, por ejemplo: [5, 2, 9]
void imprimirArreglo(const std::vector<int>& arr);

#endif // SORTING_ALGORITHMS_H
