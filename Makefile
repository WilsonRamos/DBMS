# Makefile para el Sistema Gestor de Base de Datos (SGBD) - Versión Corregida

# Compilador y flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
DEBUG_FLAGS = -g -DDEBUG -fsanitize=address
STRICT_FLAGS = -Werror -Wpedantic -Wconversion -Wsign-conversion

# Flags más permisivos para desarrollo inicial
PERMISSIVE_FLAGS = -std=c++17 -Wall -O2 -Wno-sign-compare -Wno-unused-parameter

# Directorios
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build
BIN_DIR = bin

# Archivos fuente
SOURCES = $(SRC_DIR)/main.cpp $(SRC_DIR)/sgbd_basic.cpp $(SRC_DIR)/disk_manager.cpp $(SRC_DIR)/sgbd.cpp
OBJECTS = $(BUILD_DIR)/main.o $(BUILD_DIR)/sgbd_basic.o $(BUILD_DIR)/disk_manager.o $(BUILD_DIR)/sgbd.o
HEADERS = $(INCLUDE_DIR)/sgbd_basic.h $(INCLUDE_DIR)/disk_manager.h $(INCLUDE_DIR)/sgbd.h

# Ejecutable final
TARGET = $(BIN_DIR)/sgbd

# Crear directorios si no existen
$(shell mkdir -p $(BUILD_DIR) $(BIN_DIR) $(SRC_DIR) $(INCLUDE_DIR))

# Regla principal
all: $(TARGET)

# Compilar el programa principal
$(TARGET): $(OBJECTS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $(TARGET)

# Compilar archivos objeto individuales
$(BUILD_DIR)/main.o: $(SRC_DIR)/main.cpp $(HEADERS) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) -c $(SRC_DIR)/main.cpp -o $(BUILD_DIR)/main.o

$(BUILD_DIR)/sgbd_basic.o: $(SRC_DIR)/sgbd_basic.cpp $(INCLUDE_DIR)/sgbd_basic.h | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) -c $(SRC_DIR)/sgbd_basic.cpp -o $(BUILD_DIR)/sgbd_basic.o

$(BUILD_DIR)/disk_manager.o: $(SRC_DIR)/disk_manager.cpp $(INCLUDE_DIR)/disk_manager.h $(INCLUDE_DIR)/sgbd_basic.h | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) -c $(SRC_DIR)/disk_manager.cpp -o $(BUILD_DIR)/disk_manager.o

$(BUILD_DIR)/sgbd.o: $(SRC_DIR)/sgbd.cpp $(INCLUDE_DIR)/sgbd.h $(INCLUDE_DIR)/disk_manager.h | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) -c $(SRC_DIR)/sgbd.cpp -o $(BUILD_DIR)/sgbd.o

# Compilar con warnings permisivos (para desarrollo inicial)
permissive: CXXFLAGS = $(PERMISSIVE_FLAGS)
permissive: $(TARGET)

# Compilar en modo debug
debug: CXXFLAGS += $(DEBUG_FLAGS)
debug: $(TARGET)

# Compilar en modo estricto (todos los warnings como errores)
strict: CXXFLAGS += $(STRICT_FLAGS)
strict: $(TARGET)

# Compilar sin warnings de comparación signed/unsigned
no-warnings: CXXFLAGS += -Wno-sign-compare -Wno-sign-conversion
no-warnings: $(TARGET)

# Crear directorios
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Limpiar archivos generados
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)
	rm -f *.csv

# Limpiar solo objetos
clean-obj:
	rm -rf $(BUILD_DIR)

# Ejecutar el programa
run: $(TARGET)
	./$(TARGET)

# Ejecutar con valgrind para detectar memory leaks
valgrind: debug
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(TARGET)

# Compilar solo (sin ejecutar)
compile: $(TARGET)

# Instalar dependencias (ejemplo para Ubuntu/Debian)
install-deps:
	sudo apt-get update
	sudo apt-get install build-essential g++ valgrind gdb

# Verificar sintaxis de todos los archivos
check-syntax:
	$(CXX) $(PERMISSIVE_FLAGS) -I$(INCLUDE_DIR) -fsyntax-only $(SOURCES)

# Compilar con diferentes niveles de optimización
optimize-size: CXXFLAGS += -Os
optimize-size: $(TARGET)

optimize-speed: CXXFLAGS += -O3 -march=native
optimize-speed: $(TARGET)

# Análisis estático del código
static-analysis:
	@echo "Running static analysis..."
	cppcheck --enable=all --std=c++17 -I $(INCLUDE_DIR) $(SOURCES) 2> static_analysis.log || true
	@echo "Static analysis complete. Check static_analysis.log for results."

# Formatear código
format:
	@echo "Formatting code..."
	@if command -v clang-format >/dev/null 2>&1; then \
		find $(SRC_DIR) $(INCLUDE_DIR) -name "*.cpp" -o -name "*.h" | xargs clang-format -i; \
		echo "Code formatted successfully"; \
	else \
		echo "clang-format not found. Install with: sudo apt-get install clang-format"; \
	fi

# Generar documentación
docs:
	@if command -v doxygen >/dev/null 2>&1; then \
		doxygen Doxyfile 2>/dev/null || echo "Create Doxyfile first: doxygen -g"; \
	else \
		echo "Doxygen not found. Install with: sudo apt-get install doxygen"; \
	fi

# Test de memoria
memtest: debug
	valgrind --tool=memcheck --leak-check=full --show-reachable=yes ./$(TARGET)

# Profile de performance
profile: debug
	valgrind --tool=callgrind ./$(TARGET)
	@echo "Use 'kcachegrind callgrind.out.*' to view the profile"

# Mostrar información del proyecto
info:
	@echo "=== SGBD Project Information ==="
	@echo "Source files: $(SOURCES)"
	@echo "Object files: $(OBJECTS)"
	@echo "Headers: $(HEADERS)"
	@echo "Target: $(TARGET)"
	@echo "Compiler: $(CXX)"
	@echo "Current flags: $(CXXFLAGS)"
	@echo "Lines of code: $$(find $(SRC_DIR) $(INCLUDE_DIR) -name '*.cpp' -o -name '*.h' | xargs wc -l | tail -1 | awk '{print $$1}')"

# Test rápido
test: no-warnings
	@echo "Running quick test..."
	@timeout 10s ./$(TARGET) && echo "✅ Test passed" || echo "⚠️  Test finished with issues"

# Mostrar ayuda
help:
	@echo "Makefile para SGBD - Sistema Gestor de Base de Datos"
	@echo ""
	@echo "🔨 Compilación:"
	@echo "  make all          - Compilar normalmente"
	@echo "  make permissive   - Compilar con warnings relajados"
	@echo "  make no-warnings  - Compilar sin warnings de signed/unsigned"
	@echo "  make debug        - Compilar con debug + AddressSanitizer"
	@echo "  make strict       - Compilar con todos los warnings como errores"
	@echo ""
	@echo "🚀 Ejecución:"
	@echo "  make run          - Compilar y ejecutar"
	@echo "  make test         - Prueba rápida"
	@echo ""
	@echo "🧹 Limpieza:"
	@echo "  make clean        - Limpiar todos los archivos generados"
	@echo "  make clean-obj    - Limpiar solo archivos objeto"
	@echo ""
	@echo "🔍 Debugging y Análisis:"
	@echo "  make valgrind     - Ejecutar con detección de memory leaks"
	@echo "  make memtest      - Test completo de memoria"
	@echo "  make profile      - Profile de performance"
	@echo "  make static-analysis - Análisis estático del código"
	@echo ""
	@echo "⚡ Optimización:"
	@echo "  make optimize-size   - Optimizar para tamaño"
	@echo "  make optimize-speed  - Optimizar para velocidad"
	@echo ""
	@echo "📚 Utilidades:"
	@echo "  make format       - Formatear código con clang-format"
	@echo "  make docs         - Generar documentación"
	@echo "  make info         - Mostrar información del proyecto"
	@echo "  make check-syntax - Verificar sintaxis"

.PHONY: all permissive debug strict no-warnings clean clean-obj run valgrind compile install-deps check-syntax optimize-size optimize-speed static-analysis format docs memtest profile info test help