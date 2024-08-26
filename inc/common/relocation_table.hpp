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

    int getTableSize() {
      return table.size();
    }

    vector<RelocationTableEntry*> getTable() {
      return table;
    }

    static void extractRelocationTable(ifstream &input, int fileNo, int sectionNdx);

    static vector<unordered_map<int, RelocationTable*>> getFileNoToRelocationTablesArray() {
      return fileNoToRelocationTablesArray;
    }
    
    void updateSectionIndex(int newSectionIndex) {
      for(auto& entry : table) {
        entry->sectionNdx = newSectionIndex;
      }
    }

    // only linker uses this
    void concatenateAnotherRelocationTable(RelocationTable *table) {
      for(auto& entry : table->table) {
        this->table.push_back(entry);
      }
    }

    void increaseEntryOffsets(int offset) {
      for(auto& entry : table) {
        entry->offset += offset;
      }
    }

    ~RelocationTable() {}

  private:

    vector<RelocationTableEntry*> table;

    static vector<unordered_map<int, RelocationTable*>> fileNoToRelocationTablesArray;
    static void addRelocationTableToArray(int fileNo, RelocationTable *table, int sectionNdx) {
      if(fileNoToRelocationTablesArray.size() <= (size_t)fileNo) {
        fileNoToRelocationTablesArray.resize(fileNo + 1);
      }
      fileNoToRelocationTablesArray[fileNo][sectionNdx] = table;
    }
};

#endif