#ifndef _DIRECTIVE_COMPONENT_HPP
#define _DIRECTIVE_COMPONENT_HPP

#include "code_component.hpp"
#include <algorithm>

class Directive : public CodeComponent {

  public:

  Directive(string directive, Operand *operands) : CodeComponent(directive, operands) {
    this->type = "directive";
  }

  int analize();

  private:

  int globalHandler();

  int externHandler();

  int sectionHandler();

  int wordHandler();

  int skipHandler();

  int asciiHandler();

  int equHandler();

  int endHandler();

  void setFlagsAndSectionInExternHandler(SymbolTableEntry *entry);
  void setFlagsAndSectionInSectionHandler(SymbolTableEntry *entry);
};

#endif