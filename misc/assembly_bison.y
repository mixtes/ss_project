%{
  #include <stdio.h>

  #include "inc/assembler/spassembler.hpp"

  extern int yylex();
  extern int yyparse();
  extern int line_num;
  extern FILE *yyin;

  void yyerror(const char *msg);
%}

%output "assembly_bison.tab.cpp"
%defines "assembly_bison.tab.hpp"

%union 
{
  int number;
  const char *str;
  struct Operand *operand;
}

%token <number> NUMBERTK
%token <str>    STRINGTK
%token <number> GPREGTK
%token <str>    SREGTK

%token          EOLTK
%token          COLTK
%token          DOLTK
%token          PLSTK
%token          COMTK
%token          OPNTK
%token          CLSTK

%token          COMMENTTK

%token <str>    GLOBALTK
%token <str>    EXTERNTK
%token <str>    SECTIONTK
%token <str>    WORDTK
%token <str>    SKIPTK
%token <str>    ASCIITK
%token <str>    EQUTK
%token <str>    ENDTK

%token <str>    HALTTK
%token <str>    INTTK
%token <str>    IRETTK
%token <str>    CALLTK
%token <str>    RETTK
%token <str>    JMPTK
%token <str>    BEQTK
%token <str>    BNETK
%token <str>    BGTTK
%token <str>    PUSHTK
%token <str>    POPTK
%token <str>    XCHGTK
%token <str>    ADDTK
%token <str>    SUBTK
%token <str>    MULTK
%token <str>    DIVTK
%token <str>    NOTTK
%token <str>    ANDTK
%token <str>    ORTK
%token <str>    XORTK
%token <str>    SHLTK
%token <str>    SHRTK
%token <str>    LDTK
%token <str>    STTK
%token <str>    CSRRDTK
%token <str>    CSRWRTK
%token <str>    OTHERPLAINTEXTTK

%type <operand> operand;
%type <operand> dirList;

%%

prog:
    label line comment EOLTK prog | label line comment
  ;
comment:
    | COMMENTTK { }
label:
    | OTHERPLAINTEXTTK COLTK   { SPAssembler::getInstance()->addCodeComponent($1, "label", NULL); }
  ;
line:
    | inst | directive 
  ;
inst:
    HALTTK              { SPAssembler::getInstance()->addCodeComponent($1, "instruction", NULL); }
    | INTTK             { SPAssembler::getInstance()->addCodeComponent($1, "instruction", NULL); }
    | IRETTK            { SPAssembler::getInstance()->addCodeComponent($1, "instruction", NULL); }
    | CALLTK operand    
      { 
        struct Operand *argOperand = $2;
        argOperand->next = NULL;
        SPAssembler::getInstance()->addCodeComponent($1, "instruction", argOperand);
      }
    | RETTK             { SPAssembler::getInstance()->addCodeComponent($1, "instruction", NULL); }
    | JMPTK operand
      {
        struct Operand *argOperand = $2;
        argOperand->next = NULL;
        SPAssembler::getInstance()->addCodeComponent($1, "instruction", argOperand);
      }
    | BEQTK operand COMTK operand COMTK operand
      {
        struct Operand *gpreg1 = $2, *gpreg2 = $4, *argOperand = $6;
        gpreg1->next = gpreg2;
        gpreg2->next = argOperand;
        argOperand->next = NULL;
        SPAssembler::getInstance()->addCodeComponent($1, "instruction", gpreg1);
      }
    | BNETK operand COMTK operand COMTK operand
      {
        struct Operand *gpreg1 = $2, *gpreg2 = $4, *argOperand = $6;
        gpreg1->next = gpreg2;
        gpreg2->next = argOperand;
        argOperand->next = NULL;
        SPAssembler::getInstance()->addCodeComponent($1, "instruction", gpreg1);
      }
    | BGTTK operand COMTK operand COMTK operand
      {
        struct Operand *gpreg1 = $2, *gpreg2 = $4, *argOperand = $6;
        gpreg1->next = gpreg2;
        gpreg2->next = argOperand;
        argOperand->next = NULL;
        SPAssembler::getInstance()->addCodeComponent($1, "instruction", gpreg1);
      }
    | PUSHTK operand
      {
        struct Operand *gpreg = $2;
        gpreg->next = NULL;
        SPAssembler::getInstance()->addCodeComponent($1, "instruction", gpreg);
      }
    | POPTK operand
      {
        struct Operand *gpreg = $2;
        gpreg->next = NULL;
        SPAssembler::getInstance()->addCodeComponent($1, "instruction", gpreg);
      }
    | XCHGTK operand COMTK operand
      {
        struct Operand *gpregS = $2, *gpregD = $4;
        gpregS->next = gpregD;
        gpregD->next = NULL;
        SPAssembler::getInstance()->addCodeComponent($1, "instruction", gpregS);
      }
    | ADDTK operand COMTK operand
      {
        struct Operand *gpregS = $2, *gpregD = $4;
        gpregS->next = gpregD;
        gpregD->next = NULL;
        SPAssembler::getInstance()->addCodeComponent($1, "instruction", gpregS);
      }
    | SUBTK operand COMTK operand
      {
        struct Operand *gpregS = $2, *gpregD = $4;
        gpregS->next = gpregD;
        gpregD->next = NULL;
        SPAssembler::getInstance()->addCodeComponent($1, "instruction", gpregS);
      }
    | MULTK operand COMTK operand
      {
        struct Operand *gpregS = $2, *gpregD = $4;
        gpregS->next = gpregD;
        gpregD->next = NULL;
        SPAssembler::getInstance()->addCodeComponent($1, "instruction", gpregS);
      }
    | DIVTK operand COMTK operand
      {
        struct Operand *gpregS = $2, *gpregD = $4;
        gpregS->next = gpregD;
        gpregD->next = NULL;
        SPAssembler::getInstance()->addCodeComponent($1, "instruction", gpregS);
      }
    | NOTTK operand
      {
        struct Operand *gpreg = $2;
        gpreg->next = NULL;
        SPAssembler::getInstance()->addCodeComponent($1, "instruction", gpreg);
      }
    | ANDTK operand COMTK operand
      {
        struct Operand *gpregS = $2, *gpregD = $4;
        gpregS->next = gpregD;
        gpregD->next = NULL;
        SPAssembler::getInstance()->addCodeComponent($1, "instruction", gpregS);
      }
    | ORTK operand COMTK operand
      {
        struct Operand *gpregS = $2, *gpregD = $4;
        gpregS->next = gpregD;
        gpregD->next = NULL;
        SPAssembler::getInstance()->addCodeComponent($1, "instruction", gpregS);
      }
    | XORTK operand COMTK operand
      {
        struct Operand *gpregS = $2, *gpregD = $4;
        gpregS->next = gpregD;
        gpregD->next = NULL;
        SPAssembler::getInstance()->addCodeComponent($1, "instruction", gpregS);
      }
    | SHLTK operand COMTK operand
      {
        struct Operand *gpregS = $2, *gpregD = $4;
        gpregS->next = gpregD;
        gpregD->next = NULL;
        SPAssembler::getInstance()->addCodeComponent($1, "instruction", gpregS);
      }
    | SHRTK operand COMTK operand
      {
        struct Operand *gpregS = $2, *gpregD = $4;
        gpregS->next = gpregD;
        gpregD->next = NULL;
        SPAssembler::getInstance()->addCodeComponent($1, "instruction", gpregS);
      }
    | LDTK operand COMTK operand
      {
        struct Operand *argOperand = $2, *gpreg = $4;
        argOperand->next = gpreg;
        gpreg->next = NULL;
        SPAssembler::getInstance()->addCodeComponent($1, "instruction", argOperand);
      }
    | STTK operand COMTK operand
      {
        struct Operand *gpreg = $2, *argOperand = $4;
        gpreg->next = argOperand;
        argOperand->next = NULL;
        SPAssembler::getInstance()->addCodeComponent($1, "instruction", gpreg);
      }
    | CSRRDTK operand COMTK operand
      {
        struct Operand *sysreg = $2, *gpreg = $4;
        sysreg->next = gpreg;
        gpreg->next = NULL;
        SPAssembler::getInstance()->addCodeComponent($1, "instruction", sysreg);
      }
    | CSRWRTK operand COMTK operand
      {
        struct Operand *gpreg = $2, *sysreg = $4;
        gpreg->next = sysreg;
        sysreg->next = NULL;
        SPAssembler::getInstance()->addCodeComponent($1, "instruction", gpreg);
      }
  ;
directive:
    GLOBALTK dirList                               
      { 
        struct Operand *operandList = SPAssembler::getInstance()->getHoardHead();
        SPAssembler::getInstance()->addCodeComponent($1, "directive", operandList);
      }
    | EXTERNTK dirList
      {
        struct Operand *operandList = SPAssembler::getInstance()->getHoardHead();
        SPAssembler::getInstance()->addCodeComponent($1, "directive", operandList);
      }
    | SECTIONTK OTHERPLAINTEXTTK
      {
        struct Operand *sectionName = formOperand("sectionName", $2);
        SPAssembler::getInstance()->addCodeComponent($1, "directive", sectionName);
      }
    | WORDTK dirList
      {
        struct Operand *operandList = SPAssembler::getInstance()->getHoardHead();
        SPAssembler::getInstance()->addCodeComponent($1, "directive", operandList);
      }
    | SKIPTK NUMBERTK
      {
        struct Operand *literal = formOperand("literal", std::to_string($2));
        SPAssembler::getInstance()->addCodeComponent($1, "directive", literal);
      }
    | ASCIITK STRINGTK
      {
        struct Operand *string = formOperand("string", $2);
        SPAssembler::getInstance()->addCodeComponent($1, "directive", string);
      }
    | EQUTK OTHERPLAINTEXTTK COMTK expression
      {

      }
    | ENDTK
      {
        SPAssembler::getInstance()->addCodeComponent($1, "directive", NULL);
      }
  ;
dirList:
    OTHERPLAINTEXTTK COMTK dirList                 
      { 
        $$ = formOperand("symbol", $1);
        SPAssembler::getInstance()->hoardOperand($$); 
      }
    | OTHERPLAINTEXTTK                            
      {
        $$ = formOperand("symbol", $1);
        SPAssembler::getInstance()->hoardOperand($$);
      }
    | NUMBERTK COMTK dirList
      {
        $$ = formOperand("literal", std::to_string($1));
        SPAssembler::getInstance()->hoardOperand($$);
      }
    | NUMBERTK
      {
        $$ = formOperand("literal", std::to_string($1));
        SPAssembler::getInstance()->hoardOperand($$);
      }
  ;
expression:

  ;
operand:
    DOLTK NUMBERTK                                { $$ = formOperand("$literal", std::to_string($2)); }
    | DOLTK OTHERPLAINTEXTTK                      { $$ = formOperand("$symbol", $2); }
    | NUMBERTK                                    { $$ = formOperand("literal", std::to_string($1)); }
    | OTHERPLAINTEXTTK                            { $$ = formOperand("symbol", $1); }
    | GPREGTK                                     { $$ = formOperand("register", std::to_string($1)); }
    | SREGTK                                      { $$ = formOperand("sysRegister", $1); }
    | OPNTK GPREGTK CLSTK                         { $$ = formOperand("regindRegister", std::to_string($2)); }
    | OPNTK GPREGTK PLSTK NUMBERTK CLSTK          { $$ = formOperandWithAdditionalData("litDispRegister", std::to_string($2), std::to_string($4)); }
    | OPNTK GPREGTK PLSTK OTHERPLAINTEXTTK CLSTK  { $$ = formOperandWithAdditionalData("symDispRegister", std::to_string($2), $4); }
  ;
%%

void yyerror(const char *msg) 
{
  cout << "parsing error on line " << line_num << msg << endl;
  exit(-1);
}