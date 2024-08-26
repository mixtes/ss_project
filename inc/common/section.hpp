#ifndef _SECTION_H
#define _SECTION_H

#include <string>
#include <vector>
#include <unordered_map>
#include "relocation_table.hpp"
using namespace std;

class Section {
  public:
    Section(string name, int ndx);

    //only linker uses this
    Section(string name, int ndx, int size) : Section(name, ndx) {
      this->size = size;
      this->locationCounter = size;
    }

    void addByteToSectionContent(int8_t content);

    void addQuadbyteToSectionContent(int32_t content);

    void addInstructionToSectionContent(int32_t content);

    void addQuadbyteToSectionContentWithOffset(int32_t content, uint32_t offset);

    void changeDisplacementInInstruction(int32_t content, uint32_t offset);

    string getName() {
      return this->name;
    }

    int getNdx() {
      return this->ndx;
    }

    int getLocationCounter() {
      return this->locationCounter;
    }

    int getSize() {
      return this->size;
    }

    //only linker uses this
    void increaseSize(int size) {
      this->size += size;
    }

    //only linker uses this
    void concatenateAnotherSectionsContent(vector<uint8_t> content);

    //only linker uses this
    void setRelocationTable(RelocationTable *table) {
      delete this->relocationTable;
      this->relocationTable = table;
    }

    RelocationTable *getRelocationTable() {
      return this->relocationTable;
    }

    unordered_map<int, vector<int>> getLiteralPool() {
      return this->literalPool;
    }

    unordered_map<int, vector<int>> getSymbolOffsets() {
      return this->symbolOffsets;
    }

    vector<int> getOffsetsForLiteral(int literal) {
      return this->literalPool[literal];
    }

    vector<int> getOffsetsForSymbol(int symbol) {
      return this->symbolOffsets[symbol];
    }

    void addLiteralToPool(int literal);

    void addSymbolOffset(int symbol, int offset);

    vector<uint8_t> getContent() {
      return this->content;
    }

    static int extractSection(ifstream &input, int fileNo, string line);

    static vector<unordered_map<int, Section *>> getFileNoToSectionsArray() {
      return fileNoToSectionsArray;
    }

    static vector<vector<int>> getFileNoToSectionIndexArray() {
      return fileNoToSectionIndexArray;
    }

    void setSectionFinalAddress(uint32_t address) {
      this->sectionFinalAddress = address;
    }

    uint32_t getSectionFinalAddress() {
      return this->sectionFinalAddress;
    }

    ~Section() { delete relocationTable; }

  private:
    string name;
    int ndx;
    int locationCounter;
    int size;

    vector<uint8_t> content;

    RelocationTable *relocationTable;
    unordered_map<int, vector<int>> literalPool; // <literal value, vector of offsets>
    unordered_map<int, vector<int>> symbolOffsets; // <symbol index in SymbolTable, vector of offsets>

    static vector<unordered_map<int, Section *>> fileNoToSectionsArray;
    static vector<vector<int>> fileNoToSectionIndexArray;

    static void addSectionToArray(Section *section, int fileNo) {
      if(fileNoToSectionsArray.size() <= (size_t)fileNo) {
        fileNoToSectionsArray.resize(fileNo + 1);
      }
      fileNoToSectionsArray[fileNo][section->getNdx()] = section;
    }

    static void addSectionIndexToArray(int sectionIndex, int fileNo) {
      if(fileNoToSectionIndexArray.size() <= (size_t)fileNo) {
        fileNoToSectionIndexArray.resize(fileNo + 1);
      }
      fileNoToSectionIndexArray[fileNo].push_back(sectionIndex);
    }

    //either user determines the final address or linker does (unnecessary for assembler)
    uint32_t sectionFinalAddress;
};

#endif