#include "../../inc/common/label_component.hpp"

void Label::setSymbolTableEntryInformation(SymbolTableEntry *entry) {
  entry->isDefined = true;
  entry->sectionNdx = symbolTable->getEntry(getSectionsList().back()->getName())->index;
  entry->value = getSectionsList().back()->getLocationCounter();
}

int Label::analize() {

  if(symbolTable->getEntry(this->value) == nullptr) {
    SymbolTableEntry *newEntry = new SymbolTableEntry();
    newEntry->name = this->value;
    setSymbolTableEntryInformation(newEntry);
    symbolTable->addEntry(newEntry);
  } 
  else if(symbolTable->getEntry(this->value)->isDefined) {
    cout << "Label " << this->value << " already defined." << endl;
    return -1;
  }
  else {
    setSymbolTableEntryInformation(symbolTable->getEntry(this->value));
    backpatch(this->value);
  }
  return 0;
}
