#include "../../inc/common/section.hpp"

unordered_map<int, vector<Section *>> Section::fileNoToSectionsMap = unordered_map<int, vector<Section *>>();

Section::Section(string name, int ndx) {
  this->name = name;
  this->ndx = ndx;
  this->relocationTable = new RelocationTable();
  this->locationCounter = 0;
  this->size = 0;
}

void Section::addByteToSectionContent(int8_t content) {
  this->content.push_back(content);
  this->locationCounter++;
}

void Section::addQuadbyteToSectionContent(int32_t content) {
  this->content.push_back(content & 0xFF);
  this->content.push_back((content >> 8) & 0xFF);
  this->content.push_back((content >> 16) & 0xFF);
  this->content.push_back((content >> 24) & 0xFF);
  this->locationCounter += 4;
}

void Section::addInstructionToSectionContent(int32_t content) {
  this->content.push_back((content >> 24) & 0xFF);
  this->content.push_back((content >> 16) & 0xFF);
  this->content.push_back((content >> 8) & 0xFF);
  this->content.push_back(content & 0xFF);
  this->locationCounter += 4;
}

void Section::addQuadbyteToSectionContentWithOffset(int32_t content, int offset) {
  this->content[offset] = content & 0xFF;
  this->content[offset + 1] = (content >> 8) & 0xFF;
  this->content[offset + 2] = (content >> 16) & 0xFF;
  this->content[offset + 3] = (content >> 24) & 0xFF;
}

void Section::changeDisplacementInInstruction(int32_t content, int offset) {
  //we are only changing the last 12 bits of the instruction
  this->content[offset + 2] = (content >> 8) & 0x0F;
  this->content[offset + 3] = content & 0xFF;
}

void Section::addLiteralToPool(int literal) {
  if(literalPool.find(literal) == literalPool.end()) {
    literalPool[literal] = vector<int>();
  }
  literalPool[literal].push_back(this->locationCounter);
}

void Section::addSymbolOffset(int symbol, int offset) {
  if(symbolOffsets.find(symbol) == symbolOffsets.end()) {
    symbolOffsets[symbol] = vector<int>();
  }
  symbolOffsets[symbol].push_back(offset);
}

void Section::extractSection(ifstream &input, int fileNo, string line) {
  istringstream iss(line);
  string sectionName;
  string hexNum;
  int sectionNdx;
  int numberOfBytesToRead;

  iss >> sectionName;
  iss >> hexNum;
  numberOfBytesToRead = stoi(hexNum, nullptr, 16);
  int sectionSize = numberOfBytesToRead;

  iss >> hexNum;
  sectionNdx = stoi(hexNum, nullptr, 16);

  Section *section = new Section(sectionName, sectionNdx, sectionSize);

  int8_t byte;

  while(getline(input, line)) {
    if(line.empty()) {
      break;
    }
    iss >> hexNum;
    byte = stoi(hexNum, nullptr, 16);

    section->addByteToSectionContent(byte);

    numberOfBytesToRead--;
    if(numberOfBytesToRead == 0) {
      break;
    }
  }

  addSectionToMap(section, fileNo);
}

void Section::concatenateAnotherSectionsContent(vector<uint8_t> content) {
  for(auto& byte : content) {
    this->content.push_back(byte);
    this->locationCounter++;
  }
}
