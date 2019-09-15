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

// Extra options
int op_Underscore_As_Letter           = 1 << 0; // 1
int op_Underscore_As_SpecialChar      = 1 << 1; // 2
int op_Symbol_NEQ                     = 1 << 2; // 4
int op_String_Double_Quote            = 1 << 3; // 8
int op_Single_Quote_As_Letter         = 1 << 4; // 16
int op_String_Length                  = 1 << 5; // 32

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

// op_Underscore_As_Letter = 1 allow '_' inside an ident
// op_Single_Quote_As_Letter = 1 consider single quote as a letter
// op_String_Length = 1 allow string length up to 255 characters, but only take the first 15
Token* readIdentKeyword(void) {
  Token * token = makeToken(TK_IDENT, lineNo, colNo);
  int i = 0;

  while (
    currentChar != EOF &&
    ( charCodes[currentChar] == CHAR_LETTER ||
      charCodes[currentChar] == CHAR_DIGIT  ||
      charCodes[currentChar] == CHAR_SINGLEQUOTE ||
      charCodes[currentChar] == CHAR_UNDERSCORE
      )
    ) {
    if (charCodes[currentChar] == CHAR_SINGLEQUOTE && op_Single_Quote_As_Letter == 0)
      error(ERR_INVALIDSYMBOL, lineNo, colNo);
    else if (charCodes[currentChar] == CHAR_UNDERSCORE && op_Underscore_As_Letter == 0)
      error(ERR_INVALIDSYMBOL, lineNo, colNo);
    if (i >= MAX_IDENT_LEN) {
      if (op_String_Length == 0) error(ERR_IDENTTOOLONG, lineNo, colNo);
      else {
        readChar();
        continue;
      }
    }
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

// op_Underscore_As_Letter = 1 allow '_' inside an ident
// op_String_Double_Quote allow string to be marked by double quote
// op_Single_Quote_As_Letter = 1 consider single quote as a letter
// op_String_Length = 1 allow string length up to 255 characters, but only take the first 15
Token* readConstChar(void) {
  Token * token = makeToken(TK_CHAR, lineNo, colNo);
  int i = 0;
  int openSingleQuote = charCodes[currentChar] == CHAR_SINGLEQUOTE ? 1 : 0;

  readChar();
  while (
    currentChar != EOF && charCodes[currentChar] != CHAR_DOUBLEQUOTE &&
    ( charCodes[currentChar] == CHAR_LETTER ||
      charCodes[currentChar] == CHAR_DIGIT ||
      charCodes[currentChar] == CHAR_SPACE ||
      charCodes[currentChar] == CHAR_SINGLEQUOTE ||
      charCodes[currentChar] == CHAR_UNDERSCORE)
    ) {
    if (charCodes[currentChar] == CHAR_SINGLEQUOTE) {
      if (openSingleQuote == 1) break;
      if (openSingleQuote == 0 && op_Single_Quote_As_Letter == 0) error(ERR_INVALIDCHARCONSTANT, lineNo, colNo);
    } else if (charCodes[currentChar] == CHAR_UNDERSCORE && op_Underscore_As_Letter == 0)
      error(ERR_INVALIDCHARCONSTANT, lineNo, colNo);
    if (i >= MAX_IDENT_LEN) {
      if (op_String_Length == 0) error(ERR_INVALIDCHARCONSTANT, lineNo, colNo);
      else {
        readChar();
        continue;
      }
    }
    token->string[i++] = currentChar;
    readChar();
  }
  token->string[i] = '\0';
  if (
    (charCodes[currentChar] == CHAR_SINGLEQUOTE && openSingleQuote == 1) ||
    (charCodes[currentChar] == CHAR_DOUBLEQUOTE && op_String_Double_Quote == 1 && openSingleQuote == 0)
    ) {
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

// op_Underscore_As_SpecialChar = 1 allow '_' to be considered a symbol
// op_Symbol_NEQ = 1 allow "<>" and "><" to be considered as SB_NEQ
// op_String_Double_Quote allow string to be marked by double quote
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
  case CHAR_LT:
    token = makeToken(SB_LT, lineNo, colNo);
    readChar();
    switch(charCodes[currentChar]) {
      case CHAR_EQ:
        token->tokenType = SB_LE;
        readChar();
        return token;
      case CHAR_GT:
        if (op_Symbol_NEQ == 1) return createSymbolToken(SB_NEQ);
        else error(ERR_INVALIDSYMBOL, lineNo, colNo);
      default:
        error(ERR_INVALIDSYMBOL, lineNo, colNo);
    }
  case CHAR_GT:
    token = makeToken(SB_GT, lineNo, colNo);
    readChar();
    switch(charCodes[currentChar]) {
      case CHAR_EQ:
        token->tokenType = SB_GE;
        readChar();
        return token;
      case CHAR_LT:
        if (op_Symbol_NEQ == 1) return createSymbolToken(SB_NEQ);
        else error(ERR_INVALIDSYMBOL, lineNo, colNo);
      default:
        error(ERR_INVALIDSYMBOL, lineNo, colNo);
    }
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
  case CHAR_DOUBLEQUOTE:
    if (op_String_Double_Quote == 1) return readConstChar();
    else error(ERR_INVALIDSYMBOL, lineNo, colNo);
  case CHAR_LPAR:
    token = makeToken(SB_LPAR, lineNo, colNo);
    readChar();
    if (charCodes[currentChar] == CHAR_TIMES) {
      skipComment();
      return getToken();
    } else return token;
  case CHAR_RPAR: return createSymbolToken(SB_RPAR);
  case CHAR_UNDERSCORE:
    if (op_Underscore_As_SpecialChar == 1)
      return createSymbolToken(SB_UNDERSCORE);
    else
      error(ERR_INVALIDSYMBOL, lineNo, colNo);
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
  case SB_UNDERSCORE: printf("SB_UNDERSCORE\n"); break;
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

void enableOptions(int options) {
  op_Underscore_As_Letter = (op_Underscore_As_Letter == (options & op_Underscore_As_Letter)) ? 1 : 0;
  op_Underscore_As_SpecialChar = (op_Underscore_As_SpecialChar == (options & op_Underscore_As_SpecialChar)) ? 1 : 0;
  op_Symbol_NEQ = (op_Symbol_NEQ == (options & op_Symbol_NEQ)) ? 1 : 0;
  op_String_Double_Quote = (op_String_Double_Quote == (options & op_String_Double_Quote)) ? 1 : 0;
  op_Single_Quote_As_Letter = (op_Single_Quote_As_Letter == (options & op_Single_Quote_As_Letter)) ? 1 : 0;
  op_String_Length = (op_String_Length == (options & op_String_Length)) ? 1 : 0;
}

void printOptions() {
  printf("Options: %d, %d, %d, %d, %d, %d\n", op_Underscore_As_Letter, op_Underscore_As_SpecialChar
  , op_Symbol_NEQ, op_String_Double_Quote, op_Single_Quote_As_Letter, op_String_Length);
}

void printHelp() {
  printf("\t\t--<syntax>--\n");
  printf("\t./scanner --help\t\tDisplay help\n");
  printf("\t./scanner <filename> <options>\n");
  printf("Available options:\n");
  printf("1. Allow underscore as letter\n");
  printf("2. Underscore is considered a special character\n");
  printf("4. <> and >< are considered as non-equivalent symbols\n");
  printf("8. Allow string constants sperated by double qoute\n");
  printf("16. Allow single qoute for character\n");
  printf("32. String length up to 255 characters (only compare the first 15)\n");
  printf("\tFor example: enable 2 and 4 by passing options as 6 (= 2 + 4)\n");
  printf("\t./scanner example.kpl 6\n");
}

int main(int argc, char *argv[]) {
  if (argc <= 1) {
    printf("scanner: no input file.\n");
    return -1;
  }

  if (argc == 2) {
    if (strcmp(argv[1], "--help") != 0)
      printf("Invalid options\n");
    printHelp();
    return 0;
  }

  if (argc >= 3) {
    enableOptions(atoi(argv[2]));
    printOptions();
    printf("\n");
    if (scan(argv[1]) == IO_ERROR) {
      printf("Can\'t read input file!\n");
      return -1;
    }
  }
    
  return 0;
}
