#include "../../inc/linker/linker.hpp"

SymbolTable *Linker::symbolTable = SymbolTable::getInstance();

int Linker::link(vector<string> input_files, string output_file, unordered_map<string, uint32_t> &sectionAddresses, bool hex_output, bool relocatable) {

  this->hex_output = hex_output;
  this->relocatable = relocatable;
  int fileNo = 0;

  for(auto& file : input_files) {
    ifstream input(file);
    if(!input.is_open()) {
      cout << "Error: Could not open file " << file << endl;
      return 1;
    }

    string line;
    int sectionNdx;

    while(getline(input, line)) {
      if(line == "SYMT") {
        SymbolTable::extractSymbolTable(input, fileNo);
      }
      else if(line == "RELT") {
        RelocationTable::extractRelocationTable(input, fileNo, sectionNdx);
      }
      else { // Section
        sectionNdx = Section::extractSection(input, fileNo, line);
      }
    } 
    input.close();
    fileNo++;
  }

  this->sectionAddresses = sectionAddresses;

  symbolTableArray = SymbolTable::getFileNoToSymbolTable();
  sectionsArray = Section::getFileNoToSectionsArray();
  fileNoToSectionIndexArray = Section::getFileNoToSectionIndexArray();
  relocationTableArray = RelocationTable::getFileNoToRelocationTablesArray();

  int check;
  combineSectionsWithSameNamesAndTheirRelocationTables();
  check = combineAllSymbolTables();
  if(check < 0) {
    return 1;
  }

  ofstream output(output_file);
  if(!output.is_open()) {
    cout << "Error: Could not open file " << output_file << endl;
    return 1;
  }
  
  if(relocatable) {
    //we are making same output as assembler
    for(auto& combinedSection : combinedSections) {
      combinedSection.second->setRelocationTable(combinedRelocationTables[combinedSection.first]);
      CodeComponent::addSection(combinedSection.second);
    }

    CodeComponent::printToOutput(output);
  }
  else { 
    //we are making an executable file
    completeRelocations();
    printHexOutput(output);
  }

  return 0;
}

void Linker::combineSectionsWithSameNamesAndTheirRelocationTables() {

  // combine sections with the same names and their relocation tables
  combineSectionsAndRelTabs();

  // calculate and set final addresses for sections if file is not relocatable
  if(!relocatable) calculateAndSetFinalAddressesForSections();

  // add sections to the symbol table
  addSectionsToSymbolTable();
}

int Linker::combineAllSymbolTables() {
  //static table will be used as the main table
  SymbolTable *bigSymbolTable = SymbolTable::getInstance();

  for(size_t currentFileNo = 0; currentFileNo < symbolTableArray.size(); currentFileNo++) {
    SymbolTable *smallSymbolTable = symbolTableArray[currentFileNo];
    for(auto& smallSymbolTableEntry : smallSymbolTable->getTable()) {
      
      if(smallSymbolTableEntry->isSection) {
        continue;
      }

      int oldSymbolIndex = smallSymbolTableEntry->index;
      int oldSectionNdx = smallSymbolTableEntry->sectionNdx;
      int newSectionNdx = fileToSectionToCombinedSection[currentFileNo][oldSectionNdx];

      SymbolTableEntry *bigSymbolTableEntry = bigSymbolTable->getEntry(smallSymbolTableEntry->name);
      if(bigSymbolTableEntry != nullptr) {
        if(bigSymbolTableEntry->global && smallSymbolTableEntry->global) {
          cout << "Error: Multiple global symbols with the same name. " << smallSymbolTableEntry->name << endl;
          return -1;
        }
        if(bigSymbolTableEntry->global && smallSymbolTableEntry->isExtern && hex_output) {
          updateRelocationTablesSymbolIndices(  
            oldSymbolIndex, bigSymbolTableEntry->index,
            currentFileNo, oldSectionNdx
          );
          continue;
        }
        if(smallSymbolTableEntry->global || smallSymbolTableEntry->isExtern) {
          bigSymbolTable->changeSymbolNameToMakeItUnique(bigSymbolTableEntry);
          continue;
        }
        smallSymbolTable->changeSymbolNameToMakeItUnique(smallSymbolTableEntry);
      }

      SymbolTableEntry *newEntry = new SymbolTableEntry();
      if(smallSymbolTableEntry->isExtern) {
        newEntry->index = smallSymbolTableEntry->index;
        newEntry->sectionNdx = 0;
        newEntry->isDefined = true;
        newEntry->isExtern = true;
        newEntry->name = smallSymbolTableEntry->name;

        UnresolvedExtern *unresolvedExtern = new UnresolvedExtern();
        unresolvedExtern->symbolTableEntry = newEntry;
        unresolvedExtern->fileNo = currentFileNo;
        addUnresolvedExtern(unresolvedExtern);
      }
      else {
        int calculatedValue;
        if(smallSymbolTableEntry->absolute) {
          calculatedValue = smallSymbolTableEntry->value;
        }
        else {
          // calculatedValue = value + address of the section in the combined section + address of the combined section
          calculatedValue = smallSymbolTableEntry->value + fileToSectionToItsAddressInCombinedSection[currentFileNo][smallSymbolTableEntry->sectionNdx] 
          + combinedSections[smallSymbolTable->getEntry(smallSymbolTableEntry->sectionNdx)->name]->getSectionFinalAddress(); 
        }

        updateRelocationTablesSymbolIndices( //ERROR HERE
          oldSymbolIndex, bigSymbolTable->getSize(),
          currentFileNo, oldSectionNdx
        );

        //index is given to an entry automatically in addEntry function
        newEntry->sectionNdx = newSectionNdx;
        newEntry->name = smallSymbolTableEntry->name;
        newEntry->value = calculatedValue;
        newEntry->global = smallSymbolTableEntry->global;
        newEntry->isDefined = true;
        bigSymbolTable->addEntry(newEntry);

        if(newEntry->global && checkIfSymbolIsInUnresolvedExterns(newEntry->name)) {
          for(auto& unresolvedExternEntry : unresolvedExterns[newEntry->name]) {
            updateRelocationTablesSymbolIndices(  
              unresolvedExternEntry->symbolTableEntry->index, bigSymbolTable->getSize(),
              unresolvedExternEntry->fileNo, unresolvedExternEntry->symbolTableEntry->sectionNdx
            );
          }
          unresolvedExterns.erase(newEntry->name);
        }
      }
    }
  }

  int check = finalizeUnresolvedExterns();
  if(check < 0) {
    return 1;
  }

  return 0;
}

void Linker::combineSectionsAndRelTabs() {

  unordered_map<int, Section *> sections;
  vector<int> currentFileSectionIndices;
  unordered_map<int, RelocationTable *> relocationTables;

  for(size_t currentFileNo = 0; currentFileNo < sectionsArray.size(); currentFileNo++) {
    sections = sectionsArray[currentFileNo];
    currentFileSectionIndices = fileNoToSectionIndexArray[currentFileNo];
    relocationTables = relocationTableArray[currentFileNo];
    for(size_t i = 0; i < currentFileSectionIndices.size(); i++) {
      int csi = currentFileSectionIndices[i]; //csi == currentSectionIndex
      if(combinedSections.find(sections[csi]->getName()) == combinedSections.end()) {

        combinedSections[sections[csi]->getName()] = sections[csi];
        combinedSectionsInOrder.push_back(sections[csi]);
        sectionIndices[sections[csi]->getName()] = combinedSections.size() - 1;
        fileToSectionToCombinedSection[currentFileNo][sections[csi]->getNdx()] = sectionIndices[sections[csi]->getName()];

        fileToSectionToItsAddressInCombinedSection[currentFileNo][sections[csi]->getNdx()] = 0;

        relocationTables[csi]->updateSectionIndex(sectionIndices[sections[csi]->getName()]);
        combinedRelocationTables[sections[csi]->getName()] = relocationTables[csi];
        combinedRelocationTablesInOrder.push_back(relocationTables[csi]);
        fileToSectionToCombinedRelocationTableStart[currentFileNo][sections[csi]->getNdx()] = 0;
        fileToSectionToCombinedRelocationTableEnd[currentFileNo][sections[csi]->getNdx()] = relocationTables[csi]->getTableSize();
      }
      else {
        Section *combinedSection = combinedSections[sections[csi]->getName()];
        combinedSection->concatenateAnotherSectionsContent(sections[csi]->getContent());
        fileToSectionToCombinedSection[currentFileNo][sections[csi]->getNdx()] = sectionIndices[sections[csi]->getName()];

        fileToSectionToItsAddressInCombinedSection[currentFileNo][sections[csi]->getNdx()] = combinedSection->getSize();
        relocationTables[csi]->increaseEntryOffsets(combinedSection->getSize());

        combinedSection->increaseSize(sections[csi]->getSize());
        fileToSectionToCombinedRelocationTableStart[currentFileNo][sections[csi]->getNdx()] = combinedRelocationTables[sections[csi]->getName()]->getTableSize();
        combinedRelocationTables[sections[csi]->getName()]->concatenateAnotherRelocationTable(relocationTables[csi]);
        fileToSectionToCombinedRelocationTableEnd[currentFileNo][sections[csi]->getNdx()] = combinedRelocationTables[sections[csi]->getName()]->getTableSize();
      }
    }
  }
}

void Linker::checkForSectionAddressInterference(uint32_t start, uint32_t end) {
  for(size_t i = 0; i < predeterminedSectionStarts.size(); i++) {
    if((start >= predeterminedSectionStarts[i] && start <= predeterminedSectionEnds[i]) || 
      (end >= predeterminedSectionStarts[i] && end <= predeterminedSectionEnds[i])) {
      cout << "Linking fatal Error: Section address interference." << endl;
      cout << predeterminedSectionStarts[i] << " " << predeterminedSectionEnds[i] << " " << start << " " << end << endl;
      exit(1);
    }
  }
}

void Linker::calculateAndSetFinalAddressesForSections() {

  uint32_t maxPredeterminedAddress = 0x40000000;
  uint32_t placeForNextNonPredeterminedSection = 0x40000000;

  for(auto& combinedSection : combinedSectionsInOrder) {
    string sectionName = combinedSection->getName();

    if(sectionAddresses.find(sectionName) != sectionAddresses.end()) {

      checkForSectionAddressInterference(sectionAddresses[sectionName], sectionAddresses[sectionName] + combinedSections[sectionName]->getSize());

      combinedSections[sectionName]->setSectionFinalAddress(sectionAddresses[sectionName]);
      combinedRelocationTables[sectionName]->increaseEntryOffsets(sectionAddresses[sectionName]);

      predeterminedSectionStarts.push_back(sectionAddresses[sectionName]);
      predeterminedSectionEnds.push_back(sectionAddresses[sectionName] + combinedSections[sectionName]->getSize());

      // determine the starting place of sections without predetermined addresses
      if(sectionAddresses[sectionName] > maxPredeterminedAddress) {
        maxPredeterminedAddress = sectionAddresses[sectionName];
        placeForNextNonPredeterminedSection = maxPredeterminedAddress + combinedSections[sectionName]->getSize();
      }
    }
  }
  
  for(auto& combinedSection : combinedSectionsInOrder) {
    string sectionName = combinedSection->getName();
    
    if(sectionAddresses.find(sectionName) == sectionAddresses.end()) {
      combinedSection->setSectionFinalAddress(placeForNextNonPredeterminedSection);
      combinedRelocationTables[sectionName]->increaseEntryOffsets(placeForNextNonPredeterminedSection);
      placeForNextNonPredeterminedSection += combinedSection->getSize();
    }
  }
}

void Linker::addSectionsToSymbolTable() {
  SymbolTable *symbolTable = SymbolTable::getInstance();
  for(auto& combinedSection : combinedSectionsInOrder) {
    SymbolTableEntry *newEntry = new SymbolTableEntry();
    newEntry->name = combinedSection->getName();
    newEntry->value = combinedSection->getSectionFinalAddress();
    newEntry->isDefined = true;
    newEntry->isSection = true;
    symbolTable->addEntry(newEntry);
  }
}

int Linker::finalizeUnresolvedExterns() {
  SymbolTable *bigSymbolTable = SymbolTable::getInstance();

  if(!unresolvedExterns.empty() && !relocatable) {
    for(auto& unresolvedExtern : unresolvedExterns) {
      cout << "Linking fatal Error: Unresolved external symbol " << unresolvedExtern.first << endl;
    }
    return -1;
  }
  else {
    for(auto& unresolvedExtern : unresolvedExterns) {
      for(auto& unresolvedExternEntry : unresolvedExtern.second) {
        updateRelocationTablesSymbolIndices(
          unresolvedExternEntry->symbolTableEntry->index, bigSymbolTable->getSize(),
          unresolvedExternEntry->fileNo, unresolvedExternEntry->symbolTableEntry->sectionNdx
        );
      }

      SymbolTableEntry *newEntry = new SymbolTableEntry();
      newEntry->name = unresolvedExtern.first;
      newEntry->isExtern = true;
      newEntry->isDefined = true;
      newEntry->sectionNdx = 0;
      bigSymbolTable->addEntry(newEntry);
    }
  }

  return 0;
}

bool Linker::checkIfSymbolIsInUnresolvedExterns(string symbol) {
  
  if(unresolvedExterns.find(symbol) != unresolvedExterns.end()) {
    return true;
  }
  return false;
}

void Linker::addUnresolvedExtern(UnresolvedExtern *unresolvedExtern) {

  if(unresolvedExterns.find(unresolvedExtern->symbolTableEntry->name) == unresolvedExterns.end()) {
    unresolvedExterns[unresolvedExtern->symbolTableEntry->name] = vector<UnresolvedExtern *>();
  }
  unresolvedExterns[unresolvedExtern->symbolTableEntry->name].push_back(unresolvedExtern);
}

void Linker::updateRelocationTablesSymbolIndices(int oldSymbolIndex, int newSymbolIndex, int fileNo, int sectionIndex) {
  
  int start = fileToSectionToCombinedRelocationTableStart[fileNo][sectionIndex];
  int end = fileToSectionToCombinedRelocationTableEnd[fileNo][sectionIndex];

  for(int i = start; i < end; i++) {
    if(combinedRelocationTables[sectionsArray[fileNo][sectionIndex]->getName()]->getTable()[i]->symbolIndex == oldSymbolIndex) {
      combinedRelocationTables[sectionsArray[fileNo][sectionIndex]->getName()]->getTable()[i]->symbolIndex = newSymbolIndex;
    }
  }
}

void Linker::completeRelocations() {
  SymbolTable *symbolTable = SymbolTable::getInstance();
  for(size_t i = 0; i < combinedRelocationTablesInOrder.size(); i++) {
    Section *section = combinedSectionsInOrder[i];
    for(auto& relocationTableEntry : combinedRelocationTablesInOrder[i]->getTable()) {
      section->addQuadbyteToSectionContentWithOffset(symbolTable->getEntry(relocationTableEntry->symbolIndex)->value, (uint32_t)relocationTableEntry->offset - section->getSectionFinalAddress());
    }
  }
}

void Linker::printHexOutput(ofstream &output) {
  for(auto& combinedSection : combinedSectionsInOrder) {
    uint32_t sectionAddress = combinedSection->getSectionFinalAddress();
    vector<uint8_t> content = combinedSection->getContent();

    for(size_t i = 0; i < content.size(); i++) {
      
      if(i % 8 == 0) {
        output << hex << setw(8) << setfill('0') << sectionAddress + i << ": ";
      }

      output << hex << setw(2) << setfill('0') << static_cast<int>(content[i]) << " ";

      if((i + 1) % 8 == 0 || i == content.size() - 1) {
        output << endl;
      }
    }
  }
  output.close();
}