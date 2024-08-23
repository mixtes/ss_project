#ifndef _CODE_COMPONENT_HPP
#define _CODE_COMPONENT_HPP

#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>

#include "symbol_table.hpp"
#include "operand.hpp"

using namespace std;

class CodeComponent {

  public:
  
  CodeComponent(string value, Operand *operands);

  virtual int analize() = 0;

  string getValue() {
    return value;
  }

  string getType() {
    return type;
  }

  void operandsToArray();

  static void setCurrentSectionNdx(int sectionNdx) {
    currentSectionNdx = sectionNdx;
  }

  static int getCurrentSectionNdx() {
    return currentSectionNdx;
  }

  static Section *getCurrentSection() {
    return sectionNdxToSection[currentSectionNdx];
  }

  static vector<Section *> getSectionsList() {
    return sectionsList;
  }

  static void addSection(Section *section) {
    setCurrentSectionNdx(section->getNdx());
    sectionsList.push_back(section);
    sectionNdxToSection[section->getNdx()] = section;
  }

  static Section *getSection(int sectionNdx) {
    return sectionNdxToSection[sectionNdx];
  }

  static bool checkFinished() {
    return finished;
  }

  static void setFinished(bool value) {
    finished = value;
  }

  static void sewLiteralPoolsToSections();

  static void sewSymbolOffsetsToSections();

  static void printToOutput(ofstream& output);

  void backpatch(string symbol);

  virtual ~CodeComponent() {}

  protected:

    inline bool valueWithin12BitRange(int value) {
      return value >= -2048 && value <= 2047;
    }

    static int nextID;

    string type;
    string value;
    Operand *operands;
    vector<Operand*> operandsList;

    static SymbolTable *symbolTable;

  private: 

    static vector<Section *> sectionsList;
    static unordered_map<int, Section *> sectionNdxToSection;
    static int currentSectionNdx;
    static bool finished;
    int myID;  
};

#endif