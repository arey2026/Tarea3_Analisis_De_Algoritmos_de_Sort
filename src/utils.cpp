// utils.cpp
// Funciones auxiliares compartidas por la demo, las pruebas y el benchmark.
#include "sorting_algorithms.h"
#include <iostream>

bool estaOrdenado(const std::vector<int>& arr) {
    // Recorremos pares vecinos: si algún elemento es mayor que el
    // siguiente, el arreglo NO está ordenado.
    for (std::size_t i = 1; i < arr.size(); i++) {
        if (arr[i - 1] > arr[i]) {
            return false;
        }
    }
    return true; // un arreglo vacío o de 1 elemento está ordenado
}

void imprimirArreglo(const std::vector<int>& arr) {
    std::cout << "[";
    for (std::size_t i = 0; i < arr.size(); i++) {
        std::cout << arr[i];
        if (i + 1 < arr.size()) {
            std::cout << ", ";
        }
    }
    std::cout << "]" << std::endl;
}
