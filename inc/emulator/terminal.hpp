#ifndef _TERMINAL_H
#define _TERMINAL_H

#include <cstdint>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
using namespace std;

class Terminal {
  public:

    Terminal();

    int32_t getTermIn() {
      return term_in;
    }

    void readInputIfAvailable();

    void writeOutput(int32_t output);

    bool checkForInterrupt() {
      return terminalInterrupt;
    }

    void interruptHandled() {
      terminalInterrupt = false;
    }

    ~Terminal();
    
  private:

    int32_t term_in = 0;
    int32_t term_out;
    termios tio;
    tcflag_t old_flags;

    bool terminalInterrupt = false;
};

#endif