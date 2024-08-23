#ifndef _LINKER_H
#define _LINKER_H

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
using namespace std;

#include "../common/symbol_table.hpp"
#include "../common/section.hpp"
#include "../common/relocation_table.hpp"

class Linker {
  public:

    static Linker* getInstance() {
      static Linker instance;
      return &instance;
    }

    int link(vector<string> input_files, string output_file, unordered_map<string, uint32_t> &sectionAddresses, bool hex_output, bool relocatable);

    ~Linker() {}

  private:

    static SymbolTable *symbolTable;

    Linker() {}
    Linker(Linker const&) = delete;
    void operator=(Linker const&) = delete;

    unordered_map<string, uint32_t> sectionAddresses;

    unordered_map<int, SymbolTable *> symbolTableMap;
    unordered_map<int, vector<Section *>> sectionsMap;
    unordered_map<int, vector<RelocationTable *>> relocationTableMap;

    unordered_map<string, Section *> combinedSections;
    unordered_map<string, int> sectionIndices; // section name -> index in combinedSections
    unordered_map<int, unordered_map<int, int>> fileToSectionToCombinedSection; // fileNo -> section index -> combined section index

    unordered_map<int, unordered_map<int, int>> fileToSectionToItsAddressInCombinedSection; // fileNo -> section index -> address in combined section

    void combineSectionsWithSameNames();
    int combineAllSymbolTables();
};

#endif