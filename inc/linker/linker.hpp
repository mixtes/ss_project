#ifndef _LINKER_H
#define _LINKER_H

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
using namespace std;

#include "../common/code_component.hpp"

struct UnresolvedExtern {
  SymbolTableEntry *symbolTableEntry;
  int fileNo;
};

class Linker {
  public:

    static Linker* getInstance() {
      static Linker instance;
      return &instance;
    }

    int link(vector<string> input_files, string output_file, unordered_map<string, uint32_t> &sectionAddresses, bool hex_output, bool relocatable);

    ~Linker() {}

  private:

    bool hex_output;
    bool relocatable;

    static SymbolTable *symbolTable;

    Linker() {}
    Linker(Linker const&) = delete;
    void operator=(Linker const&) = delete;

    unordered_map<string, uint32_t> sectionAddresses;
    vector<uint32_t> predeterminedSectionStarts;
    vector<uint32_t> predeterminedSectionEnds;

    void checkForSectionAddressInterference(uint32_t start, uint32_t end);

    unordered_map<int, SymbolTable *> symbolTableMap;
    unordered_map<int, vector<Section *>> sectionsMap;
    unordered_map<int, vector<RelocationTable *>> relocationTableMap;

    unordered_map<string, Section *> combinedSections;
    unordered_map<string, int> sectionIndices; // section name -> index in combinedSections
    unordered_map<int, unordered_map<int, int>> fileToSectionToCombinedSection; // fileNo -> section index -> combined section index

    unordered_map<string, RelocationTable *> combinedRelocationTables; // section name -> combined relocation table
    unordered_map<int, unordered_map<int, int>> fileToSectionToCombinedRelocationTableStart; // fileNo -> section index -> combined relocation table start index
    unordered_map<int, unordered_map<int, int>> fileToSectionToCombinedRelocationTableEnd; // fileNo -> section index -> combined relocation table end index

    unordered_map<int, unordered_map<int, int>> fileToSectionToItsAddressInCombinedSection; // fileNo -> section index -> address in combined section

    unordered_map<string, vector<UnresolvedExtern *>> unresolvedExterns;

    void combineSectionsWithSameNamesAndTheirRelocationTables();
    int combineAllSymbolTables();

    bool checkIfSymbolIsInUnresolvedExterns(string symbol);
    void addUnresolvedExtern(UnresolvedExtern *unresolvedExtern);

    void updateRelocationTablesSymbolIndices(int oldSymbolIndex, int newSymbolIndex, int fileNo, int sectionIndex);

    void combineSectionsAndRelTabs();
    void calculateAndSetFinalAddressesForSections();
    void addSectionsToSymbolTable();

    int finalizeUnresolvedExterns();

    void completeRelocations();
    void printHexOutput(ofstream &output);
};

#endif