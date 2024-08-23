#ifndef _INSTRUCTION_COMPONENT_HPP
#define _INSTRUCTION_COMPONENT_HPP

#include "code_component.hpp"

enum Register {
  R0  = 0x00,
  R1  = 0x01,
  R2  = 0x02,
  R3  = 0x03,
  R4  = 0x04,
  R5  = 0x05,
  R6  = 0x06,
  R7  = 0x07,
  R8  = 0x08,
  R9  = 0x09,
  R10 = 0x0A,
  R11 = 0x0B,
  R12 = 0x0C,
  R13 = 0x0D,
  SP  = 0x0E,
  PC  = 0x0F
};

enum SystemRegister {
  STATUS  = 0x00,
  HANDLER = 0x01,
  CAUSE   = 0x02
};

enum Opcode {
  HALT  = 0x00,
  INT   = 0x01,
  CALL  = 0x02,
  JMP   = 0x03,
  XCHG  = 0x04,
  ARITM = 0x05,
  LOGIC = 0x06,
  SHIFT = 0x07,
  STORE = 0x08,
  LOAD  = 0x09
};

enum CALLModification {
  CALL_NO_MOD = 0x00,
  
  CALL_MEM_MOD = 0x01
};

enum JMPModification {
  JMP_NO_MOD = 0x00,
  JMP_BEQ_MOD = 0x01,
  JMP_BNE_MOD = 0x02,
  JMP_BGT_MOD = 0x03,

  JMP_MEM_MOD = 0x08
};

enum ARITMModification {
  ARITM_ADD_MOD = 0x00,
  ARITM_SUB_MOD = 0x01,
  ARITM_MUL_MOD = 0x02,
  ARITM_DIV_MOD = 0x03
};

enum LOGICModification {
  LOGIC_NOT_MOD = 0x00,
  LOGIC_AND_MOD = 0x01,
  LOGIC_OR_MOD  = 0x02,
  LOGIC_XOR_MOD = 0x03
};

enum SHIFTModification {
  SHIFT_SHL_MOD = 0x00,
  SHIFT_SHR_MOD = 0x01
};

enum STOREModification {
  STORE_MEMDIR_MOD = 0x00,
  STORE_MEMIND_MOD = 0x02,
  STORE_PUSH_MOD = 0x01
};

enum LOADModification {
  LOAD_CSRRD_MOD = 0x00,
  LOAD_REGDIR_MOD = 0x01,
  LOAD_REGIND_MOD = 0x02,
  LOAD_POP_MOD = 0x03,
  LOAD_CSRWR_MOD = 0x04,
  LOAD_CSR_SPECIAL_MOD = 0x05,
  LOAD_CSR_IND_MOD = 0x06,
  LOAD_CSR_POP_MOD = 0x07
};

class Instruction : public CodeComponent{
  
  public:

  Instruction(string instruction, Operand *operands) : CodeComponent(instruction, operands) {
    this->type = "instruction";
  }

  int analize();

  private:

  void formInstructionAndAddToSection(Opcode opcode, uint8_t mod, uint8_t operand1, uint8_t operand2, uint8_t operand3, int16_t displacement);
  void formInstructionWithForwardReference(Opcode opcode, uint8_t mod, uint8_t operand1, uint8_t operand2, uint8_t operand3, string symbol);
  int loadOrJumpOrCallWithLiteral(Opcode opcode, uint8_t mod, uint8_t operand1, uint8_t operand2, uint8_t operand3, int literal);
  int loadOrJumpOrCallWithSymbol(Opcode opcode, uint8_t mod, uint8_t operand1, uint8_t operand2, uint8_t operand3, string symbol);
  int detectCsrId(string csr);

  int haltHandler();

  int intHandler();

  int iretHandler();

  int callHandler();

  int retHandler();

  int jmpHandler();

  int beqHandler();

  int bneHandler();

  int bgtHandler();

  int pushHandler();

  int popHandler();

  int xchgHandler();

  int addHandler();

  int subHandler();

  int mulHandler();

  int divHandler();

  int notHandler();
  
  int andHandler();

  int orHandler();

  int xorHandler();

  int shlHandler();

  int shrHandler();

  int ldHandler();

  int stHandler();

  int csrrdHandler();

  int csrwrHandler();
};

#endif