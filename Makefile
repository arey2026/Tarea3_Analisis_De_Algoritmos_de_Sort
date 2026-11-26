# Makefile del proyecto
# En Windows se ejecuta con:  mingw32-make <objetivo>
#
#   mingw32-make demo     -> compila y ejecuta la demostración (src/main.cpp)
#   mingw32-make test     -> compila y ejecuta las pruebas (tests/test_sorting.cpp)
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

.PHONY: all demo test clean

all: demo.exe tests.exe

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
