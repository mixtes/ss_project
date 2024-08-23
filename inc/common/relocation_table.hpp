#ifndef _RELOCATION_TABLE_H
#define _RELOCATION_TABLE_H

#include <iostream>
#include <cstdint>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>
using namespace std;

enum RelocationType {
  REL_TYPE_ABS = 0x00 //seems like this is the only relocation type we need
};

struct RelocationTableEntry {
  int sectionNdx; //section in which the relocation is
  int offset; //offset in the section
  RelocationType relocationType; //type of relocation
  int symbolIndex; //symbol to relocate
};

class RelocationTable {
  public:

    RelocationTable() {}

    void addEntry(int sectionNdx, int offset, RelocationType type, int symbolIndex);

    void printToOutput(ofstream &output);

    static void extractRelocationTable(ifstream &input, int fileNo);

    static unordered_map<int, vector<RelocationTable*>> getFileNoToRelocationTablesMap() {
      return fileNoToRelocationTablesMap;
    }

    ~RelocationTable() {}

  private:

    vector<RelocationTableEntry*> table;

    static unordered_map<int, vector<RelocationTable*>> fileNoToRelocationTablesMap;
    static void addRelocationTableToMap(int fileNo, RelocationTable *table) {
      if(fileNoToRelocationTablesMap.find(fileNo) == fileNoToRelocationTablesMap.end()) {
        fileNoToRelocationTablesMap[fileNo] = vector<RelocationTable*>();
      }
      fileNoToRelocationTablesMap[fileNo].push_back(table);
    }
};

#endif