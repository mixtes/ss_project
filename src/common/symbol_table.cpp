#include "../../inc/common/symbol_table.hpp"

vector<SymbolTable *> SymbolTable::linkerSymbolTables = vector<SymbolTable *>();
int SymbolTable::globalUniqueIndex = 0;

SymbolTableEntry* SymbolTable::getEntry(string name) {

  if(tableMap.find(name) != tableMap.end()) {
    return tableMap[name];
  }

  return nullptr;
}

SymbolTableEntry* SymbolTable::getEntry(int index) {
  if((size_t)index < table.size()) {
    return table[index];
  }

  return nullptr;
}

// bool word argument is an indicator that the forward reference is for a word directive
void SymbolTable::addForwardReference(string symbol, int patchLocation, int sectionNdx, bool word) {

  SymbolTableEntry *symbolTableEntry = this->getEntry(symbol);

  SymbolTableForwardReference *newReference = new SymbolTableForwardReference();
  newReference->sectionNdx = sectionNdx;
  newReference->patchLocation = patchLocation;
  newReference->word = word;
  newReference->nextReference = symbolTableEntry->forwardReference;

  symbolTableEntry->forwardReference = newReference;
}

void SymbolTable::clearForwardReferences(SymbolTableEntry *entry) {
  
  SymbolTableForwardReference *currentReference = entry->forwardReference;
  SymbolTableForwardReference *nextReference;
  while(currentReference != nullptr) {
    nextReference = currentReference->nextReference;
    delete currentReference;
    currentReference = nextReference;
  }
}

void SymbolTable::printToOutput(ofstream &output) {
  //"Index  Name  Value  Section  Global  Absolute  isDefined  isExtern  isSection"
  char header[4] = {'S', 'Y', 'M', 'T'};
  output << header << endl;
  for(auto& entry : table) {
    output << entry->index << "\t" << entry->name << "\t" << entry->value << "\t" 
    << entry->sectionNdx << "\t" << entry->global << "\t" << entry->absolute << "\t" 
    << entry->isDefined << "\t" << entry->isExtern << "\t" << entry->isSection << endl;
  }
  output << endl;
}

void SymbolTable::printCurrentTable() {
  for(auto& entry : table) {
    cout << entry->index << "\t" << entry->name << endl;
  }
}

void SymbolTable::extractSymbolTable(ifstream &input, int fileNo) {
  string line;
  SymbolTable *symbolTable = new SymbolTable();
  if(linkerSymbolTables.size() <= (size_t)fileNo) {
    linkerSymbolTables.resize(fileNo + 1);
  }
  linkerSymbolTables[fileNo] = symbolTable;

  istringstream iss;

  int skipIndex;

  while(getline(input, line)) {
    if(line.empty()) {
      break;
    }
    iss.clear();
    iss.str(line);

    SymbolTableEntry *newEntry = new SymbolTableEntry();

    iss >> skipIndex;
    iss >> newEntry->name;
    iss >> newEntry->value;
    iss >> newEntry->sectionNdx;
    iss >> newEntry->global;
    iss >> newEntry->absolute;
    iss >> newEntry->isDefined;
    iss >> newEntry->isExtern;
    iss >> newEntry->isSection;

    addEntryToLinkerSymbolTable(newEntry, fileNo);
  }
}

void SymbolTable::changeSymbolNameToMakeItUnique(SymbolTableEntry *entry) {
  entry->name = entry->name + "_" + to_string(globalUniqueIndex++);
}

