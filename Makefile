# Makefile proyecto Huffman

CC        := gcc
CFLAGS    := -std=c11 -D_GNU_SOURCE -D_POSIX_C_SOURCE=199309L -O2
LDFLAGS   := -lpthread

# Nombre del ejecutable pthread
PROG_PTHREAD := compress_pthread

# Solo el fuente de la versión pthread (que incluye compress.c)
SRCS_PTHREAD := compress_pthread.c

# Carpetas de entrada con los 100 libros
INPUT_DIRS := parte1 parte2

.PHONY: all clean run-pthread

all: $(PROG_PTHREAD)
	@echo " Ejecutable listo: ./$(PROG_PTHREAD)"
	@echo "  Usa 'make run-pthread' para comprimir todo"

# Compila la versión pthread
$(PROG_PTHREAD): $(SRCS_PTHREAD)
	@echo "Compilando mi pthread..."
	$(CC) $(CFLAGS) -o $@ $(SRCS_PTHREAD) $(LDFLAGS)
	@echo "  → $@ compilado correctamente"

# Borra binarios y salidas viejas
clean:
	@echo "Limpiando binarios y carpeta salida/"
	rm -f $(PROG_PTHREAD)
	rm -rf salida
	@echo "  → Limpieza completa"

# Corre la version pthread contra todos los .txt
run-pthread: clean $(PROG_PTHREAD)
	@echo "Creando carpeta de salida/ y lanzando hilos..."
	mkdir -p salida
	@echo " Procesando parte1 y parte2"
	./$(PROG_PTHREAD) parte1/*.txt parte2/*.txt
	@echo "Todo listo en ./salida/"
