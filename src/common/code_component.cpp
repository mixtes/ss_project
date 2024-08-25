#include "../../inc/common/code_component.hpp"

int CodeComponent::nextID = 0;
bool CodeComponent::finished = false;
int CodeComponent::currentSectionNdx = 0;
SymbolTable *CodeComponent::symbolTable = SymbolTable::getInstance();
vector<Section *> CodeComponent::sectionsList = vector<Section *>();
unordered_map<int, Section *> CodeComponent::sectionNdxToSection = unordered_map<int, Section *>();

CodeComponent::CodeComponent(string value, Operand *operands)  {
  this->value = value;
  this->operands = operands;
  myID = nextID++;
}

void CodeComponent::operandsToArray()  {
  Operand *current = operands;
  while (current != nullptr) {
    operandsList.push_back(current);
    current = current->next;
  }
}

void CodeComponent::sewLiteralPoolsToSections() {
  int displacement;
  for(auto& section : sectionsList) {
    for(auto& literal : section->getLiteralPool()) {
      section->addQuadbyteToSectionContent(literal.first);
      for(auto& offset : literal.second) {
        displacement = section->getLocationCounter() - offset - 4;
        section->changeDisplacementInInstruction(displacement, offset);
      }
    }
  }
}

void CodeComponent::sewSymbolOffsetsToSections() {
  int displacement;
  for(auto& section : sectionsList) {
    for(auto& symbol : section->getSymbolOffsets()) {

      section->getRelocationTable()->addEntry(section->getNdx(), section->getLocationCounter(), REL_TYPE_ABS, symbol.first);
      section->addQuadbyteToSectionContent(symbol.first);

      for(auto& offset : symbol.second) {
        displacement = section->getLocationCounter() - offset - 4;
        section->changeDisplacementInInstruction(displacement, offset);
      }
    }
  }
}

void CodeComponent::printToOutput(ofstream &output) {
  symbolTable->printToOutput(output);

  for(auto& section : sectionsList) {
    output << section->getName() << " " << hex << section->getLocationCounter() << " " << section->getNdx() << endl;
    int enter = 0;
    int tab = 4;
    for(auto& content : section->getContent()) {
      output << hex << setw(2) << setfill('0') << static_cast<int>(content) << " ";
      tab++; enter++;
      if(tab == 8) {
        output << "\t";
        tab = 0;
      }
      if(enter == 8) {
        output << endl;
        enter = 0;
      }
    }
    output << endl;
    if(enter != 0) {
      output << endl;
    }
    section->getRelocationTable()->printToOutput(output);
  }

  output.close();
}

void CodeComponent::backpatch(string symbol)
{

  SymbolTableEntry *symbolTableEntry = symbolTable->getEntry(symbol);
  if(symbolTableEntry->forwardReference == nullptr) {
    return;
  }

  if(symbolTableEntry->absolute && symbolTableEntry->edgecase12bit) {
    if(!valueWithin12BitRange(symbolTableEntry->value)) {
      cout << "Assembler: Symbol " << symbol << " is absolute and referenced by symDispReg, but is not within 12 bit range." << endl;
      exit(1);
    }
  }
  if(!symbolTableEntry->absolute && symbolTableEntry->edgecase12bit) {
    cout << "Assembler: Symbol " << symbol << " is not absolute and referenced by symDispReg." << endl;
    exit(1);
  }

  int symbolIndex = symbolTableEntry->index;
  SymbolTableForwardReference *currentReference = symbolTableEntry->forwardReference;
  while(currentReference != nullptr) {
    int sectionNdx = currentReference->sectionNdx;
    Section *section = getSection(sectionNdx);

    if(!currentReference->word) section->addSymbolOffset(symbolIndex, currentReference->patchLocation);
    else { // when forward reference was formed by .word directive
      section->getRelocationTable()->addEntry(sectionNdx, currentReference->patchLocation, REL_TYPE_ABS, symbolIndex);
      section->addQuadbyteToSectionContentWithOffset(symbolTableEntry->value, currentReference->patchLocation);
    }
    
    currentReference = currentReference->nextReference;
  }

  symbolTable->clearForwardReferences(symbolTableEntry);
}
