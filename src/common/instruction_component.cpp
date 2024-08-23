#include "../../inc/common/instruction_component.hpp"

void Instruction::formInstructionAndAddToSection(Opcode opcode, uint8_t mod, uint8_t operand1, uint8_t operand2, uint8_t operand3, int16_t displacement) {
  
  uint32_t instruction = 0;
  instruction |= opcode;
  instruction <<= 4;
  instruction |= mod;
  instruction <<= 4;
  instruction |= operand1;
  instruction <<= 4;
  instruction |= operand2;
  instruction <<= 4;
  instruction |= operand3;
  instruction <<= 12;
  instruction |= displacement;

  getCurrentSection()->addInstructionToSectionContent(instruction);
}

int Instruction::analize() {

  string instruction = this->value;
  operandsToArray();

  int check;
  if(instruction == "halt") {
    check = haltHandler();
  }
  else if(instruction == "int") {
    check = intHandler();
  }
  else if(instruction == "iret") {
    check = iretHandler();
  }
  else if(instruction == "call") {
    check = callHandler();
  }
  else if(instruction == "ret") {
    check = retHandler();
  }
  else if(instruction == "jmp") {
    check = jmpHandler();
  }
  else if(instruction == "beq") {
    check = beqHandler();
  }
  else if(instruction == "bne") {
    check = bneHandler();
  }
  else if(instruction == "bgt") {
    check = bgtHandler();
  }
  else if(instruction == "push") {
    check = pushHandler();
  }
  else if(instruction == "pop") {
    check = popHandler();
  }
  else if(instruction == "xchg") {
    check = xchgHandler();
  }
  else if(instruction == "add") {
    check = addHandler();
  }
  else if(instruction == "sub") {
    check = subHandler();
  }
  else if(instruction == "mul") {
    check = mulHandler();
  }
  else if(instruction == "div") {
    check = divHandler();
  }
  else if(instruction == "not") {
    check = notHandler();
  }
  else if(instruction == "and") {
    check = andHandler();
  }
  else if(instruction == "or") {
    check = orHandler();
  }
  else if(instruction == "xor") {
    check = xorHandler();
  }
  else if(instruction == "shl") {
    check = shlHandler();
  }
  else if(instruction == "shr") {
    check = shrHandler();
  }
  else if(instruction == "ld") {
    check = ldHandler();
  }
  else if(instruction == "st") {
    check = stHandler();
  }
  else if(instruction == "csrrd") {
    check = csrrdHandler();
  }
  else if(instruction == "csrwr") {
    check = csrwrHandler();
  }

  else {
    cout << "Instruction not recognized" << endl;
    return -1;
  }

  if(check != 0) {
    cout << "Instruction " << instruction << " failed." << endl;
    return -1;
  }
  return 0;
}

void Instruction::formInstructionWithForwardReference(Opcode opcode, uint8_t mod, uint8_t operand1, uint8_t operand2, uint8_t operand3, string symbol) {

  this->symbolTable->addForwardReference(symbol, getCurrentSection()->getLocationCounter(), getCurrentSectionNdx(), false);
  formInstructionAndAddToSection(opcode, mod, operand1, operand2, operand3, 0);
}

int Instruction::loadOrJumpOrCallWithLiteral(Opcode opcode, uint8_t mod, uint8_t operand1, uint8_t operand2, uint8_t operand3, int literal) {

  if(valueWithin12BitRange(literal)) {
    if(opcode == LOAD) operand2 = 0;
    formInstructionAndAddToSection(opcode, mod, operand1, operand2, operand3, literal);
  }
  else {
    int memMod;
    opcode == CALL ? memMod = CALL_MEM_MOD : memMod = JMP_MEM_MOD;
    if(opcode == LOAD) mod = LOAD_REGIND_MOD;

    getCurrentSection()->addLiteralToPool(literal);
    formInstructionAndAddToSection(opcode, mod | memMod, operand1, operand2, operand3, 0);
  }
  return 0;
}

int Instruction::loadOrJumpOrCallWithSymbol(Opcode opcode, uint8_t mod, uint8_t operand1, uint8_t operand2, uint8_t operand3, string symbol) {

  SymbolTableEntry *entry = symbolTable->getEntry(symbol);
  if(entry == nullptr) {
    entry = new SymbolTableEntry();
    entry->name = symbol;
    entry->isDefined = false;
    symbolTable->addEntry(entry);
  }
  if(entry->isDefined && entry->sectionNdx == getCurrentSectionNdx()) {
    int literal = entry->value - getCurrentSection()->getLocationCounter();
    loadOrJumpOrCallWithLiteral(opcode, mod, operand1, operand2, operand3, literal);
  }
  else {
    if(entry->isDefined) {
      getCurrentSection()->addSymbolOffset(entry->index, getCurrentSection()->getLocationCounter());
      formInstructionAndAddToSection(opcode, mod, operand1, operand2, operand3, 0);
    }
    else {
      formInstructionWithForwardReference(opcode, mod, operand1, operand2, operand3, symbol);
    } 
  }

  return 0;
}

int Instruction::detectCsrId(string csr) {
  
  if(csr == "status") {
    return STATUS;
  }
  else if(csr == "handler") {
    return HANDLER;
  }
  else if(csr == "cause") {
    return CAUSE;
  }
  else {
    cout << "Instruction: " << this->value << " invalid csr." << endl;
    return -1;
  }
}

int Instruction::haltHandler() {

  formInstructionAndAddToSection(HALT, 0, 0, 0, 0, 0);

  return 0;
}

int Instruction::intHandler() {

  formInstructionAndAddToSection(INT, 0, 0, 0, 0, 0);

  return 0;
}

int Instruction::iretHandler() {

  formInstructionAndAddToSection(LOAD, LOAD_REGDIR_MOD, SP, SP, 0, 8);
  formInstructionAndAddToSection(LOAD, LOAD_CSR_IND_MOD, STATUS, SP, 0, -4);
  formInstructionAndAddToSection(LOAD, LOAD_REGIND_MOD, PC, SP, 0, -8);

  return 0;
}

int Instruction::callHandler() {

  if(operandsList[0]->type == "literal") {
    return loadOrJumpOrCallWithLiteral(CALL, CALL_NO_MOD, 0, 0, 0, stoi(operandsList[0]->value));
  }
  else if(operandsList[0]->type == "symbol") {
    return loadOrJumpOrCallWithSymbol(CALL, CALL_NO_MOD, 0, 0, 0, operandsList[0]->value);
  }
  else {
    cout << "Invalid operand type for call instruction" << endl;
    return -1;
  }
}

int Instruction::retHandler() {

  formInstructionAndAddToSection(LOAD, LOAD_POP_MOD, PC, SP, 0, 4);
  return 0;
}

int Instruction::jmpHandler() {

  if(operandsList[0]->type == "literal") {
    return loadOrJumpOrCallWithLiteral(JMP, JMP_NO_MOD, 0, 0, 0, stoi(operandsList[0]->value));
  }
  else if(operandsList[0]->type == "symbol") {
    return loadOrJumpOrCallWithSymbol(JMP, JMP_NO_MOD, 0, 0, 0, operandsList[0]->value);
  }
  else {
    cout << "Invalid operand type for jmp instruction" << endl;
    return -1;
  }
}

int Instruction::beqHandler() {

  uint8_t gprA = stoi(operandsList[0]->value);
  uint8_t gprB = stoi(operandsList[1]->value);

  if(operandsList[2]->type == "literal") {
    return loadOrJumpOrCallWithLiteral(JMP, JMP_BEQ_MOD, 0, gprA, gprB, stoi(operandsList[2]->value));
  }
  else if(operandsList[2]->type == "symbol") {
    return loadOrJumpOrCallWithSymbol(JMP, JMP_BEQ_MOD, 0, gprA, gprB, operandsList[2]->value);
  }
  else {
    cout << "Invalid operand type for beq instruction" << endl;
    return -1;
  }
}

int Instruction::bneHandler() {

  uint8_t gprA = stoi(operandsList[0]->value);
  uint8_t gprB = stoi(operandsList[1]->value);

  if(operandsList[2]->type == "literal") {
    return loadOrJumpOrCallWithLiteral(JMP, JMP_BNE_MOD, 0, gprA, gprB, stoi(operandsList[2]->value));
  }
  else if(operandsList[2]->type == "symbol") {
    return loadOrJumpOrCallWithSymbol(JMP, JMP_BNE_MOD, 0, gprA, gprB, operandsList[2]->value);
  }
  else {
    cout << "Invalid operand type for bne instruction" << endl;
    return -1;
  }
}

int Instruction::bgtHandler() {

  uint8_t gprA = stoi(operandsList[0]->value);
  uint8_t gprB = stoi(operandsList[1]->value);

  if(operandsList[2]->type == "literal") {
    return loadOrJumpOrCallWithLiteral(JMP, JMP_BGT_MOD, 0, gprA, gprB, stoi(operandsList[2]->value));
  }
  else if(operandsList[2]->type == "symbol") {
    return loadOrJumpOrCallWithSymbol(JMP, JMP_BGT_MOD, 0, gprA, gprB, operandsList[2]->value);
  }
  else {
    cout << "Invalid operand type for bgt instruction" << endl;
    return -1;
  }
}

int Instruction::pushHandler() {

  uint8_t gpr = stoi(operandsList[0]->value);

  formInstructionAndAddToSection(STORE, STORE_PUSH_MOD, SP, 0, gpr, -4);

  return 0;
}

int Instruction::popHandler() {

  uint8_t gpr = stoi(operandsList[0]->value);

  formInstructionAndAddToSection(LOAD, LOAD_POP_MOD, gpr, SP, 0, 4);

  return 0;
}

int Instruction::xchgHandler() {

  uint8_t gprA = stoi(operandsList[0]->value);
  uint8_t gprB = stoi(operandsList[1]->value);

  formInstructionAndAddToSection(XCHG, 0, 0, gprA, gprB, 0);

  return 0;
}

int Instruction::addHandler() {

  uint8_t gprA = stoi(operandsList[0]->value);
  uint8_t gprB = stoi(operandsList[1]->value);

  formInstructionAndAddToSection(ARITM, ARITM_ADD_MOD, gprA, gprA, gprB, 0);

  return 0;
}

int Instruction::subHandler() {

  uint8_t gprA = stoi(operandsList[0]->value);
  uint8_t gprB = stoi(operandsList[1]->value);

  formInstructionAndAddToSection(ARITM, ARITM_SUB_MOD, gprA, gprA, gprB, 0);

  return 0;
}

int Instruction::mulHandler() {

  uint8_t gprA = stoi(operandsList[0]->value);
  uint8_t gprB = stoi(operandsList[1]->value);

  formInstructionAndAddToSection(ARITM, ARITM_MUL_MOD, gprA, gprA, gprB, 0);

  return 0;
}

int Instruction::divHandler() {

  uint8_t gprA = stoi(operandsList[0]->value);
  uint8_t gprB = stoi(operandsList[1]->value);

  formInstructionAndAddToSection(ARITM, ARITM_DIV_MOD, gprA, gprA, gprB, 0);

  return 0;
}

int Instruction::notHandler() {

  uint8_t gprA = stoi(operandsList[0]->value);

  formInstructionAndAddToSection(LOGIC, LOGIC_NOT_MOD, gprA, gprA, 0, 0);

  return 0;
}

int Instruction::andHandler() {

  uint8_t gprA = stoi(operandsList[0]->value);
  uint8_t gprB = stoi(operandsList[1]->value);

  formInstructionAndAddToSection(LOGIC, LOGIC_AND_MOD, gprA, gprA, gprB, 0);

  return 0;
}

int Instruction::orHandler() {

  uint8_t gprA = stoi(operandsList[0]->value);
  uint8_t gprB = stoi(operandsList[1]->value);

  formInstructionAndAddToSection(LOGIC, LOGIC_OR_MOD, gprA, gprA, gprB, 0);

  return 0;
}

int Instruction::xorHandler() {

  uint8_t gprA = stoi(operandsList[0]->value);
  uint8_t gprB = stoi(operandsList[1]->value);

  formInstructionAndAddToSection(LOGIC, LOGIC_XOR_MOD, gprA, gprA, gprB, 0);

  return 0;
}

int Instruction::shlHandler() {

  uint8_t gprA = stoi(operandsList[0]->value);
  uint8_t gprB = stoi(operandsList[1]->value);

  formInstructionAndAddToSection(SHIFT, SHIFT_SHL_MOD, gprA, gprA, gprB, 0);

  return 0;
}

int Instruction::shrHandler() {

  uint8_t gprA = stoi(operandsList[0]->value);
  uint8_t gprB = stoi(operandsList[1]->value);

  formInstructionAndAddToSection(SHIFT, SHIFT_SHR_MOD, gprA, gprA, gprB, 0);

  return 0;
}

int Instruction::ldHandler() {

  string operandType = operandsList[0]->type;
  int operand;
  string operandValue = operandsList[0]->value;
  uint8_t gpr = stoi(operandsList[1]->value);
  int disp;

  if(operandType == "$literal"){
    operand = stoi(operandValue);
    loadOrJumpOrCallWithLiteral(LOAD, LOAD_REGDIR_MOD, gpr, PC, 0, operand);
  }
  else if(operandType == "$symbol") {
    loadOrJumpOrCallWithSymbol(LOAD, LOAD_REGDIR_MOD, gpr, PC, 0, operandValue);
  }
  else if(operandType == "literal") {
    operand = stoi(operandValue);
    if(valueWithin12BitRange(operand)) {
      formInstructionAndAddToSection(LOAD, LOAD_REGIND_MOD, gpr, 0, 0, operand);
    } 
    else {
      loadOrJumpOrCallWithLiteral(LOAD, LOAD_REGDIR_MOD, gpr, PC, 0, operand);//place operand in register
      formInstructionAndAddToSection(LOAD, LOAD_REGIND_MOD, gpr, gpr, 0, 0);//indirect load from register
    }
  }
  else if(operandType == "symbol") {
    SymbolTableEntry *entry = symbolTable->getEntry(operandValue);
    if(entry != nullptr) {
      int literal = entry->value - getCurrentSection()->getLocationCounter();
      if(entry->isDefined && entry->sectionNdx == getCurrentSectionNdx() && valueWithin12BitRange(literal)) {
        formInstructionAndAddToSection(LOAD, LOAD_REGIND_MOD, gpr, 0, 0, literal);
      }
      else {
        loadOrJumpOrCallWithSymbol(LOAD, LOAD_REGDIR_MOD, gpr, PC, 0, operandValue);
        formInstructionAndAddToSection(LOAD, LOAD_REGIND_MOD, gpr, gpr, 0, 0);
      }
    }
    else {
      entry = new SymbolTableEntry();
      entry->name = operandValue;
      entry->isDefined = false;
      symbolTable->addEntry(entry);
      formInstructionWithForwardReference(LOAD, LOAD_REGIND_MOD, gpr, PC, 0, operandValue);
      formInstructionAndAddToSection(LOAD, LOAD_REGIND_MOD, gpr, gpr, 0, 0);
    }
  }
  else if(operandType == "register") {
    operand = stoi(operandValue);
    formInstructionAndAddToSection(LOAD, LOAD_REGDIR_MOD, gpr, operand, 0, 0);
  }
  else if(operandType == "regindRegister") {
    operand = stoi(operandValue);
    formInstructionAndAddToSection(LOAD, LOAD_REGIND_MOD, gpr, operand, 0, 0);
  }
  else if(operandType == "litDispRegister") {
    string dispString = operandsList[0]->addittionalData;
    uint8_t gprB = stoi(operandValue);
    disp = stoi(dispString);
    if(valueWithin12BitRange(disp)) {
      formInstructionAndAddToSection(LOAD, LOAD_REGIND_MOD, gpr, gprB, 0, disp);
    }
    else {
      cout << "Instruction ld: invalid displacement, too large." << endl;
    }
  }
  else if(operandType == "symDispRegister") {
    string symbol = operandsList[0]->addittionalData;
    uint8_t gprB = stoi(operandValue);

    SymbolTableEntry *entry = symbolTable->getEntry(symbol);
    if(entry != nullptr && entry->isDefined && entry->sectionNdx == getCurrentSectionNdx()) {
      disp = entry->value - getCurrentSection()->getLocationCounter();
      if(valueWithin12BitRange(disp)) {
        formInstructionAndAddToSection(LOAD, LOAD_REGIND_MOD, gpr, gprB, 0, disp);
        return 0;
      }
      else {
        cout << "Instruction ld: invalid displacement, too large." << endl;
        return -1;
      } 
    }
    else {
      if(entry == nullptr) {
        entry = new SymbolTableEntry();
        entry->name = symbol;
        entry->isDefined = false;
        symbolTable->addEntry(entry);
      }
      formInstructionWithForwardReference(LOAD, LOAD_REGIND_MOD, gpr, gprB, 0, symbol);
    }
  }

  return 0;
}

int Instruction::stHandler() {

  string operandType = operandsList[1]->type;
  int operand;
  string operandValue = operandsList[1]->value;
  uint8_t gpr = stoi(operandsList[0]->value);
  int disp;
  uint8_t gprB;

  if(operandType == "$literal" || operandType == "$symbol" || operandType == "register") {
    cout << "Instruction st: invalid operand." << endl;
    return -1;
  }
  else if(operandType == "literal") {
    operand = stoi(operandValue);
    if(valueWithin12BitRange(operand)) {
      formInstructionAndAddToSection(STORE, STORE_MEMDIR_MOD, 0, 0, gpr, operand);
    } 
    else {
      getCurrentSection()->addLiteralToPool(operand);
      formInstructionAndAddToSection(STORE, STORE_MEMIND_MOD, PC, 0, gpr, 0);
    }
  }
  else if(operandType == "symbol") {
    SymbolTableEntry *entry = symbolTable->getEntry(operandValue);
    if(entry != nullptr) {
      int literal = entry->value - getCurrentSection()->getLocationCounter();
      if(entry->isDefined && entry->sectionNdx == getCurrentSectionNdx() && valueWithin12BitRange(literal)) {
        formInstructionAndAddToSection(STORE, STORE_MEMDIR_MOD, 0, 0, gpr, literal);
        return 0;
      }
    }
    if(entry == nullptr) {
      entry = new SymbolTableEntry();
      entry->name = operandValue;
      entry->isDefined = false;
      symbolTable->addEntry(entry);
    }
    formInstructionWithForwardReference(STORE, STORE_MEMIND_MOD, PC, 0, gpr, operandValue);
  }
  else if(operandType == "regindRegister") {
    gprB = stoi(operandValue);
    formInstructionAndAddToSection(STORE, STORE_MEMDIR_MOD, gprB, 0, gpr, 0);
  }
  else if(operandType == "litDispRegister") {
    string dispString = operandsList[1]->addittionalData;
    gprB = stoi(operandValue);
    disp = stoi(dispString);

    if(valueWithin12BitRange(disp)) {
      formInstructionAndAddToSection(STORE, STORE_MEMDIR_MOD, gprB, 0, gpr, disp);
    }
    else {
      cout << "Instruction st: invalid displacement, too large." << endl;
      return -1;
    }
  }
  else if(operandType == "symDispRegister") {
    string symbol = operandsList[1]->addittionalData;
    gprB = stoi(operandValue);

    SymbolTableEntry *entry = symbolTable->getEntry(symbol);
    if(entry != nullptr && entry->isDefined && entry->sectionNdx == getCurrentSectionNdx()) {
      disp = entry->value - getCurrentSection()->getLocationCounter();
      if(valueWithin12BitRange(disp)) {
        formInstructionAndAddToSection(STORE, STORE_MEMDIR_MOD, gpr, gprB, 0, disp);
        return 0;
      }
      else {
        cout << "Instruction st: invalid displacement, too large." << endl;
        return -1;
      } 
    }
    else {
      if(entry == nullptr) {
        entry = new SymbolTableEntry();
        entry->name = symbol;
        entry->isDefined = false;
        symbolTable->addEntry(entry);
      }
      formInstructionWithForwardReference(STORE, STORE_MEMDIR_MOD, gpr, gprB, 0, symbol);
    }
  }
  else {
    cout << "Instruction st: invalid operand." << endl;
    cout << operandType << endl;
    cout << operandValue << endl;
    return -1;
  }

  return 0;
}

int Instruction::csrrdHandler() {

  string csr = operandsList[0]->value;
  uint8_t gpr = stoi(operandsList[1]->value);

  uint8_t csrID = detectCsrId(csr);

  formInstructionAndAddToSection(LOAD, LOAD_CSRRD_MOD, gpr, csrID, 0, 0);

  return 0;
}

int Instruction::csrwrHandler() {

  uint8_t gpr = stoi(operandsList[0]->value);
  string csr = operandsList[1]->value;

  uint8_t csrID = detectCsrId(csr);

  formInstructionAndAddToSection(LOAD, LOAD_CSRWR_MOD, csrID, gpr, 0, 0);

  return 0;
}