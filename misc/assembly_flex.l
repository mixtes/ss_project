%{
  #include <iostream>
  using namespace std;
  #include <string>

  #include "assembly_bison.tab.hpp"
  #include "inc/assembler/spassembler.hpp"

  int line_num = 1;
%}

%option outfile="assembly_flex.yy.cpp"
%option header-file="assembly_flex.yy.hpp"
%option noyywrap
%option nounput

EOLSGN "\n"
COLSGN ":"
DOLSGN "$"
PLSSGN "+"
COMSGN ","
OPNSGN "["
CLSSGN "]"

COMMENT     "#"[^\n\r]*
WHITESPACE  [ \t\r]*
ZERODEC     "0"
NUMBIN      0[bB][01]+
NUMOCT      0[0-7]+
NUMDEC      [1-9][0-9]*
NUMHEX      0[xX][0-9a-fA-F]+
STRING      \".+\"
GPREG       %r([0-9]|1[0-5])
DIRECTIVE   \.[a-zA-z]+
PLAINTEXT   [a-zA-Z_][a-zA-Z0-9_]*

%%

{COMMENT}         { return COMMENTTK; }
{WHITESPACE}      ;
{ZERODEC}         {
  char *ptr = strdup(yytext);
  yylval.number = stoi(ptr, NULL, 10);
  return NUMBERTK;
}
{NUMBIN}          {
  char *ptr = strdup(yytext);
  yylval.number = stol(ptr + 2, NULL, 2);
  return NUMBERTK;
}
{NUMOCT}          {
  char *ptr = strdup(yytext);
  yylval.number = stol(ptr + 1, NULL, 8);
  return NUMBERTK;
}
{NUMDEC}          {
  char *ptr = strdup(yytext);
  yylval.number = stol(ptr, NULL, 10);
  return NUMBERTK;
}
{NUMHEX}          {
  char *ptr = strdup(yytext);
  yylval.number = stol(ptr + 2, NULL, 16);
  return NUMBERTK;
}
{STRING}          {
  yylval.str = strdup(yytext);
  return STRINGTK;
}
{GPREG}           {
  char *ptr = strdup(yytext);
  yylval.number = stoi(ptr + 2, NULL, 10);
  return GPREGTK;
}
"%sp"           {
  yylval.number = 14;
  return GPREGTK;
}
"%pc"           {
  yylval.number = 15;    
  return GPREGTK;
}
"%status"       {
  yylval.str = "status";
  return SREGTK;
}
"%handler"      {
  yylval.str = "handler";
  return SREGTK;
}
"%cause"        {
  yylval.str = "cause";
  return SREGTK;
}
{DIRECTIVE}       {
  yylval.str = strdup(yytext);
  if(strcmp(yylval.str, ".global") == 0) {
    return GLOBALTK;
  }
  else if(strcmp(yylval.str, ".extern") == 0) {
    return EXTERNTK;
  }
  else if(strcmp(yylval.str, ".section") == 0) {
    return SECTIONTK;
  }
  else if(strcmp(yylval.str, ".word") == 0) {
    return WORDTK;
  }
  else if(strcmp(yylval.str, ".skip") == 0) {
    return SKIPTK;
  }
  else if(strcmp(yylval.str, ".ascii") == 0) {
    return ASCIITK;
  }
  else if(strcmp(yylval.str, ".equ") == 0) {
    return EQUTK;
  }
  else if(strcmp(yylval.str, ".end") == 0) {
    return ENDTK;
  }
}
{COLSGN}          { return COLTK; }
{EOLSGN}          { ++line_num; return EOLTK; }
{DOLSGN}          { return DOLTK; }
{PLSSGN}          { return PLSTK; }
{COMSGN}          { return COMTK; }
{OPNSGN}          { return OPNTK; }
{CLSSGN}          { return CLSTK; }
{PLAINTEXT}       {
  yylval.str = strdup(yytext);
  if(strcmp(yylval.str, "halt") == 0) {
    return HALTTK;
  }
  else if(strcmp(yylval.str, "int") == 0) {
    return INTTK;
  }
  else if(strcmp(yylval.str, "iret") == 0) {
    return IRETTK;
  }
  else if(strcmp(yylval.str, "call") == 0) {
    return CALLTK;
  }
  else if(strcmp(yylval.str, "ret") == 0) {
    return RETTK;
  }
  else if(strcmp(yylval.str, "jmp") == 0) {
    return JMPTK;
  }
  else if(strcmp(yylval.str, "beq") == 0) {
    return BEQTK;
  }
  else if(strcmp(yylval.str, "bne") == 0) {
    return BNETK;
  }
  else if(strcmp(yylval.str, "bgt") == 0) {
    return BGTTK;
  }
  else if(strcmp(yylval.str, "push") == 0) {
    return PUSHTK;
  }
  else if(strcmp(yylval.str, "pop") == 0) {
    return POPTK;
  }
  else if(strcmp(yylval.str, "xchg") == 0) {
    return XCHGTK;
  }
  else if(strcmp(yylval.str, "add") == 0) {
    return ADDTK;
  }
  else if(strcmp(yylval.str, "sub") == 0) {
    return SUBTK;
  }
  else if(strcmp(yylval.str, "mul") == 0) {
    return MULTK;
  }
  else if(strcmp(yylval.str, "div") == 0) {
    return DIVTK;
  }
  else if(strcmp(yylval.str, "not") == 0) {
    return NOTTK;
  }
  else if(strcmp(yylval.str, "and") == 0) {
    return ANDTK;
  }
  else if(strcmp(yylval.str, "or") == 0) {
    return ORTK;
  }
  else if(strcmp(yylval.str, "xor") == 0) {
    return XORTK;
  }
  else if(strcmp(yylval.str, "shl") == 0) {
    return SHLTK;
  }
  else if(strcmp(yylval.str, "shr") == 0) {
    return SHRTK;
  }
  else if(strcmp(yylval.str, "ld") == 0) {
    return LDTK;
  }
  else if(strcmp(yylval.str, "st") == 0) {
    return STTK;
  }
  else if(strcmp(yylval.str, "csrrd") == 0) {
    return CSRRDTK;
  }
  else if(strcmp(yylval.str, "csrwr") == 0) {
    return CSRWRTK;
  }
  else {
    return OTHERPLAINTEXTTK;
  }
}
.               { cout << "found some random char: \n" << yytext; }
%%
