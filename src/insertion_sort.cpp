// insertion_sort.cpp
// ---------------------------------------------------------------
// INSERTION SORT (ordenamiento por inserción)
//
// Idea: igual que ordenar cartas en la mano. La parte izquierda del
// arreglo (arr[0..i-1]) siempre está ordenada. En cada paso tomamos
// el siguiente elemento (la "llave") y lo insertamos en su lugar
// dentro de esa parte ordenada, desplazando a la derecha los
// elementos mayores que él.
//
// Complejidad temporal:
//   Mejor caso   O(n)   -> arreglo ya ordenado (el while nunca entra)
//   Promedio     O(n^2)
//   Peor caso    O(n^2) -> arreglo en orden inverso
// Complejidad espacial: O(1) (ordena en el mismo arreglo)
// Estable: sí (no cambia el orden relativo de elementos iguales)
// ---------------------------------------------------------------
#include "sorting_algorithms.h"

void insertionSort(std::vector<int>& arr) {
    const int n = static_cast<int>(arr.size());

    // Empezamos en i = 1: un solo elemento (arr[0]) ya está "ordenado".
    for (int i = 1; i < n; i++) {
        int llave = arr[i]; // elemento que vamos a insertar
        int j = i - 1;      // último índice de la parte ordenada

        // Desplazamos a la derecha todos los elementos MAYORES que la llave.
        // Usamos '>' (y no '>=') para que el algoritmo sea estable.
        while (j >= 0 && arr[j] > llave) {
            arr[j + 1] = arr[j];
            j--;
        }

        // j + 1 es el hueco donde debe quedar la llave.
        arr[j + 1] = llave;
    }
}
