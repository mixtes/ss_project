#include "../../inc/emulator/emulator.hpp"

#include <iostream>
#include <string>
using namespace std;

int main(int argc, char** argv) {
  if(argc != 2) {
    cout << "Usage: " << argv[0] << " invalid." << endl;
    return -1;
  }

  string input_filename = argv[1];

  Emulator *emulator = Emulator::getInstance();
  int check = emulator->emulate(argv[1]);

  if(check != 0) {
    cout << "Error: Emulation failed." << endl;
    return -1;
  }

  return 0;
}