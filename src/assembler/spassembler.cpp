#include "../../inc/assembler/spassembler.hpp"
#include <iostream>
using namespace std;

SymbolTable *SPAssembler::symbolTable = SymbolTable::getInstance();
bool SPAssembler::initialized = false;

void SPAssembler::initializeAssembler() {

  SPAssembler *instance = SPAssembler::getInstance();

  SymbolTableEntry *entry = new SymbolTableEntry();
  entry->name = "UND";
  entry->sectionNdx = 0;
  entry->value = 0;
  entry->isDefined = true;
  entry->isSection = true;
  instance->symbolTable->addEntry(entry);

  entry = new SymbolTableEntry();
  entry->name = "ABS";
  entry->sectionNdx = 1;
  entry->value = 0;
  entry->isDefined = true;
  entry->isSection = true;
  instance->symbolTable->addEntry(entry);

  CodeComponent::addSection(new Section("UND", 0));
  CodeComponent::addSection(new Section("ABS", 1));
  CodeComponent::setCurrentSectionNdx(0);
}

void SPAssembler::addCodeComponent(string name, string type, Operand *operands) {

  CodeComponent *newComponent;

  if(type == "instruction") {
    newComponent = new Instruction(name, operands);
  } 
  else if(type == "directive") {
    newComponent = new Directive(name, operands);
  }
  else if(type == "label") {
    newComponent = new Label(name, operands);
  }

  codeComponents.push_back(newComponent);
  this->clearHoard();
}

int SPAssembler::assemble(string input_file, string output_file) {

  if(codeComponents.size() == 0) {
    cout << "Assembler: No code components to assemble." << endl;
    return 1;
  }
  for(auto& component : codeComponents) {
    component->analize();
    if(CodeComponent::checkFinished()) break;
  }
  if(!CodeComponent::checkFinished()) {
    cout << "Assembler: no .end directive detected" << endl;
    return 1;
  }

  int check = checkIfAllSymbolsAreDefined();
  if(check != 0) return check;

  CodeComponent::sewLiteralPoolsToSections();
  CodeComponent::sewSymbolOffsetsToSections();

  ofstream outFile(output_file);
  CodeComponent::printToOutput(outFile);

  cout << "Assembler: " << input_file << " assembled successfull." << endl;

  return 0;
}

void SPAssembler::hoardOperand(Operand *operand) {

  if(hoardHead == nullptr) {
    hoardHead = operand;
    hoardTail = operand;
  } else {
    hoardTail->next = operand;
    hoardTail = operand;
  }
}

int SPAssembler::checkIfAllSymbolsAreDefined() {

  for(auto& symbolTableEntry : symbolTable->getTable()) {
    if(!symbolTableEntry->isDefined) {
      cout << "Error: Symbol " << symbolTableEntry->name << " is not defined." << endl;
      return -1;
    }
  }

  return 0;
}