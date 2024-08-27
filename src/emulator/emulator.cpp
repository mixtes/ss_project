#include "../../inc/emulator/emulator.hpp"

int Emulator::emulate(string input_file) {

  ifstream file(input_file);
  if(!file.is_open()) {
    cout << "Error: File " << input_file << " not found." << endl;
    return -1;
  }

  initializeMemory(file);

  executeProgram();

  printFinishingState();

  return 0;
}

void Emulator::initializeMemory(ifstream &file) {

  string line;
  istringstream iss(line);

  uint32_t address;
  uint16_t byte;

  while(getline(file, line)) {
    // expected format: 00000000: 00 00 00 00 00 00 00 00
    iss.str(line);

    iss >> hex >> address;
    iss.ignore(2); // skip ":"

    while(iss >> hex >> byte) {
      memory[address++] = static_cast<uint8_t>(byte);
    }

    iss.clear();
  }

  file.close();
}

void Emulator::printFinishingState() {
  cout << endl;
  cout << "-----------------------------------------------------------------" << endl;
  cout << "Emulated processor executed halt instruction" << endl;
  cout << "Emulated processor state: " << endl;

  // expected output format: 
  // r0=0x00000000   r1=0x00000000    r2=0x00000000   r3=0x00000000
  // r4=0x00000000   r5=0x00000000    r6=0x00000000   r7=0x00000000
  // r8=0x00000000   r9=0x00000000   r10=0x00000000  r11=0x00000000
  //r12=0x00000000  r13=0x00000000   r14=0x00000000  r15=0x00000000

  for(int i = 0; i < 16; i++) {
    stringstream ss;
    ss << "r" << i;
    string reg = ss.str();

    cout << setw(3) << right << setfill(' ') << reg << "=0x" << hex << setw(8) << setfill('0') << cpu.reg[i] << "\t";

    if(i % 4 == 3) cout << endl;
  }
}

int Emulator::executeProgram() {
  
  uint32_t pc;
  int32_t instruction;

  int check = 0;

  while(!halt) {
    pc = static_cast<uint32_t>(cpu.reg[PC]);
    instruction = fetchInstructionFromAddress(pc);

    check = decodeAndExecuteInstruction(instruction);

    if(check != 0) {
      cout << "Error: Bad instruction." << endl;
      return -1;
    }

    terminal.readInputIfAvailable();
    checkForInterrupt();
  }

  return 0;
}

int32_t Emulator::fetchInstructionFromAddress(uint32_t address) {
  int32_t instruction = 0;

  for(int i = 0; i < 4; i++) {
    instruction |= memory[address + (3 - i)] << (8 * i);
  }
  cpu.reg[PC] += 4;

  return instruction;
}

int Emulator::decodeAndExecuteInstruction(int32_t instruction) {
  // big endian instructions
  int opcode = (instruction >> 28) & 0x0F;
  int check;

  switch(opcode) {
    case 0x0:
      check = haltHandler(instruction);
      break;
    case 0x1:
      check = intHandler(instruction);
      break;
    case 0x2:
      check = callHandler(instruction);
      break;
    case 0x3:
      check = jmpHandler(instruction);
      break;
    case 0x4:
      check = xchgHandler(instruction);
      break;
    case 0x5:
      check = aritmHandler(instruction);
      break;
    case 0x6:
      check = logicHandler(instruction);
      break;
    case 0x7:
      check = shiftHandler(instruction);
      break;
    case 0x8:
      check = storeHandler(instruction);
      break;
    case 0x9:
      check = loadHandler(instruction);
      break;
    default:
      return -1;
  }

  return check;
}

int32_t Emulator::getInstructionMod(int32_t instruction) {
  return (instruction >> 24) & 0x0F;
}

int32_t Emulator::getInstructionREGA(int32_t instruction) {
  return (instruction >> 20) & 0x0F;
}

int32_t Emulator::getInstructionREGB(int32_t instruction) {
  return (instruction >> 16) & 0x0F;
}

int32_t Emulator::getInstructionREGC(int32_t instruction) {
  return (instruction >> 12) & 0x0F;
}

int32_t Emulator::getInstructionDISP(int32_t instruction) {
  //sign extend
  return (instruction & 0x0FFF) | ((instruction & 0x0800) ? 0xFFFFF000 : 0);
}

int Emulator::haltHandler(int32_t instruction) {
  halt = true;
  return 0;
}

int Emulator::intHandler(int32_t instruction) {

  cpu.reg[SP] -= 4;
  uint32_t address = static_cast<uint32_t>(cpu.reg[SP]);
  writeQuadByteToMemory(address, cpu.creg[STATUS]);

  cpu.reg[SP] -= 4;
  address = static_cast<uint32_t>(cpu.reg[SP]);
  writeQuadByteToMemory(address, cpu.reg[PC]);

  cpu.creg[CAUSE] = 4;
  cpu.creg[STATUS] &= ~(STATUS_INTERUPT_MASK);
  cpu.reg[PC] = cpu.creg[HANDLER];

  return 0;
}

int Emulator::callHandler(int32_t instruction) {

  int REGA = getInstructionREGA(instruction);
  int REGB = getInstructionREGB(instruction);
  int DISP = getInstructionDISP(instruction);

  cpu.reg[SP] -= 4;
  uint32_t address = static_cast<uint32_t>(cpu.reg[SP]);
  writeQuadByteToMemory(address, cpu.reg[PC]);

  int32_t mod = getInstructionMod(instruction);
  switch(mod) {
    case CALL_NO_MOD:
      cpu.reg[PC] = cpu.reg[REGA] + cpu.reg[REGB] + DISP;
      break;
    case CALL_MEM_MOD:
      address = static_cast<uint32_t>(cpu.reg[REGA] + cpu.reg[REGB] + DISP);
      cpu.reg[PC] = readQuadByteFromMemory(address);
      break;
    default:
      return -1;
  }

  return 0;
}

int Emulator::jmpHandler(int32_t instruction) {

  int REGA = getInstructionREGA(instruction);
  int REGB = getInstructionREGB(instruction);
  int REGC = getInstructionREGC(instruction);
  int DISP = getInstructionDISP(instruction);

  int32_t mod = getInstructionMod(instruction);
  uint32_t address;

  switch(mod) {
    case JMP_NO_MOD:
      cpu.reg[PC] = cpu.reg[REGA] + DISP;
      break;
    case JMP_BEQ_MOD:
      if(cpu.reg[REGB] == cpu.reg[REGC]) {
        cpu.reg[PC] = cpu.reg[REGA] + DISP;
      }
      break;
    case JMP_BNE_MOD:
      if(cpu.reg[REGB] != cpu.reg[REGC]) {
        cpu.reg[PC] = cpu.reg[REGA] + DISP;
      }
      break;
    case JMP_BGT_MOD:
      if(cpu.reg[REGB] > cpu.reg[REGC]) {
        cpu.reg[PC] = cpu.reg[REGA] + DISP;
      }
      break;
    case JMP_NO_MOD | JMP_MEM_MOD:
      address = static_cast<uint32_t>(cpu.reg[REGA] + DISP);
      cpu.reg[PC] = readQuadByteFromMemory(address);
      break;
    case JMP_BEQ_MOD | JMP_MEM_MOD:
      if(cpu.reg[REGB] == cpu.reg[REGC]) {
        address = static_cast<uint32_t>(cpu.reg[REGA] + DISP);
        cpu.reg[PC] = readQuadByteFromMemory(address);
      }
      break;
    case JMP_BNE_MOD | JMP_MEM_MOD:
      if(cpu.reg[REGB] != cpu.reg[REGC]) {
        address = static_cast<uint32_t>(cpu.reg[REGA] + DISP);
        cpu.reg[PC] = readQuadByteFromMemory(address);
      }
      break;
    case JMP_BGT_MOD | JMP_MEM_MOD:
      if(cpu.reg[REGB] > cpu.reg[REGC]) {
        address = static_cast<uint32_t>(cpu.reg[REGA] + DISP);
        cpu.reg[PC] = readQuadByteFromMemory(address);
      }
      break;
    default:
      return -1;
  }

  return 0;
}

int Emulator::xchgHandler(int32_t instruction) {
  
  int REGB = getInstructionREGB(instruction);
  int REGC = getInstructionREGC(instruction);

  int32_t tmp = cpu.reg[REGB];
  writeToGPR(REGB, cpu.reg[REGC]);
  writeToGPR(REGC, tmp);

  return 0;
}

int Emulator::aritmHandler(int32_t instruction) {

  int REGA = getInstructionREGA(instruction);
  int REGB = getInstructionREGB(instruction);
  int REGC = getInstructionREGC(instruction);

  int32_t mod = getInstructionMod(instruction);

  switch(mod) {
    case ARITM_ADD_MOD:
      writeToGPR(REGA, cpu.reg[REGB] + cpu.reg[REGC]);
      break;
    case ARITM_SUB_MOD:
      writeToGPR(REGA, cpu.reg[REGB] - cpu.reg[REGC]);
      break;
    case ARITM_MUL_MOD:
      writeToGPR(REGA, cpu.reg[REGB] * cpu.reg[REGC]);
      break;
    case ARITM_DIV_MOD:
      writeToGPR(REGA, cpu.reg[REGB] / cpu.reg[REGC]);
      break;
    default:
      return -1;
  }

  return 0;
}

int Emulator::logicHandler(int32_t instruction) {

  int REGA = getInstructionREGA(instruction);
  int REGB = getInstructionREGB(instruction);
  int REGC = getInstructionREGC(instruction);

  int32_t mod = getInstructionMod(instruction);

  switch(mod) {
    case LOGIC_NOT_MOD:
      writeToGPR(REGA, ~cpu.reg[REGB]);
      break;
    case LOGIC_AND_MOD:
      writeToGPR(REGA, cpu.reg[REGB] & cpu.reg[REGC]);
      break;
    case LOGIC_OR_MOD:
      writeToGPR(REGA, cpu.reg[REGB] | cpu.reg[REGC]);
      break;
    case LOGIC_XOR_MOD:
      writeToGPR(REGA, cpu.reg[REGB] ^ cpu.reg[REGC]);
      break;
    default:
      return -1;
  }

  return 0;
}

int Emulator::shiftHandler(int32_t instruction) {

  int REGA = getInstructionREGA(instruction);
  int REGB = getInstructionREGB(instruction);
  int REGC = getInstructionREGC(instruction);

  int32_t mod = getInstructionMod(instruction);

  switch(mod) {
    case SHIFT_SHL_MOD:
      writeToGPR(REGA, cpu.reg[REGB] << cpu.reg[REGC]);
      break;
    case SHIFT_SHR_MOD:
      writeToGPR(REGA, cpu.reg[REGB] >> cpu.reg[REGC]);
      break;
    default:
      return -1;
  }

  return 0;
}

int Emulator::storeHandler(int32_t instruction) {

  int REGA = getInstructionREGA(instruction);
  int REGB = getInstructionREGB(instruction);
  int REGC = getInstructionREGC(instruction);
  int DISP = getInstructionDISP(instruction);

  int32_t mod = getInstructionMod(instruction);
  uint32_t address;

  switch(mod) {
    case STORE_MEMDIR_MOD:
      address = static_cast<uint32_t>(cpu.reg[REGA] + cpu.reg[REGB] + DISP);
      writeQuadByteToMemory(address, cpu.reg[REGC]);
      break;
    case STORE_MEMIND_MOD:
      address = static_cast<uint32_t>(cpu.reg[REGA] + cpu.reg[REGB] + DISP);
      address = static_cast<uint32_t>(readQuadByteFromMemory(address));
      writeQuadByteToMemory(address, cpu.reg[REGC]);
      break;
    case STORE_PUSH_MOD:
      cpu.reg[REGA] += DISP;
      address = static_cast<uint32_t>(cpu.reg[REGA]);
      writeQuadByteToMemory(address, cpu.reg[REGC]);
      break;
    default:
      return -1;
  }

  return 0;
}

int Emulator::loadHandler(int32_t instruction) {
  
  int REGA = getInstructionREGA(instruction);
  int REGB = getInstructionREGB(instruction);
  int REGC = getInstructionREGC(instruction);
  int DISP = getInstructionDISP(instruction);

  int32_t mod = getInstructionMod(instruction);
  uint32_t address;

  switch(mod) {
    case LOAD_CSRRD_MOD:
      writeToGPR(REGA, cpu.creg[REGB]);
      break;
    case LOAD_REGDIR_MOD:
      writeToGPR(REGA, cpu.reg[REGB] + DISP);
      break;
    case LOAD_REGIND_MOD:
      address = static_cast<uint32_t>(cpu.reg[REGB] + cpu.reg[REGC] + DISP);
      writeToGPR(REGA, readQuadByteFromMemory(address));
      break;
    case LOAD_POP_MOD:
      address = static_cast<uint32_t>(cpu.reg[REGB]);
      writeToGPR(REGA, readQuadByteFromMemory(address));
      cpu.reg[REGB] += DISP;
      break;
    case LOAD_CSRWR_MOD:
      writeToCSR(REGA, cpu.reg[REGB]);
      break;
    case LOAD_CSR_SPECIAL_MOD:
      writeToCSR(REGA, cpu.creg[REGB] | DISP);
      break;
    case LOAD_CSR_IND_MOD:
      address = static_cast<uint32_t>(cpu.reg[REGB] + cpu.reg[REGC] + DISP);
      writeToCSR(REGA, readQuadByteFromMemory(address));
      break;
    case LOAD_CSR_POP_MOD:
      address = static_cast<uint32_t>(cpu.reg[REGB]);
      writeToCSR(REGA, readQuadByteFromMemory(address));
      cpu.creg[REGB] += DISP;
      break;
    default:
      return -1;
  }

  return 0;
}

void Emulator::writeToGPR(int reg, int32_t data) {
  if(reg != 0) cpu.reg[reg] = data;
}

void Emulator::writeQuadByteToMemory(uint32_t address, int32_t data) {

  //check if address is for memory mapped terminal registers
  if(address >= 0xFFFFFF00 && address <= 0xFFFFFF03) {
    terminal.writeOutput(data);
    return;
  }

  for(int i = 0; i < 4; i++) {
    memory[address + i] = (data >> (8 * i)) & 0xFF;
  }
}

void Emulator::writeToCSR(int reg, int32_t data) {
  cpu.creg[reg] = data;
}

int32_t Emulator::readQuadByteFromMemory(uint32_t address) {
  
  //check if address is for memory mapped terminal registers
  if(address >= 0xFFFFFF04 && address <= 0xFFFFFF07) {
    return terminal.getTermIn();
  }

  int32_t data = 0;
  for(int i = 0; i < 4; i++) {
    data |= memory[address + i] << (8 * i);
  }

  return data;
}

void Emulator::checkForInterrupt() {
  
  if(cpu.creg[STATUS] & STATUS_INTERUPT_MASK || halt) {
    return;
  }

  if(terminal.checkForInterrupt() && !(cpu.creg[STATUS] & STATUS_TERMINAL_INTERRUPT)) {
    cpu.reg[SP] -= 4;
    uint32_t address = static_cast<uint32_t>(cpu.reg[SP]);
    writeQuadByteToMemory(address, cpu.creg[STATUS]);

    cpu.reg[SP] -= 4;
    address = static_cast<uint32_t>(cpu.reg[SP]);
    writeQuadByteToMemory(address, cpu.reg[PC]);

    cpu.creg[CAUSE] = 3;
    cpu.creg[STATUS] &= ~(STATUS_INTERUPT_MASK);
    cpu.reg[PC] = cpu.creg[HANDLER];

    terminal.interruptHandled();
  }
}