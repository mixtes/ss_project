#ifndef _EMULATOR_H
#define _EMULATOR_H

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <unordered_map> 
using namespace std;

#include "terminal.hpp"
#include "timer.hpp"

#include "../common/instruction_component.hpp"

#define STATUS_TERMINAL_INTERRUPT 0x01
#define STATUS_TIMER_INTERRUPT 0x02
#define STATUS_INTERUPT_MASK 0x04

#define START_ADDRESS 0x40000000

struct CPU {
  int32_t reg[16];
  int32_t creg[3];
};

class Emulator {
  public:

    static Emulator* getInstance() {
      static Emulator instance;
      return &instance;
    }

    int emulate(string input_file);

    ~Emulator() {}

  private:

    CPU cpu;
    unordered_map<uint32_t, uint8_t> memory;
    Terminal terminal;
    Timer timer;

    Emulator() { cpu.reg[PC] = START_ADDRESS; }
    Emulator(Emulator const&) = delete;
    void operator=(Emulator const&) = delete;

    bool halt = false;

    void initializeMemory(ifstream &file);
    void printFinishingState();
    int executeProgram();
    int32_t fetchInstructionFromAddress(uint32_t address);
    int decodeAndExecuteInstruction(int32_t instruction);
    void checkForInterrupt();

    int haltHandler(int32_t instruction);
    int intHandler(int32_t instruction);
    int callHandler(int32_t instruction);
    int jmpHandler(int32_t instruction);
    int xchgHandler(int32_t instruction);
    int aritmHandler(int32_t instruction);
    int logicHandler(int32_t instruction);
    int shiftHandler(int32_t instruction);
    int storeHandler(int32_t instruction);
    int loadHandler(int32_t instruction);

    void writeToGPR(int reg, int32_t data);
    void writeToCSR(int reg, int32_t data);
    void writeQuadByteToMemory(uint32_t address, int32_t data);
    int32_t readQuadByteFromMemory(uint32_t address);

    int32_t getInstructionMod(int32_t instruction);
    int32_t getInstructionREGA(int32_t instruction);
    int32_t getInstructionREGB(int32_t instruction);
    int32_t getInstructionREGC(int32_t instruction);
    int32_t getInstructionDISP(int32_t instruction);
};

#endif