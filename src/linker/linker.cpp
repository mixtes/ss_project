#include "../../inc/linker/linker.hpp"

SymbolTable *Linker::symbolTable = SymbolTable::getInstance();

int Linker::link(vector<string> input_files, string output_file, unordered_map<string, uint32_t> &sectionAddresses, bool hex_output, bool relocatable) {

  int fileNo = 0;

  for(auto& file : input_files) {
    ifstream input(file);
    if(!input.is_open()) {
      cout << "Error: Could not open file " << file << endl;
      return 1;
    }

    string line;

    while(getline(input, line)) {
      if(line == "SYMT") {
        SymbolTable::extractSymbolTable(input, fileNo);
      }
      else if(line == "RELT") {
        RelocationTable::extractRelocationTable(input, fileNo);
      }
      else { // Section
        Section::extractSection(input, fileNo, line);
      }
    } 
    input.close();
    fileNo++;
  }

  this->sectionAddresses = sectionAddresses;

  symbolTableMap = SymbolTable::getFileNoToSymbolTable();
  sectionsMap = Section::getFileNoToSectionsMap();
  relocationTableMap = RelocationTable::getFileNoToRelocationTablesMap();

  int check;
  combineSectionsWithSameNames();
  check = combineAllSymbolTables();
  if(check < 0) {
    return 1;
  }

  return 0;
}

void Linker::combineSectionsWithSameNames() {
  int nextFileNo = 0;
  vector<Section *> sections;
  while(sectionsMap.find(nextFileNo) != sectionsMap.end()) {
    sections = sectionsMap[nextFileNo];
    for(auto& section : sections) {
      if(combinedSections.find(section->getName()) == combinedSections.end()) {
        combinedSections[section->getName()] = section;
        sectionIndices[section->getName()] = combinedSections.size() - 1;
        fileToSectionToCombinedSection[nextFileNo][section->getNdx()] = sectionIndices[section->getName()];

        fileToSectionToItsAddressInCombinedSection[nextFileNo][section->getNdx()] = 0;
      }
      else {
        Section *combinedSection = combinedSections[section->getName()];
        combinedSection->concatenateAnotherSectionsContent(section->getContent());
        fileToSectionToCombinedSection[nextFileNo][section->getNdx()] = sectionIndices[section->getName()];

        fileToSectionToItsAddressInCombinedSection[nextFileNo][section->getNdx()] = combinedSection->getSize();
        combinedSection->increaseSize(section->getSize());
      }
    }
    nextFileNo++;
  }

  int maxPredeterminedAddress = 0;
  int placeForNextNonPredeterminedSection = 0;
  for(auto& combinedSection : combinedSections) {
    string sectionName = combinedSection.first;
    if(sectionAddresses.find(sectionName) != sectionAddresses.end()) {
      combinedSection.second->setSectionFinalAddress(sectionAddresses[sectionName]);

      // determine the starting place of sections without predetermined addresses
      if(sectionAddresses[sectionName] > maxPredeterminedAddress) {
        maxPredeterminedAddress = sectionAddresses[sectionName];
        placeForNextNonPredeterminedSection = maxPredeterminedAddress + combinedSection.second->getSize();
      }
    }
  }
  
  for(auto& combinedSection : combinedSections) {
    string sectionName = combinedSection.first;
    if(sectionAddresses.find(sectionName) == sectionAddresses.end()) {
      combinedSection.second->setSectionFinalAddress(placeForNextNonPredeterminedSection);
      placeForNextNonPredeterminedSection += combinedSection.second->getSize();
    }
  }

  SymbolTable *symbolTable = SymbolTable::getInstance();
  for(auto& combinedSection : combinedSections) {
    SymbolTableEntry *newEntry = new SymbolTableEntry();
    newEntry->name = combinedSection.first;
    newEntry->value = combinedSection.second->getSectionFinalAddress();
    newEntry->isDefined = true;
    newEntry->isSection = true;
    symbolTable->addEntry(newEntry);
  }
}

int Linker::combineAllSymbolTables() {

  int nextFileNo = 0;
  //static table will be used as the main table
  SymbolTable *bigSymbolTable = SymbolTable::getInstance();
  int nextSymbolIndex = bigSymbolTable->getTable().size(); // it won't be 0 because of the sections

  while(symbolTableMap.find(nextFileNo) != symbolTableMap.end()) {
    SymbolTable *smallSymbolTable = symbolTableMap[nextFileNo];
    for(auto& symbolTableEntry : smallSymbolTable->getTable()) {
      
      if(symbolTableEntry->isSection) {
        continue;
      }

      if(bigSymbolTable->getEntry(symbolTableEntry->name) != nullptr) {
        if(bigSymbolTable->getEntry(symbolTableEntry->name)->global && symbolTableEntry->global) {
          cout << "Error: Multiple global symbols with the same name. " << symbolTableEntry->name << endl;
          return -1;
        }
        smallSymbolTable->changeSymbolNameToMakeItUnique(symbolTableEntry);
      }

      SymbolTableEntry *newEntry = new SymbolTableEntry();
      //index is given to an entry automatically in addEntry function
      newEntry->name = symbolTableEntry->name;
      newEntry->value = symbolTableEntry->value;
      newEntry->global = symbolTableEntry->global;
      bigSymbolTable->addEntry(newEntry);
    }
  }

  return 0;
}