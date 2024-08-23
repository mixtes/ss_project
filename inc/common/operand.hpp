#ifndef _OPERAND_H
#define _OPERAND_H

#include <string>
#include <iostream>
using namespace std;

struct Operand {
  string type = "";
  string value = "";
  Operand *next = nullptr;
  string addittionalData = "";
};

inline Operand* formOperand(string type, string value) {
  Operand *newOperand = new Operand();
  newOperand->type = type;
  newOperand->value = value;
  newOperand->next = nullptr;

  return newOperand;
}

inline Operand* formOperandWithAdditionalData(string type, string value, string additionalData) {
  Operand *newOperand = new Operand();
  newOperand->type = type;
  newOperand->value = value;
  newOperand->next = nullptr;
  newOperand->addittionalData = additionalData;

  return newOperand;
}

#endif
