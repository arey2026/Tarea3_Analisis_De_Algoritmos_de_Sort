# Makefile del proyecto
# En Windows se ejecuta con:  mingw32-make <objetivo>
#
#   mingw32-make demo     -> compila y ejecuta la demostración (src/main.cpp)
#   mingw32-make clean    -> borra los ejecutables generados

CXX      = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -Iinclude

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

.PHONY: all demo clean

all: demo.exe

demo.exe: src/main.cpp $(ALGO_SRCS) $(HEADERS)
	$(CXX) $(CXXFLAGS) -o $@ src/main.cpp $(ALGO_SRCS)

demo: demo.exe
	./demo.exe

clean:
	-$(RM) *.exe
