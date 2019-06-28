#include "hoc.h"

void warn_token(Token* tok, char* msg) {
  dump_token(tok);
  eprintf("%s\n", msg);
}

noreturn void bad_token(Token* tok, char* msg) {
  warn_token(tok, msg);
  exit(1);
}

void dump_token(Token* tok) {
  switch (tok->tag) {
  case TINT:
    eprintf("[INT %d]", tok->integer);
    break;
  case TIDENT:
    eprintf("[IDENT %s]", tok->ident);
    break;
  case TLE:
    eprintf("[LE]");
    break;
  case TLT:
    eprintf("[LT]");
    break;
  case TGE:
    eprintf("[GE]");
    break;
  case TGT:
    eprintf("[GT]");
    break;
  case TEQ:
    eprintf("[EQ]");
    break;
  case TNE:
    eprintf("[NE]");
    break;
  case TPLUS:
    eprintf("[PLUS]");
    break;
  case TMINUS:
    eprintf("[MINUS]");
    break;
  case TASTERISK:
    eprintf("[ASTERISK]");
    break;
  case TSLASH:
    eprintf("[SLASH]");
    break;
  case TAND:
    eprintf("[AND]");
    break;
  case TEQUAL:
    eprintf("[EQUAL]");
    break;
  case TLPAREN:
    eprintf("[LPAREN]");
    break;
  case TRPAREN:
    eprintf("[RPAREN]");
    break;
  case TLBRACE:
    eprintf("[LBRACE]");
    break;
  case TRBRACE:
    eprintf("[RBRACE]");
    break;
  case TSEMICOLON:
    eprintf("[SEMICOLON]");
    break;
  case TCOMMA:
    eprintf("[COMMA]");
    break;
  case TEOF:
    eprintf("[EOF]");
    break;
  }
}
