# Makefile del proyecto
# En Windows se ejecuta con:  mingw32-make <objetivo>
#
#   mingw32-make demo     -> compila y ejecuta la demostración (src/main.cpp)
#   mingw32-make test     -> compila y ejecuta las pruebas (tests/test_sorting.cpp)
#   mingw32-make benchmark -> mide los tiempos y genera results/resultados_sort.xlsx
#   mingw32-make excel    -> regenera el Excel desde el CSV, sin volver a medir
#   mingw32-make clean    -> borra los ejecutables generados

CXX      = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -Iinclude
# -static: incluye la biblioteca de C++ DENTRO del .exe, para que no
# dependa de las DLL (libstdc++-6.dll) que haya en el PATH. Otros programas
# instalados (por ejemplo Icarus Verilog) traen versiones incompatibles.
LDFLAGS  = -static

# Todos los .cpp de src/ EXCEPTO main.cpp: son los algoritmos y utilidades,
# compartidos por la demo, las pruebas y el benchmark.
ALGO_SRCS = $(filter-out src/main.cpp, $(wildcard src/*.cpp))
HEADERS   = $(wildcard include/*.h)

# Comando para borrar archivos según el sistema operativo
ifeq ($(OS),Windows_NT)
    RM = del /Q /F
else
    RM = rm -f
endif

.PHONY: all demo test benchmark excel clean

all: demo.exe tests.exe benchmark.exe

benchmark.exe: benchmarks/benchmark.cpp $(ALGO_SRCS) $(HEADERS)
	$(CXX) $(CXXFLAGS) -o $@ benchmarks/benchmark.cpp $(ALGO_SRCS) $(LDFLAGS)

# Ejecuta el benchmark completo (mide tiempos, crea CSV y Excel).
benchmark: benchmark.exe
	./benchmark.exe

# Vuelve a crear el Excel a partir del CSV existente (sin medir de nuevo).
excel:
	powershell -NoProfile -ExecutionPolicy Bypass -File scripts/generar_excel.ps1

demo.exe: src/main.cpp $(ALGO_SRCS) $(HEADERS)
	$(CXX) $(CXXFLAGS) -o $@ src/main.cpp $(ALGO_SRCS) $(LDFLAGS)

tests.exe: tests/test_sorting.cpp $(ALGO_SRCS) $(HEADERS)
	$(CXX) $(CXXFLAGS) -o $@ tests/test_sorting.cpp $(ALGO_SRCS) $(LDFLAGS)

demo: demo.exe
	./demo.exe

test: tests.exe
	./tests.exe

clean:
	-$(RM) *.exe
