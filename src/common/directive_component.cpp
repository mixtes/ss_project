#include "../../inc/common/directive_component.hpp"

using namespace std;

int Directive::analize() {

  string directive = this->value;
  operandsToArray();
  reverse(operandsList.begin(), operandsList.end()); // speciffically for directive operands

  int check;
  if(directive == ".global") {
    check = globalHandler();
  }
  else if(directive == ".extern") {
    check = externHandler();
  }
  else if(directive == ".section") {
    check = sectionHandler();
  }
  else if(directive == ".word") {
    check = wordHandler();
  }
  else if(directive == ".skip") {
    check = skipHandler();
  }
  else if(directive == ".ascii") {
    check = asciiHandler();
  }
  else if(directive == ".equ") {
    check = equHandler();
  }
  else if(directive == ".end") {
    check = endHandler();
  }
  
  else {
    cout << "Directive not recognized" << endl;
    return -1;
  }

  if(check != 0) {
    cout << "Directive " << directive << " failed." << endl;
    return -1;
  }
  return 0;
}

int Directive::globalHandler() {

  for(auto& symbol : operandsList) {
    if(symbolTable->getEntry(symbol->value) == nullptr) {
      SymbolTableEntry *newEntry = new SymbolTableEntry();
      newEntry->name = symbol->value;
      newEntry->global = true;
      newEntry->isDefined = false;
      symbolTable->addEntry(newEntry);
    }
    else if(symbolTable->getEntry(symbol->value)->isSection) {
      cout << "Directive " << this->value << " error: section can't be global." << endl;
      return -1;
    }
    else {
      symbolTable->getEntry(symbol->value)->global = true;
    }
  }
  return 0;
}

void Directive::setFlagsAndSectionInExternHandler(SymbolTableEntry *entry) {
  entry->isExtern = true;
  entry->isDefined = true;
  entry->sectionNdx = 0;
}

int Directive::externHandler() {

  for(auto& symbol : operandsList) {
    if(symbolTable->getEntry(symbol->value) == nullptr) {
      SymbolTableEntry *newEntry = new SymbolTableEntry();
      newEntry->name = symbol->value;
      setFlagsAndSectionInExternHandler(newEntry);
      symbolTable->addEntry(newEntry);
    }
    else if(!symbolTable->getEntry(symbol->value)->isDefined) {
      setFlagsAndSectionInExternHandler(symbolTable->getEntry(symbol->value));
      backpatch(this->value);
    }
    else {
      cout << "Directive " << this->value << " error: symbol already defined." << endl;
      return -1;
    }
  }
  return 0;
}

void Directive::setFlagsAndSectionInSectionHandler(SymbolTableEntry *entry) {

  string sectionName = this->operandsList[0]->value;
  entry->isSection = true;
  entry->isDefined = true;
  entry->sectionNdx = symbolTable->getSize();
  entry->value = 0;
  addSection(new Section(sectionName, symbolTable->getSize()));
}

int Directive::sectionHandler() {

  string sectionName = this->operandsList[0]->value;
  if(symbolTable->getEntry(sectionName) == nullptr) {
    SymbolTableEntry *newEntry = new SymbolTableEntry();
    newEntry->name = sectionName;
    setFlagsAndSectionInSectionHandler(newEntry);
    symbolTable->addEntry(newEntry);
  }
  else if(symbolTable->getEntry(this->value)->isDefined) {
    cout << "Directive " << this->value << " error: section already defined." << endl;
    return -1;
  }
  else if(symbolTable->getEntry(this->value)->global) {
    cout << "Directive " << this->value << " error: section can't be global." << endl;
    return -1;
  }
  else {
    setFlagsAndSectionInSectionHandler(symbolTable->getEntry(this->value));
    backpatch(this->value);
  }
  return 0;
}

int Directive::wordHandler() {
  for(auto& operand : operandsList) {
    if(operand->type == "literal") {
      getCurrentSection()->addQuadbyteToSectionContent(stoi(operand->value));
    }
    else if(operand->type == "symbol") {
      string symbol = operand->value;
      if(symbolTable->getEntry(symbol) == nullptr) {
        SymbolTableEntry *newEntry = new SymbolTableEntry();
        newEntry->name = symbol;
        newEntry->sectionNdx = 0;
        newEntry->isDefined = false;
        symbolTable->addEntry(newEntry);
      }
      if(!symbolTable->getEntry(symbol)->isDefined) {
        symbolTable->addForwardReference(symbol, getCurrentSection()->getLocationCounter(), getCurrentSectionNdx(), true);
        getCurrentSection()->addQuadbyteToSectionContent(0); // placeholder
      }
      else {
        getCurrentSection()->getRelocationTable()->addEntry(getCurrentSectionNdx(), getCurrentSection()->getLocationCounter(), REL_TYPE_ABS, symbolTable->getEntry(symbol)->index);
        getCurrentSection()->addQuadbyteToSectionContent(symbolTable->getEntry(symbol)->value);
      }
    }
  }
  return 0;
}

int Directive::skipHandler() {
  int numberOfBytesToSkip = stoi(operandsList[0]->value);
  for(int i = 0; i < numberOfBytesToSkip; i++) {
    getCurrentSection()->addByteToSectionContent(0);
  }
  return 0;
}

int Directive::asciiHandler() {
  string asciiStringOperand = operandsList[0]->value;
  for(auto& character : asciiStringOperand) {
    if(character != '"') getCurrentSection()->addByteToSectionContent(character);
  }
  getCurrentSection()->addByteToSectionContent(0);
  return 0;
}

int Directive::equHandler() {
  cout << "Equ handler" << endl;
  return 0;
}

int Directive::endHandler() {
  setFinished(true);
  return 0;
}