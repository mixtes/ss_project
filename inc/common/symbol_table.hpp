#ifndef _SYMBOL_TABLE_H
#define _SYMBOL_TABLE_H

#include <iostream>
#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <fstream>
using namespace std;

#include "section.hpp"

struct SymbolTableForwardReference {
  int sectionNdx;
  int patchLocation;
  bool word;
  SymbolTableForwardReference *nextReference;
};

struct SymbolTableEntry {
  int index;
  string name = "";
  int value = 0;
  int sectionNdx = 0;
  bool global = false;
  bool absolute = false;
  bool isDefined = false;
  bool isExtern = false;
  bool isSection = false;
  SymbolTableForwardReference *forwardReference = nullptr;
};

class SymbolTable {
  public:

    SymbolTable() {}

    static SymbolTable* getInstance() {
      static SymbolTable instance;
      return &instance;
    }

    int getSize() {
      return table.size();
    }

    void addEntry(SymbolTableEntry *entry) {
      entry->index = table.size();
      table.push_back(entry);
      tableMap[entry->name] = entry;
    }

    SymbolTableEntry* getEntry(string name);

    SymbolTableEntry* getEntry(int index);

    void addForwardReference(string symbol, int patchLocation, int sectionNdx, bool word);

    void clearForwardReferences(SymbolTableEntry *entry);

    void printToOutput(ofstream &output);

    void printCurrentTable();

    static void extractSymbolTable(ifstream &input, int fileNo);

    void changeSymbolNameToMakeItUnique(SymbolTableEntry *entry);

    static unordered_map<int, SymbolTable *> getFileNoToSymbolTable() {
      return linkerSymbolTables;
    }

    vector<SymbolTableEntry*> getTable() {
      return table;
    }

    ~SymbolTable() {}

  private:

    SymbolTable(SymbolTable const&) = delete;
    void operator=(SymbolTable const&) = delete;

    vector<SymbolTableEntry*> table;
    unordered_map<string, SymbolTableEntry*> tableMap;

    vector<Section *> sectionsList;

    static unordered_map<int, SymbolTable *> linkerSymbolTables;

    static void addEntryToLinkerSymbolTable(SymbolTableEntry *entry, int fileNo) {
      linkerSymbolTables[fileNo]->addEntry(entry);
    };

    static int globalUniqueIndex;
};

#endif