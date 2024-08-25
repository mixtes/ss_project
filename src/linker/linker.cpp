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

  int currentFileNo = 0;
  //static table will be used as the main table
  SymbolTable *bigSymbolTable = SymbolTable::getInstance();
  int newSymbolIndex = bigSymbolTable->getTable().size(); // it won't be 0 because of the sections

  while(symbolTableMap.find(currentFileNo) != symbolTableMap.end()) {
    SymbolTable *smallSymbolTable = symbolTableMap[currentFileNo];
    for(auto& symbolTableEntry : smallSymbolTable->getTable()) {
      
      if(symbolTableEntry->isSection) {
        continue;
      }

      int oldSymbolIndex = symbolTableEntry->index;
      int oldSectionNdx = symbolTableEntry->sectionNdx;
      int newSectionNdx = fileToSectionToCombinedSection[currentFileNo][oldSectionNdx];

      if(bigSymbolTable->getEntry(symbolTableEntry->name) != nullptr) {
        if(bigSymbolTable->getEntry(symbolTableEntry->name)->global && symbolTableEntry->global) {
          cout << "Error: Multiple global symbols with the same name. " << symbolTableEntry->name << endl;
          return -1;
        }
        if(bigSymbolTable->getEntry(symbolTableEntry->name)->global && symbolTableEntry->isExtern && !relocatable) {
          updateRelocationTablesSymbolIndices(  
            oldSymbolIndex, bigSymbolTable->getEntry(symbolTableEntry->name)->index,
            currentFileNo, newSectionNdx
          );
          continue;
        }
        smallSymbolTable->changeSymbolNameToMakeItUnique(symbolTableEntry);
      }

      SymbolTableEntry *newEntry = new SymbolTableEntry();
      if(symbolTableEntry->isExtern) {
        newEntry->index = symbolTableEntry->index;
        newEntry->sectionNdx = symbolTableEntry->sectionNdx;
        newEntry->isExtern = true;
        newEntry->name = symbolTableEntry->name;

        UnresolvedExtern *unresolvedExtern = new UnresolvedExtern();
        unresolvedExtern->symbolTableEntry = newEntry;
        unresolvedExtern->fileNo = currentFileNo;
        addUnresolvedExtern(unresolvedExtern);
      }
      else {
        int calculatedValue;
        if(symbolTableEntry->absolute) {
          calculatedValue = symbolTableEntry->value;
        }
        else {
          // value = value + address of the section in the combined section + address of the combined section
          calculatedValue = symbolTableEntry->value + fileToSectionToItsAddressInCombinedSection[currentFileNo][symbolTableEntry->sectionNdx] 
          + combinedSections[smallSymbolTable->getEntry(symbolTableEntry->sectionNdx)->name]->getSectionFinalAddress(); //ERROR HERE
        }

        updateRelocationTablesSymbolIndices(  
          oldSymbolIndex, newSymbolIndex,
          currentFileNo, newSectionNdx
        );

        //index is given to an entry automatically in addEntry function
        newEntry->sectionNdx = newSectionNdx;
        newEntry->name = symbolTableEntry->name;
        newEntry->value = calculatedValue;
        newEntry->global = symbolTableEntry->global;
        newEntry->isDefined = true;
        bigSymbolTable->addEntry(newEntry);

        if(newEntry->global && checkIfSymbolIsInUnresolvedExterns(newEntry->name)) {
          for(auto& unresolvedExternEntry : unresolvedExterns[newEntry->name]) {
            updateRelocationTablesSymbolIndices(  
              unresolvedExternEntry->symbolTableEntry->index, newSymbolIndex,
              unresolvedExternEntry->fileNo, fileToSectionToCombinedSection[unresolvedExternEntry->fileNo][unresolvedExternEntry->symbolTableEntry->sectionNdx]
            );
          }
          unresolvedExterns.erase(newEntry->name);
        }

        newSymbolIndex++;
      }
    }
    currentFileNo++;
  }

  int check = finalizeUnresolvedExterns();
  if(check < 0) {
    return 1;
  }

  return 0;
}

void Linker::combineSectionsAndRelTabs() {
  int nextFileNo = 0;
  vector<Section *> sections;
  vector<RelocationTable *> relocationTables;
  while(sectionsMap.find(nextFileNo) != sectionsMap.end()) {
    sections = sectionsMap[nextFileNo];
    relocationTables = relocationTableMap[nextFileNo];
    for(size_t i = 0; i < sections.size(); i++) {
      if(combinedSections.find(sections[i]->getName()) == combinedSections.end()) {

        combinedSections[sections[i]->getName()] = sections[i];
        sectionIndices[sections[i]->getName()] = combinedSections.size() - 1;
        fileToSectionToCombinedSection[nextFileNo][sections[i]->getNdx()] = sectionIndices[sections[i]->getName()];

        fileToSectionToItsAddressInCombinedSection[nextFileNo][sections[i]->getNdx()] = 0;

        relocationTables[i]->updateSectionIndex(sectionIndices[sections[i]->getName()]);
        combinedRelocationTables[sections[i]->getName()] = relocationTables[i];
        fileToSectionToCombinedRelocationTableStart[nextFileNo][sections[i]->getNdx()] = 0;
        fileToSectionToCombinedRelocationTableEnd[nextFileNo][sections[i]->getNdx()] = relocationTables[i]->getTableSize();
      }
      else {
        Section *combinedSection = combinedSections[sections[i]->getName()];
        combinedSection->concatenateAnotherSectionsContent(sections[i]->getContent());
        fileToSectionToCombinedSection[nextFileNo][sections[i]->getNdx()] = sectionIndices[sections[i]->getName()];

        fileToSectionToItsAddressInCombinedSection[nextFileNo][sections[i]->getNdx()] = combinedSection->getSize();
        relocationTables[i]->increaseEntryOffsets(combinedSection->getSize());

        combinedSection->increaseSize(sections[i]->getSize());
        fileToSectionToCombinedRelocationTableStart[nextFileNo][sections[i]->getNdx()] = combinedRelocationTables[sections[i]->getName()]->getTableSize();
        combinedRelocationTables[sections[i]->getName()]->concatenateAnotherRelocationTable(relocationTables[i]);
        fileToSectionToCombinedRelocationTableEnd[nextFileNo][sections[i]->getNdx()] = combinedRelocationTables[sections[i]->getName()]->getTableSize();
      }
    }
    nextFileNo++;
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
  uint32_t placeForNextNonPredeterminedSection = 0;

  for(auto& combinedSection : combinedSections) {
    string sectionName = combinedSection.first;

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
  
  for(auto& combinedSection : combinedSections) {
    string sectionName = combinedSection.first;
    
    if(sectionAddresses.find(sectionName) == sectionAddresses.end()) {
      combinedSection.second->setSectionFinalAddress(placeForNextNonPredeterminedSection);
      combinedRelocationTables[sectionName]->increaseEntryOffsets(placeForNextNonPredeterminedSection);
      placeForNextNonPredeterminedSection += combinedSection.second->getSize();
    }
  }
}

void Linker::addSectionsToSymbolTable() {
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
          unresolvedExternEntry->fileNo, fileToSectionToCombinedSection[unresolvedExternEntry->fileNo][unresolvedExternEntry->symbolTableEntry->sectionNdx]
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
    if(combinedRelocationTables[combinedSections[sectionsMap[fileNo][sectionIndex]->getName()]->getName()]->getTable()[i]->symbolIndex == oldSymbolIndex) {
      combinedRelocationTables[combinedSections[sectionsMap[fileNo][sectionIndex]->getName()]->getName()]->getTable()[i]->symbolIndex = newSymbolIndex;
    }
  }
}

void Linker::completeRelocations() {
  SymbolTable *symbolTable = SymbolTable::getInstance();
  for(auto& combinedRelocationTable : combinedRelocationTables) {
    Section *section = combinedSections[combinedRelocationTable.first];
    for(auto& relocationTableEntry : combinedRelocationTable.second->getTable()) {
      section->addQuadbyteToSectionContentWithOffset(symbolTable->getEntry(relocationTableEntry->symbolIndex)->value, relocationTableEntry->offset);
    }
  }
}

void Linker::printHexOutput(ofstream &output) {
  for(auto& combinedSection : combinedSections) {
    uint32_t sectionAddress = combinedSection.second->getSectionFinalAddress();
    vector<uint8_t> content = combinedSection.second->getContent();

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