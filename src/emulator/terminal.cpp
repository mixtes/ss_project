#include "../../inc/emulator/terminal.hpp"

Terminal::Terminal() {

  int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);

  tcgetattr(STDIN_FILENO, &tio);
  old_flags = tio.c_lflag;

  tio.c_lflag &= ~ICANON & ~ECHO;
  tcsetattr(STDIN_FILENO, TCSANOW, &tio);
}

void Terminal::readInputIfAvailable() {
  char c;
  ssize_t n = read(STDIN_FILENO, &c, 1);

  if (n >= 0) {
    term_in = c;
    terminalInterrupt = true;
  }
}

void Terminal::writeOutput(int32_t output) {
  term_out = output;
  cout << static_cast<char>(term_out) << flush;
}

Terminal::~Terminal() {
  tio.c_lflag = old_flags;
  tcsetattr(STDIN_FILENO, TCSADRAIN, &tio);
}