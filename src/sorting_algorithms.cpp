// sorting_algorithms.cpp
// ---------------------------------------------------------------
// Implementación de los seis algoritmos de la clase SortingAlgorithms.
// Ninguno usa std::sort ni otra función de ordenamiento de la biblioteca:
// todo está implementado a mano. Solo usamos std::swap para intercambiar.
// ---------------------------------------------------------------
#include "sorting_algorithms.h"
#include <utility> // std::swap

// =================================================================
// 1. INSERTION SORT (ordenamiento por inserción)
// -----------------------------------------------------------------
// Idea: como ordenar cartas en la mano. La parte izquierda
// (arr[0..i-1]) siempre está ordenada; tomamos el siguiente elemento
// (la "llave") y lo insertamos en su lugar, desplazando a la derecha
// los mayores que él.
//   Mejor O(n) [ya ordenado] | Promedio O(n^2) | Peor O(n^2) [inverso]
//   Espacio O(1) | Estable: sí
// =================================================================
void SortingAlgorithms::insertionSort(std::vector<int>& arr) {
    const int n = static_cast<int>(arr.size());

    for (int i = 1; i < n; i++) {
        int llave = arr[i]; // elemento que vamos a insertar
        int j = i - 1;      // último índice de la parte ordenada

        // Desplazamos a la derecha los elementos MAYORES que la llave.
        // '>' (y no '>=') mantiene el algoritmo estable.
        while (j >= 0 && arr[j] > llave) {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = llave; // j + 1 es el hueco correcto
    }
}

// =================================================================
// 2. SELECTION SORT (ordenamiento por selección)
// -----------------------------------------------------------------
// Idea: en cada pasada buscamos el MÍNIMO de la parte no ordenada
// (arr[i..n-1]) y lo intercambiamos con arr[i]. Así, tras la pasada i,
// las primeras i+1 posiciones tienen los menores elementos en orden.
//   Mejor O(n^2) | Promedio O(n^2) | Peor O(n^2)
//   (siempre hace n(n-1)/2 comparaciones, aunque ya esté ordenado)
//   Espacio O(1) | Estable: no | Hace como máximo n-1 intercambios
// =================================================================
void SortingAlgorithms::selectionSort(std::vector<int>& arr) {
    const int n = static_cast<int>(arr.size());

    for (int i = 0; i < n - 1; i++) {
        int indiceMinimo = i; // suponemos que el mínimo está en i

        // Buscamos un elemento menor en el resto del arreglo.
        for (int j = i + 1; j < n; j++) {
            if (arr[j] < arr[indiceMinimo]) {
                indiceMinimo = j;
            }
        }

        // Colocamos el mínimo en su posición final.
        if (indiceMinimo != i) {
            std::swap(arr[i], arr[indiceMinimo]);
        }
    }
}

// =================================================================
// 3. BUBBLE SORT (ordenamiento de burbuja)
// -----------------------------------------------------------------
// Idea: recorremos el arreglo comparando pares vecinos e
// intercambiándolos si están en desorden. En cada pasada el mayor
// elemento restante "sube como burbuja" hasta el final.
// Optimización: si en una pasada no hubo ningún intercambio, el arreglo
// ya está ordenado y terminamos antes.
//   Mejor O(n) [ya ordenado, gracias a la bandera] | Promedio O(n^2) | Peor O(n^2)
//   Espacio O(1) | Estable: sí
// =================================================================
void SortingAlgorithms::bubbleSort(std::vector<int>& arr) {
    const int n = static_cast<int>(arr.size());

    for (int pasada = 0; pasada < n - 1; pasada++) {
        bool huboIntercambio = false;

        // Los últimos 'pasada' elementos ya están en su lugar final.
        for (int j = 0; j < n - 1 - pasada; j++) {
            if (arr[j] > arr[j + 1]) {
                std::swap(arr[j], arr[j + 1]);
                huboIntercambio = true;
            }
        }

        if (!huboIntercambio) {
            break; // ya está ordenado
        }
    }
}

// =================================================================
// 4. MERGE SORT (ordenamiento por mezcla)
// -----------------------------------------------------------------
// Idea: "divide y vencerás".
//   1. Dividir el arreglo en dos mitades.
//   2. Ordenar cada mitad recursivamente.
//   3. Mezclar las dos mitades ordenadas en una sola.
// Un arreglo de 0 o 1 elemento ya está ordenado (caso base).
//   Mejor O(n log n) | Promedio O(n log n) | Peor O(n log n)
//   Espacio O(n) (arreglo auxiliar) + O(log n) de recursión | Estable: sí
// =================================================================
void SortingAlgorithms::mergeSort(std::vector<int>& arr) {
    if (arr.size() < 2) return;

    // Creamos el arreglo auxiliar UNA sola vez (en vez de uno por cada
    // mezcla) para no gastar tiempo reservando memoria repetidamente.
    std::vector<int> aux(arr.size());
    mergeSortRecursivo(arr, aux, 0, static_cast<int>(arr.size()) - 1);
}

void SortingAlgorithms::mergeSortRecursivo(std::vector<int>& arr, std::vector<int>& aux,
                                           int izq, int der) {
    if (izq >= der) return; // 0 o 1 elemento: caso base

    int medio = izq + (der - izq) / 2; // evita desbordamiento de (izq + der)
    mergeSortRecursivo(arr, aux, izq, medio);     // ordena mitad izquierda
    mergeSortRecursivo(arr, aux, medio + 1, der); // ordena mitad derecha
    mezclar(arr, aux, izq, medio, der);           // une ambas mitades
}

// Mezcla arr[izq..medio] y arr[medio+1..der] (ambas ya ordenadas).
void SortingAlgorithms::mezclar(std::vector<int>& arr, std::vector<int>& aux,
                                int izq, int medio, int der) {
    // Copiamos el tramo al arreglo auxiliar.
    for (int k = izq; k <= der; k++) {
        aux[k] = arr[k];
    }

    int i = izq;       // recorre la mitad izquierda
    int j = medio + 1; // recorre la mitad derecha
    int k = izq;       // posición donde escribimos en arr

    // Tomamos siempre el menor de los dos "frentes".
    // '<=' da prioridad a la izquierda en empates -> estable.
    while (i <= medio && j <= der) {
        if (aux[i] <= aux[j]) {
            arr[k++] = aux[i++];
        } else {
            arr[k++] = aux[j++];
        }
    }

    // Copiamos lo que haya sobrado de la mitad izquierda.
    // (Si sobró de la derecha, ya está en su lugar en arr.)
    while (i <= medio) {
        arr[k++] = aux[i++];
    }
}

// =================================================================
// 5. HEAP SORT (ordenamiento por montículo)
// -----------------------------------------------------------------
// Un "max-heap" es un árbol binario guardado en el arreglo donde cada
// padre es >= que sus hijos. Para el índice i:
//     hijo izquierdo = 2i + 1,  hijo derecho = 2i + 2
// Por lo tanto, el MAYOR elemento siempre está en arr[0].
// Idea:
//   1. Convertir el arreglo en un max-heap.
//   2. Repetir: intercambiar arr[0] (el mayor) con el último elemento
//      del heap, reducir el heap en 1 y restaurar la propiedad.
//   Mejor O(n log n) | Promedio O(n log n) | Peor O(n log n)
//   Espacio O(1) | Estable: no
// =================================================================
void SortingAlgorithms::heapSort(std::vector<int>& arr) {
    const int n = static_cast<int>(arr.size());

    // Paso 1: construir el max-heap. Empezamos en el último nodo que
    // tiene hijos (n/2 - 1) y vamos hacia la raíz. Este paso cuesta O(n).
    for (int i = n / 2 - 1; i >= 0; i--) {
        hundir(arr, n, i);
    }

    // Paso 2: extraer el mayor una y otra vez.
    for (int fin = n - 1; fin > 0; fin--) {
        std::swap(arr[0], arr[fin]); // el mayor pasa a su posición final
        hundir(arr, fin, 0);         // el heap ahora tiene 'fin' elementos
    }
}

// "Hunde" arr[i] dentro de un heap de tamaño n hasta que sea >= que sus
// hijos. Cada paso baja un nivel del árbol: O(log n).
void SortingAlgorithms::hundir(std::vector<int>& arr, int n, int i) {
    while (true) {
        int mayor = i;
        int izq = 2 * i + 1;
        int der = 2 * i + 2;

        if (izq < n && arr[izq] > arr[mayor]) mayor = izq;
        if (der < n && arr[der] > arr[mayor]) mayor = der;

        if (mayor == i) {
            return; // el padre ya es mayor que sus hijos
        }

        std::swap(arr[i], arr[mayor]);
        i = mayor; // seguimos hundiendo desde la nueva posición
    }
}

// =================================================================
// 6. QUICK SORT (ordenamiento rápido)
// -----------------------------------------------------------------
// Idea: "divide y vencerás".
//   1. Elegir un PIVOTE (aquí: el elemento del medio).
//   2. PARTICIONAR: dejar a la izquierda los elementos <= pivote y a la
//      derecha los >= pivote (esquema de Hoare).
//   3. Ordenar recursivamente cada parte.
// Elegir el pivote del medio evita el peor caso con arreglos ya
// ordenados o inversos, que sí sufriría si eligiéramos el primero/último.
//   Mejor O(n log n) | Promedio O(n log n)
//   Peor O(n^2) (pivotes muy desbalanceados, muy improbable con datos aleatorios)
//   Espacio O(log n) promedio (pila de recursión) | Estable: no
// =================================================================
void SortingAlgorithms::quickSort(std::vector<int>& arr) {
    if (arr.size() < 2) return;
    quickSortRecursivo(arr, 0, static_cast<int>(arr.size()) - 1);
}

void SortingAlgorithms::quickSortRecursivo(std::vector<int>& arr, int izq, int der) {
    if (izq >= der) return; // 0 o 1 elemento: caso base

    int p = particion(arr, izq, der);
    quickSortRecursivo(arr, izq, p);     // parte izquierda: arr[izq..p]
    quickSortRecursivo(arr, p + 1, der); // parte derecha:   arr[p+1..der]
}

// Partición de Hoare: dos índices avanzan desde los extremos hacia el
// centro e intercambian los elementos que están del lado equivocado.
// Devuelve p tal que todo arr[izq..p] <= todo arr[p+1..der].
int SortingAlgorithms::particion(std::vector<int>& arr, int izq, int der) {
    int pivote = arr[izq + (der - izq) / 2];
    int i = izq - 1;
    int j = der + 1;

    while (true) {
        do { i++; } while (arr[i] < pivote); // busca uno >= pivote a la izquierda
        do { j--; } while (arr[j] > pivote); // busca uno <= pivote a la derecha

        if (i >= j) {
            return j; // los índices se cruzaron: partición terminada
        }
        std::swap(arr[i], arr[j]);
    }
}
