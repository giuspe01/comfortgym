# Makefile - Comfortgym (caso di studio ITPS, traccia 2)
#
# Target principali:
#   make            -> compila l'eseguibile palestra(.exe)
#   make run        -> compila e avvia il server
#   make clean      -> rimuove eseguibile e oggetti
#
# Requisiti: g++ con supporto C++17 (testato con MinGW-w64 GCC 15.2).

# --- Rilevamento sistema operativo ------------------------------------------

ifeq ($(OS),Windows_NT)
    TARGET   := palestra.exe
    LDLIBS   := -lws2_32
    LDSTATIC := -static -static-libgcc -static-libstdc++
    MKDIR     = if not exist "$(subst /,\,$1)" mkdir "$(subst /,\,$1)"
    RM_DIR    = if exist "$(subst /,\,$1)" rmdir /s /q "$(subst /,\,$1)"
    RM_FILE   = if exist "$(subst /,\,$1)" del /q /f "$(subst /,\,$1)"
    RUN_CMD   = $(TARGET)
else
    TARGET   := palestra
    LDLIBS   := -lpthread -ldl
    LDSTATIC :=
    MKDIR     = mkdir -p $1
    RM_DIR    = rm -rf $1
    RM_FILE   = rm -f $1
    RUN_CMD   = ./$(TARGET)
endif

# --- Configurazione compilazione --------------------------------------------

CXX       := g++
CC        := gcc
CXXSTD    := -std=c++17
WARN      := -Wall -Wextra -Wpedantic
OPT       := -O2
INCLUDES  := -Ithird_party -Isrc

# Flag specifici per SQLite (riducono la dimensione, evitano feature non usate)
SQLITE_DEFS := -DSQLITE_THREADSAFE=1 \
               -DSQLITE_OMIT_LOAD_EXTENSION \
               -DSQLITE_OMIT_DEPRECATED

CXXFLAGS := $(CXXSTD) $(WARN) $(OPT) $(INCLUDES)
CFLAGS   := $(OPT) $(SQLITE_DEFS)

# --- Sorgenti ---------------------------------------------------------------

SRC_CPP := $(wildcard src/*.cpp) \
           $(wildcard src/modello/*.cpp) \
           $(wildcard src/persistenza/*.cpp) \
           $(wildcard src/servizi/*.cpp) \
           $(wildcard src/controller/*.cpp) \
           $(wildcard src/utils/*.cpp)

BUILD   := build
OBJ_CPP := $(SRC_CPP:%.cpp=$(BUILD)/%.o)
OBJ_C   := $(BUILD)/third_party/sqlite3.o
OBJ     := $(OBJ_CPP) $(OBJ_C)

# --- Target -----------------------------------------------------------------

.PHONY: all run clean rebuild

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $@ $(LDSTATIC) $(LDLIBS)
	@echo === Compilazione completata: $@ ===

# .cpp -> .o (C++17)
$(BUILD)/%.o: %.cpp
	@$(call MKDIR,$(dir $@))
	$(CXX) $(CXXFLAGS) -c $< -o $@

# sqlite3.c -> sqlite3.o (compilato come C)
$(BUILD)/third_party/sqlite3.o: third_party/sqlite3.c
	@$(call MKDIR,$(dir $@))
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	$(RUN_CMD)

clean:
	@$(call RM_DIR,$(BUILD))
	@$(call RM_FILE,$(TARGET))
	@$(call RM_FILE,data/palestra.db)
	@echo === Pulizia completata ===

rebuild: clean all
