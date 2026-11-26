// benchmark.cpp
// ---------------------------------------------------------------
// Mide el tiempo de ejecución de los seis algoritmos de la clase
// SortingAlgorithms con distintos tamaños de arreglo y guarda los
// resultados en results/resultados_sort.csv. Al terminar, llama al
// script scripts/generar_excel.ps1 para crear results/resultados_sort.xlsx.
//
// Uso (desde la carpeta raíz del proyecto):
//     benchmark.exe              -> límite de 30 segundos por prueba
//     benchmark.exe 60           -> límite de 60 segundos por prueba
//     benchmark.exe 30 --sin-excel  -> no genera el Excel al final
//
// Reglas de medición:
//   * Mismos datos para todos: para cada tamaño se generan los datos
//     UNA vez con una semilla fija, y cada algoritmo recibe su propia copia.
//   * Solo se mide el ordenamiento: la generación de datos, las copias,
//     la verificación y las impresiones quedan FUERA del cronómetro.
//   * Se usa std::chrono::steady_clock (reloj que nunca retrocede).
//   * Cada resultado se verifica con estaOrdenado(); si falla, se aborta.
//   * Nunca se inventa un tiempo: si una prueba no se ejecuta, se
//     registra como OMITIDO.
// ---------------------------------------------------------------
#include "sorting_algorithms.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>

// ===================== CONFIGURACIÓN =====================
const unsigned SEMILLA = 2026;               // semilla fija -> experimentos reproducibles
const int VALOR_MINIMO = 0;                  // rango de los números aleatorios
const int VALOR_MAXIMO = 10000000;
const int REPETICIONES_MAXIMAS = 5;          // repeticiones por prueba (si el tiempo lo permite)
const double LIMITE_POR_DEFECTO_S = 30.0;    // límite de tiempo por prueba, en segundos
const long long ELEMENTOS_POR_LOTE = 100000; // ver explicación de "lote" más abajo

const std::vector<int> TAMANOS = {10, 100, 1000, 5000, 10000, 50000, 100000, 500000, 1000000};

const std::string RUTA_CSV = "results/resultados_sort.csv";
const std::string RUTA_CONFIG = "results/config_benchmark.csv";
// =========================================================

enum class Complejidad { CUADRATICA, N_LOG_N };

struct Algoritmo {
    std::string nombre;
    FuncionSort funcion;
    Complejidad complejidad;

    // Estado durante el benchmark:
    bool detenido = false;     // true -> se omiten los tamaños siguientes
    int ultimoN = 0;           // último tamaño medido
    double ultimoMs = 0.0;     // tiempo promedio en ese tamaño
};

struct Resultado {
    int n;
    std::string algoritmo;
    std::string estado;        // COMPLETADO, SUPERO_LIMITE u OMITIDO
    double promedioMs;         // solo válido si no está OMITIDO
    double minimoMs;
    double maximoMs;
    int repeticiones;          // repeticiones realmente ejecutadas
    int lote;                  // ordenamientos por repetición
    std::string nota;
};

// Genera n números aleatorios con la semilla fija.
std::vector<int> generarDatos(int n) {
    std::mt19937 generador(SEMILLA);
    std::uniform_int_distribution<int> distribucion(VALOR_MINIMO, VALOR_MAXIMO);
    std::vector<int> datos(n);
    for (int i = 0; i < n; i++) {
        datos[i] = distribucion(generador);
    }
    return datos;
}

// Estima el tiempo (ms) de UN ordenamiento de tamaño n a partir del último
// tamaño medido, usando la complejidad teórica del algoritmo:
//   O(n^2):     t(n) = t(n0) * (n / n0)^2
//   O(n log n): t(n) = t(n0) * (n log n) / (n0 log n0)
// Solo se usa para DECIDIR si conviene ejecutar la prueba; nunca se registra.
double estimarMs(const Algoritmo& a, int n) {
    if (a.ultimoN == 0) return 0.0; // sin datos previos: no se puede estimar
    double r = static_cast<double>(n) / a.ultimoN;
    if (a.complejidad == Complejidad::CUADRATICA) {
        return a.ultimoMs * r * r;
    }
    return a.ultimoMs * (n * std::log2(n)) / (a.ultimoN * std::log2(a.ultimoN));
}

std::string formatoMiles(long long x) {
    std::string s = std::to_string(x);
    for (int i = static_cast<int>(s.size()) - 3; i > 0; i -= 3) s.insert(i, ",");
    return s;
}

int main(int argc, char* argv[]) {
    double limiteS = LIMITE_POR_DEFECTO_S;
    bool generarExcel = true;
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--sin-excel") generarExcel = false;
        else limiteS = std::atof(argv[i]);
    }
    if (limiteS <= 0) {
        std::cerr << "Limite invalido. Uso: benchmark.exe [segundos] [--sin-excel]" << std::endl;
        return 1;
    }
    const double limiteMs = limiteS * 1000.0;

    std::vector<Algoritmo> algoritmos = {
        {"Insertion Sort", SortingAlgorithms::insertionSort, Complejidad::CUADRATICA},
        {"Selection Sort", SortingAlgorithms::selectionSort, Complejidad::CUADRATICA},
        {"Bubble Sort",    SortingAlgorithms::bubbleSort,    Complejidad::CUADRATICA},
        {"Merge Sort",     SortingAlgorithms::mergeSort,     Complejidad::N_LOG_N},
        {"Heap Sort",      SortingAlgorithms::heapSort,      Complejidad::N_LOG_N},
        {"Quick Sort",     SortingAlgorithms::quickSort,     Complejidad::N_LOG_N},
    };

    std::cout << "=== Benchmark de algoritmos de ordenamiento ===" << std::endl;
    std::cout << "Semilla: " << SEMILLA << " | Limite por prueba: " << limiteS
              << " s | Repeticiones maximas: " << REPETICIONES_MAXIMAS << std::endl << std::endl;

    std::vector<Resultado> resultados;
    std::cout << std::fixed << std::setprecision(4);

    for (int n : TAMANOS) {
        // 1) Datos originales del tamaño n: se generan ANTES de medir.
        const std::vector<int> original = generarDatos(n);

        // 2) "Lote": ordenar 10 elementos tarda menos de un microsegundo, demasiado
        //    poco para el reloj. Por eso, en tamaños pequeños, cada repetición
        //    ordena un LOTE de copias y el tiempo se divide entre el tamaño del lote.
        const int lote = static_cast<int>(std::max(1LL, ELEMENTOS_POR_LOTE / n));

        std::cout << "--- n = " << formatoMiles(n) << " (lote de " << lote << ") ---" << std::endl;

        for (Algoritmo& a : algoritmos) {
            Resultado r{n, a.nombre, "", 0.0, 0.0, 0.0, 0, lote, ""};

            // 3) ¿Se omite? (si un tamaño anterior superó el límite o la estimación lo supera)
            double estimadoLoteMs = estimarMs(a, n) * lote;
            if (a.detenido) {
                r.estado = "OMITIDO";
                r.nota = "Un tamano menor ya fue omitido o supero el limite";
            } else if (estimadoLoteMs > limiteMs) {
                r.estado = "OMITIDO";
                r.nota = "Tiempo estimado (~" + std::to_string(static_cast<long long>(estimadoLoteMs / 1000))
                         + " s) supera el limite";
                a.detenido = true;
            }
            if (r.estado == "OMITIDO") {
                std::cout << "  " << std::left << std::setw(15) << a.nombre << " OMITIDO (" << r.nota << ")" << std::endl;
                resultados.push_back(r);
                continue;
            }

            // 4) Número de repeticiones: hasta REPETICIONES_MAXIMAS, pero sin pasar
            //    (según la estimación) el límite de tiempo total de la prueba.
            int repeticiones = REPETICIONES_MAXIMAS;
            if (estimadoLoteMs > 0) {
                repeticiones = std::max(1, std::min(REPETICIONES_MAXIMAS, static_cast<int>(limiteMs / estimadoLoteMs)));
            }

            double sumaMs = 0.0, minMs = 1e300, maxMs = 0.0;
            int completadas = 0;
            bool superoLimite = false;

            for (int rep = 0; rep < repeticiones; rep++) {
                // Copias para este lote (FUERA del cronómetro).
                std::vector<std::vector<int>> copias(lote, original);

                auto inicio = std::chrono::steady_clock::now();
                for (int k = 0; k < lote; k++) {
                    a.funcion(copias[k]);
                }
                auto fin = std::chrono::steady_clock::now();

                double loteMs = std::chrono::duration<double, std::milli>(fin - inicio).count();

                // Verificación (FUERA del cronómetro).
                for (int k = 0; k < lote; k++) {
                    if (!estaOrdenado(copias[k])) {
                        std::cerr << "ERROR: " << a.nombre << " no ordeno correctamente n=" << n << std::endl;
                        return 1;
                    }
                }

                double msPorSort = loteMs / lote;
                sumaMs += msPorSort;
                minMs = std::min(minMs, msPorSort);
                maxMs = std::max(maxMs, msPorSort);
                completadas++;

                // Si una ejecución superó el límite, no repetimos más y
                // omitimos los tamaños más grandes de este algoritmo.
                if (loteMs > limiteMs) {
                    superoLimite = true;
                    break;
                }
            }

            r.promedioMs = sumaMs / completadas;   // promedio de ejecuciones COMPLETADAS
            r.minimoMs = minMs;
            r.maximoMs = maxMs;
            r.repeticiones = completadas;
            if (superoLimite) {
                r.estado = "SUPERO_LIMITE";
                r.nota = "La ejecucion termino pero tardo mas que el limite";
                a.detenido = true;
            } else {
                r.estado = "COMPLETADO";
            }
            a.ultimoN = n;
            a.ultimoMs = r.promedioMs;

            std::cout << "  " << std::left << std::setw(15) << a.nombre << std::right << std::setw(14)
                      << r.promedioMs << " ms  (" << completadas << " rep.)"
                      << (superoLimite ? "  SUPERO EL LIMITE" : "") << std::endl;
            resultados.push_back(r);
        }
    }

    // ===== Guardar resultados en CSV =====
    std::ofstream csv(RUTA_CSV);
    if (!csv) {
        std::cerr << "No se pudo crear " << RUTA_CSV << ". Ejecuta el programa desde la raiz del proyecto." << std::endl;
        return 1;
    }
    csv << "Tamano,Algoritmo,Estado,PromedioMs,MinimoMs,MaximoMs,Repeticiones,Lote,Nota\n";
    csv << std::setprecision(6);
    for (const Resultado& r : resultados) {
        csv << r.n << "," << r.algoritmo << "," << r.estado << ",";
        if (r.estado != "OMITIDO") {
            csv << r.promedioMs << "," << r.minimoMs << "," << r.maximoMs;
        } else {
            csv << ",,"; // sin tiempo: NUNCA inventamos uno
        }
        csv << "," << r.repeticiones << "," << r.lote << "," << r.nota << "\n";
    }
    csv.close();

    std::ofstream config(RUTA_CONFIG);
    config << "Clave,Valor\n";
    config << "Semilla," << SEMILLA << "\n";
    config << "Rango de valores,[" << VALOR_MINIMO << " - " << VALOR_MAXIMO << "]\n";
    config << "Limite por prueba (s)," << limiteS << "\n";
    config << "Repeticiones maximas," << REPETICIONES_MAXIMAS << "\n";
    config << "Elementos por lote," << ELEMENTOS_POR_LOTE << "\n";
    config << "Compilador,GCC " << __VERSION__ << "\n";
#ifdef __OPTIMIZE__
    config << "Optimizacion,Activada (-O2)\n";
#else
    config << "Optimizacion,Desactivada\n";
#endif
    config << "Reloj,std::chrono::steady_clock\n";
    config.close();

    std::cout << "\nResultados guardados en " << RUTA_CSV << std::endl;

    // ===== Generar el Excel =====
    if (generarExcel) {
        std::cout << "Generando results/resultados_sort.xlsx con Excel..." << std::endl;
        int codigo = std::system("powershell -NoProfile -ExecutionPolicy Bypass -File scripts\\generar_excel.ps1");
        if (codigo != 0) {
            std::cerr << "No se pudo generar el Excel (codigo " << codigo << ")." << std::endl;
            return 1;
        }
    }
    return 0;
}
