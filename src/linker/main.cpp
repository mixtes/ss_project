#include "../../inc/linker/linker.hpp"

using namespace std;
#include <getopt.h>
#include <iostream>

int main(int argc, char** argv) {

  Linker *linker = Linker::getInstance();

  static struct option long_options[] = 
  {
    {"output", required_argument, 0, 'o'},
    {"place", required_argument, 0, 'p'},
    {"hex", no_argument, 0, 'h'},
    {"relocatable", no_argument, 0, 'r'},
    {0, 0 ,0 ,0}
  };

  string output_file = "default.o";
  vector<string> input_files = {};

  unordered_map<string, uint32_t> section_addresses;

  bool hex_output = false;
  bool relocatable = false;

  int option_index = 0;
  int c;

  while ((c = getopt_long(argc, argv, "o:p:h:r", long_options, &option_index)) != -1)
  {
    switch (c)
    {
      case 'o':
        output_file = optarg;
        break;

      case 'p': {
        // Parse --place=<section_name>@<address>
        string place_arg = optarg;
        uint32_t at_pos = place_arg.find('@');
        if (at_pos != string::npos) {
          string section_name = place_arg.substr(0, at_pos);
          unsigned long address = stoul(place_arg.substr(at_pos + 1), nullptr, 0);
          section_addresses[section_name] = address;
        } else {
          cerr << "Invalid format for --place option. Expected format: --place=<section_name>@<address>" << endl;
          return 1;
        }
        break;
      }

      case 'h':
        hex_output = true;
        break;

      case 'r':
        relocatable = true;
        break;

      default:
        break;
    }
  }

  // Handle input files (remaining command-line arguments)
  for (int i = optind; i < argc; i++) {
    input_files.push_back(argv[i]);
  }

  if(input_files.size() == 0) {
    cerr << "No input files provided." << endl;
    return 1;
  }

  if(hex_output && relocatable) {
    cerr << "Cannot output both hex and relocatable file." << endl;
    return 1;
  }

  if(!hex_output && !relocatable) {
    cerr << "Must specify output format: --hex or --relocatable" << endl;
    return 1;
  }

  int check = linker->link(input_files, output_file, section_addresses, hex_output, relocatable);

  if(check != 0) {
    cerr << "Linker: Error during linking." << endl;
    return 1;
  }

  return 0;
}