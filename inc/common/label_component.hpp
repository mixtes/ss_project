#ifndef _LABEL_COMPONENT_HPP
#define _LABEL_COMPONENT_HPP

#include "code_component.hpp"

class Label : public CodeComponent {
  
  public:

  Label(string label, Operand *operands) : CodeComponent(label, operands) {
    this->type = "label";
  }

  int analize();

  private:

  void setSymbolTableEntryInformation(SymbolTableEntry *entry);

};

#endif