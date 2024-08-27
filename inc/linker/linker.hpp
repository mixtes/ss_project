#ifndef _LINKER_H
#define _LINKER_H

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <map>
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

    vector<SymbolTable *> symbolTableArray;
    vector<unordered_map<int, Section *>> sectionsArray;
    vector<unordered_map<int, RelocationTable *>> relocationTableArray;
    vector<vector<int>> fileNoToSectionIndexArray;

    unordered_map<string, Section *> combinedSections;
    vector<Section *> combinedSectionsInOrder; // this is an ordered vector, so the sections will be in the order they were added
    unordered_map<string, int> sectionIndices; // section name -> index in combinedSections
    unordered_map<int, unordered_map<int, int>> fileToSectionToCombinedSection; // fileNo -> section index -> combined section index

    unordered_map<string, RelocationTable *> combinedRelocationTables; // section name -> combined relocation table
    vector<RelocationTable *> combinedRelocationTablesInOrder; // this is an ordered vector, so the relocation tables will be in the order they were added

    unordered_map<int, unordered_map<int, int>> fileToSectionToItsAddressInCombinedSection; // fileNo -> section index -> address in combined section

    unordered_map<string, vector<UnresolvedExtern *>> unresolvedExterns;

    void combineSectionsWithSameNamesAndTheirRelocationTables();
    int combineAllSymbolTables();

    bool checkIfSymbolIsInUnresolvedExterns(string symbol);
    void addUnresolvedExtern(UnresolvedExtern *unresolvedExtern);

    void updateRelocationTablesSymbolIndices(int oldSymbolIndex, int newSymbolIndex, int fileNo);

    void combineSectionsAndRelTabs();
    void calculateAndSetFinalAddressesForSections();
    void addSectionsToSymbolTable();

    int finalizeUnresolvedExterns();

    void commitIndicesInRelocationTables();
    void completeRelocations();
    void printHexOutput(ofstream &output);

    map<uint32_t, Section *> sectionsSortedByAddress;
};

#endif