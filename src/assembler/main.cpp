#include <iostream>
#include <getopt.h>
#include "../../assembly_flex.yy.hpp"
#include "../../assembly_bison.tab.hpp"
#include "../../inc/assembler/spassembler.hpp"
using namespace std;

extern FILE* yyin;

string convertStoO(const string &filename) {
  string output = filename;
  output.replace(output.end() - 1, output.end(), "o");
  return output;
}

int main(int argc, char** argv) {
  if(argc < 2 || argc > 4) {
    cout << "Usage: " << argv[0] << " invalid" << endl;
    return -1;
  }

  static struct option long_options[] = 
  {
    {"output", 0, 0, 'o'},
    {0, 0 ,0 ,0}
  };

  string input_file = argv[argc - 1];
  string output_file = convertStoO(input_file);

  int option_index = 0;
  int c;
  while((c = getopt_long(argc, argv, "o:", long_options, &option_index)) != -1)
  {
    switch(c)
    {
      case 'o':
        output_file = optarg;
        break;
      default:
        break;
    }
  }

  yyin = fopen(input_file.c_str(), "r");
  if(yyin == nullptr) {
    cout << "Error: File " << input_file << " not found" << endl;
    return -1;
  }

  int check = yyparse();

  fclose(yyin);

  if(check != 0) {
    cout << "Error: Parsing failed" << endl;
    return -1;
  }

  SPAssembler *assembler = SPAssembler::getInstance();
  check = assembler->assemble(input_file, output_file);
  if(check != 0) {
    cout << "Error: Assembly failed" << endl;
    return -1;
  }

  return 0;
}