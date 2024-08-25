#ifndef _SPASSEMBLER_HPP
#define _SPASSEMBLER_HPP

#include <iostream>
#include <string>
#include <vector>

#include "../common/instruction_component.hpp"
#include "../common/directive_component.hpp"
#include "../common/label_component.hpp"

using namespace std;

class SPAssembler {
  public:

  static SPAssembler* getInstance() {
    static SPAssembler instance;
    if(!initialized) {
      initialized = true; // before initializing to avoid infinite recursion(deadlock)
      initializeAssembler();   
    }
    return &instance;
  }

  int assemble(string input_file, string output_file);

  void addCodeComponent(string name, string type, Operand *operands);

  Operand* getHoardHead() {
    return hoardHead;
  }

  void hoardOperand(Operand *operand);

  void clearHoard() {
    hoardHead = nullptr;
    hoardTail = nullptr;
  }

  ~SPAssembler() {}

  private:

  static bool initialized;
  Operand *hoardHead = nullptr, *hoardTail = nullptr;

  static void initializeAssembler();
  SPAssembler() {}
  SPAssembler(SPAssembler const&) = delete;
  void operator=(SPAssembler const&) = delete;

  vector<CodeComponent *> codeComponents;

  static SymbolTable *symbolTable;

  int checkIfAllSymbolsAreDefined();
};

#endif