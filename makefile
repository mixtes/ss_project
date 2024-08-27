# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++11 -Wall -g

# Flex and Bison sources for assembler
LEX_SOURCE = misc/assembly_flex.l
BISON_SOURCE = misc/assembly_bison.y

# Flex and Bison output files
LEX_C_OUTPUT = assembly_flex.yy.cpp
LEX_H_OUTPUT = assembly_flex.yy.hpp
BISON_C_OUTPUT = assembly_bison.tab.cpp
BISON_H_OUTPUT = assembly_bison.tab.hpp

# Object files for assembler
ASSEMBLER_OBJ_FILES = assembly_flex.yy.o assembly_bison.tab.o $(patsubst %.cpp, %.o, $(wildcard src/common/*.cpp src/assembler/*.cpp))

# Object files for linker
LINKER_OBJ_FILES = $(patsubst %.cpp, %.o, $(wildcard src/common/*.cpp src/linker/*.cpp))

# Object files for emulator
EMULATOR_OBJ_FILES = $(patsubst %.cpp, %.o, $(wildcard src/common/*.cpp src/emulator/*.cpp))

# Executable names
ASSEMBLER_EXEC = assembler
LINKER_EXEC = linker
EMULATOR_EXEC = emulator

# Default target to build all executables
all: $(ASSEMBLER_EXEC) $(LINKER_EXEC) $(EMULATOR_EXEC)

# Rule to generate the bison output files (runs before flex)
$(BISON_C_OUTPUT) $(BISON_H_OUTPUT): $(BISON_SOURCE)
	bison -d -o $(BISON_C_OUTPUT) $(BISON_SOURCE)

# Rule to generate the flex output file
$(LEX_C_OUTPUT): $(LEX_SOURCE) $(BISON_H_OUTPUT)
	flex -o $@ $<

# Rule to compile the lex output
assembly_flex.yy.o: $(LEX_C_OUTPUT)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Rule to compile the bison output
assembly_bison.tab.o: $(BISON_C_OUTPUT)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Rule to compile other .cpp files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Linking all object files for assembler
$(ASSEMBLER_EXEC): $(ASSEMBLER_OBJ_FILES)
	$(CXX) $(CXXFLAGS) -o $@ $^ -lfl

# Linking all object files for linker
$(LINKER_EXEC): $(LINKER_OBJ_FILES)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Linking all object files for emulator
$(EMULATOR_EXEC): $(EMULATOR_OBJ_FILES)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Clean up
clean:
	rm -f $(ASSEMBLER_OBJ_FILES) $(LINKER_OBJ_FILES) $(EMULATOR_OBJ_FILES) $(ASSEMBLER_EXEC) $(LINKER_EXEC) $(EMULATOR_EXEC) $(LEX_C_OUTPUT) $(BISON_C_OUTPUT) $(BISON_H_OUTPUT) $(LEX_H_OUTPUT) *.o ./tests/nivo-a/*.o ./tests/nivo-b/*.o *.hex