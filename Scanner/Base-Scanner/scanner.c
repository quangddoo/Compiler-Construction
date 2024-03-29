#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "reader.h"
#include "charcode.h"
#include "token.h"
#include "error.h"


extern int lineNo;
extern int colNo;
extern int currentChar;

extern CharCode charCodes[];

/***************************************************************/

void skipBlank() {
  while (charCodes[currentChar] == CHAR_SPACE && currentChar != EOF) {
    readChar();
  }
}

void skipComment() {
  while (currentChar != EOF) {
    readChar();
    switch(charCodes[currentChar]) {
      case CHAR_TIMES:
        readChar();
        if (charCodes[currentChar] == CHAR_RPAR) {
          readChar();
          return;
        } else error(ERR_ENDOFCOMMENT, lineNo, colNo);
      case CHAR_RPAR:
        error(ERR_ENDOFCOMMENT, lineNo, colNo);
      default:
        break;
    }
  }
}

Token* readIdentKeyword(void) {
  Token * token = makeToken(TK_IDENT, lineNo, colNo);
  int i = 0;

  while (currentChar != EOF && (charCodes[currentChar] == CHAR_LETTER || charCodes[currentChar] == CHAR_DIGIT)) {
    if (i >= MAX_IDENT_LEN) error(ERR_IDENTTOOLONG, lineNo, colNo);
    token->string[i++] = currentChar;
    readChar();
  }
  token->string[i] = '\0';

  TokenType tokenType = checkKeyword(token->string);
  if (tokenType != TK_NONE) token->tokenType = tokenType;

  return token;
}

Token* readNumber(void) {
  Token * token = makeToken(TK_NUMBER, lineNo, colNo);
  int i = 0;

  while (currentChar != EOF && charCodes[currentChar] == CHAR_DIGIT) {
    if (i >= MAX_IDENT_LEN) error(ERR_IDENTTOOLONG, lineNo, colNo);
    token->string[i++] = currentChar;
    readChar();
  }
  token->string[i] = '\0';
  token->value = atoi(token->string);

  return token;
}

Token* readConstChar(void) {
  Token * token = makeToken(TK_CHAR, lineNo, colNo);
  int i = 0;

  readChar();
  while (
    currentChar != EOF && charCodes[currentChar] != CHAR_SINGLEQUOTE &&
    ( charCodes[currentChar] == CHAR_LETTER ||
      charCodes[currentChar] == CHAR_DIGIT ||
      charCodes[currentChar] == CHAR_SPACE )
    ) {
    if (i >= MAX_IDENT_LEN) error(ERR_INVALIDCHARCONSTANT, lineNo, colNo);
    token->string[i++] = currentChar;
    readChar();
  }
  token->string[i] = '\0';
  if (charCodes[currentChar] == CHAR_SINGLEQUOTE) {
    readChar();
    return token;
  }
  else error(ERR_INVALIDCHARCONSTANT, lineNo, colNo);
}

Token * createSymbolToken(TokenType type) {
  Token * token = makeToken(type, lineNo, colNo);
  readChar();
  return token;
}

Token* getToken(void) {
  Token *token;

  if (currentChar == EOF) 
    return makeToken(TK_EOF, lineNo, colNo);

  switch (charCodes[currentChar]) {
  case CHAR_SPACE: skipBlank(); return getToken();
  case CHAR_LETTER: return readIdentKeyword();
  case CHAR_DIGIT: return readNumber();
  case CHAR_PLUS: return createSymbolToken(SB_PLUS);
  case CHAR_MINUS: return createSymbolToken(SB_MINUS);
  case CHAR_TIMES: return createSymbolToken(SB_TIMES);
  case CHAR_SLASH: return createSymbolToken(SB_SLASH);
  case CHAR_LT: return createSymbolToken(SB_LT);
  case CHAR_GT: return createSymbolToken(SB_GT);
  case CHAR_EXCLAIMATION:
    token = makeToken(SB_NEQ, lineNo, colNo);
    readChar();
    if (charCodes[currentChar] == CHAR_EQ) {
      readChar();
      return token;
    } else error(ERR_INVALIDSYMBOL, lineNo, colNo);
  case CHAR_EQ: return createSymbolToken(SB_EQ);
  case CHAR_COMMA: return createSymbolToken(SB_COMMA);
  case CHAR_PERIOD: return createSymbolToken(SB_PERIOD);
  case CHAR_COLON:
    token = makeToken(SB_COLON, lineNo, colNo);
    readChar();
    if (charCodes[currentChar] == CHAR_EQ) {
      token->tokenType = SB_ASSIGN;
      readChar();
      return token;
    }
    else return token;
  case CHAR_SEMICOLON: return createSymbolToken(SB_SEMICOLON);
  case CHAR_SINGLEQUOTE: return readConstChar();
  case CHAR_LPAR:
    token = makeToken(SB_LPAR, lineNo, colNo);
    readChar();
    if (charCodes[currentChar] == CHAR_TIMES) {
      skipComment();
      return getToken();
    } else return token;
  case CHAR_RPAR: return createSymbolToken(SB_RPAR);
  default:
    token = makeToken(TK_NONE, lineNo, colNo);
    error(ERR_INVALIDSYMBOL, lineNo, colNo);
    readChar(); 
    return token;
  }
}


/******************************************************************/

void printToken(Token *token) {

  printf("%d-%d:", token->lineNo, token->colNo);

  switch (token->tokenType) {
  case TK_NONE: printf("TK_NONE\n"); break;
  case TK_IDENT: printf("TK_IDENT(%s)\n", token->string); break;
  case TK_NUMBER: printf("TK_NUMBER(%s)\n", token->string); break;
  case TK_CHAR: printf("TK_CHAR(\'%s\')\n", token->string); break;
  case TK_EOF: printf("TK_EOF\n"); break;

  case KW_PROGRAM: printf("KW_PROGRAM\n"); break;
  case KW_CONST: printf("KW_CONST\n"); break;
  case KW_TYPE: printf("KW_TYPE\n"); break;
  case KW_VAR: printf("KW_VAR\n"); break;
  case KW_INTEGER: printf("KW_INTEGER\n"); break;
  case KW_CHAR: printf("KW_CHAR\n"); break;
  case KW_ARRAY: printf("KW_ARRAY\n"); break;
  case KW_OF: printf("KW_OF\n"); break;
  case KW_FUNCTION: printf("KW_FUNCTION\n"); break;
  case KW_PROCEDURE: printf("KW_PROCEDURE\n"); break;
  case KW_BEGIN: printf("KW_BEGIN\n"); break;
  case KW_END: printf("KW_END\n"); break;
  case KW_CALL: printf("KW_CALL\n"); break;
  case KW_IF: printf("KW_IF\n"); break;
  case KW_THEN: printf("KW_THEN\n"); break;
  case KW_ELSE: printf("KW_ELSE\n"); break;
  case KW_WHILE: printf("KW_WHILE\n"); break;
  case KW_DO: printf("KW_DO\n"); break;
  case KW_FOR: printf("KW_FOR\n"); break;
  case KW_TO: printf("KW_TO\n"); break;

  case SB_SEMICOLON: printf("SB_SEMICOLON\n"); break;
  case SB_COLON: printf("SB_COLON\n"); break;
  case SB_PERIOD: printf("SB_PERIOD\n"); break;
  case SB_COMMA: printf("SB_COMMA\n"); break;
  case SB_ASSIGN: printf("SB_ASSIGN\n"); break;
  case SB_EQ: printf("SB_EQ\n"); break;
  case SB_NEQ: printf("SB_NEQ\n"); break;
  case SB_LT: printf("SB_LT\n"); break;
  case SB_LE: printf("SB_LE\n"); break;
  case SB_GT: printf("SB_GT\n"); break;
  case SB_GE: printf("SB_GE\n"); break;
  case SB_PLUS: printf("SB_PLUS\n"); break;
  case SB_MINUS: printf("SB_MINUS\n"); break;
  case SB_TIMES: printf("SB_TIMES\n"); break;
  case SB_SLASH: printf("SB_SLASH\n"); break;
  case SB_LPAR: printf("SB_LPAR\n"); break;
  case SB_RPAR: printf("SB_RPAR\n"); break;
  case SB_LSEL: printf("SB_LSEL\n"); break;
  case SB_RSEL: printf("SB_RSEL\n"); break;
  }
}

int scan(char *fileName) {
  Token * token;

  if (openInputStream(fileName) == IO_ERROR)
    return IO_ERROR;

  token = getToken();
  while (token->tokenType != TK_EOF) {
    printToken(token);
    free(token);
    token = getToken();
  }

  free(token);
  closeInputStream();
  return IO_SUCCESS;
}

/******************************************************************/

int main(int argc, char *argv[]) {
  if (argc <= 1) {
    printf("scanner: no input file.\n");
    return -1;
  }

  if (scan(argv[1]) == IO_ERROR) {
    printf("Can\'t read input file!\n");
    return -1;
  }
    
  return 0;
}
