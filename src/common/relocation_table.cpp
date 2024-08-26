#include "../../inc/common/relocation_table.hpp"

vector<unordered_map<int, RelocationTable*>> RelocationTable::fileNoToRelocationTablesArray = vector<unordered_map<int, RelocationTable*>>();

void RelocationTable::addEntry(int sectionNdx, int offset, RelocationType type, int symbolIndex) {

  RelocationTableEntry *entry = new RelocationTableEntry();
  entry->sectionNdx = sectionNdx;
  entry->offset = offset;
  entry->relocationType = type;
  entry->symbolIndex = symbolIndex;
  table.push_back(entry);
}

void RelocationTable::printToOutput(ofstream &output) {

  //"Section  Offset  Type  Symbol"
  char header[4] = {'R', 'E', 'L', 'T'};
  output << header << endl;
  
  for(auto& entry : table) {
    output << entry->sectionNdx << "\t" << entry->offset << "\t" << entry->relocationType << "\t" << entry->symbolIndex << endl;
  }

  header[0] = 'E'; header[1] = 'N'; header[2] = 'D'; header[3] = 'R';
  output << header << endl;
}

void RelocationTable::extractRelocationTable(ifstream &input, int fileNo, int sectionNdx) {
  string line;
  string hexNum;
  int relocationType;
  RelocationTable *relcoationTable = new RelocationTable();

  istringstream iss;

  while(getline(input, line)) {
    if(line == "ENDR") {
      break;
    }
    iss.clear();
    iss.str(line);
    RelocationTableEntry *entry = new RelocationTableEntry();

    iss >> hexNum;
    entry->sectionNdx = stoi(hexNum, nullptr, 16);
    iss >> hexNum;
    entry->offset = stoi(hexNum, nullptr, 16); 
    iss >> hexNum;
    relocationType = stoi(hexNum, nullptr, 16);
    entry->relocationType = (RelocationType)relocationType;
    iss >> hexNum;
    entry->symbolIndex = stoi(hexNum, nullptr, 16);
    
    relcoationTable->table.push_back(entry);
  }

  addRelocationTableToArray(fileNo, relcoationTable, sectionNdx);
}
