// sorting_algorithms.h
// ---------------------------------------------------------------
// Declaraciones (prototipos) de todos los algoritmos de ordenamiento
// y de las funciones auxiliares del proyecto.
//
// Cada algoritmo se implementa en su propio archivo dentro de src/.
// Todos reciben el arreglo POR REFERENCIA (std::vector<int>&) y lo
// ordenan de menor a mayor "en el mismo arreglo".
// ---------------------------------------------------------------
#ifndef SORTING_ALGORITHMS_H
#define SORTING_ALGORITHMS_H

#include <vector>

// ===== Algoritmos de ordenamiento =====
// (se agregan uno por uno en las fases 3 a 8)


// ===== Funciones auxiliares (src/utils.cpp) =====

// Devuelve true si el arreglo está ordenado de menor a mayor.
bool estaOrdenado(const std::vector<int>& arr);

// Imprime el arreglo en consola, por ejemplo: [5, 2, 9]
void imprimirArreglo(const std::vector<int>& arr);

#endif // SORTING_ALGORITHMS_H
