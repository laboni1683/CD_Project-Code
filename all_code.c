#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

// =================================================================
// PART 1: LEXER (DFA-BASED) - CORRECTED
// =================================================================

#define MAX_TOKEN_LEN 100
#define MAX_TOKENS 500
#define MAXSYM 50

// Token Definitions (Matching Parser's Terminals)
typedef enum {
    TOKEN_EOF = 0,
    TOKEN_INCLUDE, // 1: #include<stdio.h>
    TOKEN_KW_INT, // 2: int
    TOKEN_KW_DEC, // 3: dec
    TOKEN_VAR_NAME, // 4: _alpha...digit_alpha
    TOKEN_FUNC_NAME, // 5: alpha...Fn
    TOKEN_MAIN_FUNC, // 6: main
    TOKEN_LOOP_KW, // 7: loop
    TOKEN_WHILE, // 8: while
    TOKEN_OPEN_PAREN, // 9: (
    TOKEN_CLOSE_PAREN, // 10: )
    TOKEN_OPEN_BRACE, // 11: {
    TOKEN_CLOSE_BRACE, // 12: }
    TOKEN_DOTDOT, // 13: .. (End of statement)
    TOKEN_ASSIGN, // 14: =
    TOKEN_COMPARATOR, // 15: <
    TOKEN_NUMBER, // 16: 10, 5, 2 etc.
    TOKEN_PLUS, // 17: +
    TOKEN_RETURN, // 18: return
    TOKEN_PRINTF, // 19: printf
    TOKEN_BREAK, // 20: break
    TOKEN_COMMA, // 21: ,
    TOKEN_COLON, // 22: : (For loop termination)
    TOKEN_ERROR // 23: Unrecognized token
} TokenType;

// Global structure to hold tokens
typedef struct {
    TokenType type;
    char lexeme[MAX_TOKEN_LEN];
} Token;

Token tokens[MAX_TOKENS];
int token_count = 0;

// DFA States
enum {
    S0, // Start
    S_INC, // Start state for #include
    S_DATATYPE_I, S_DATATYPE_IN, S_DATATYPE_INT,
    S_DATATYPE_D, S_DATATYPE_DE, S_DATATYPE_DEC,
    S_VAR_START, S_VAR_ALPHA, S_VAR_DIGIT, S_VAR_END,
    S_FUNC_ALPHA, S_FUNC_F, S_FUNC_FN, // Simplified FuncName
    S_MAIN_M, S_MAIN_MA, S_MAIN_MAIN,
    S_LOOP_L, S_LOOP_LO, S_LOOP_LOOP, S_LOOP_UNDERSCORE, S_LOOP_ALPHA, S_LOOP_DIGIT1, S_LOOP_DIGIT2, S_LOOP_COLON,
    S_W_W, S_W_WH, S_W_WHI, S_W_WHIL, S_W_WHILE,
    S_R_R, S_R_RE, S_R_RET, S_R_RETU, S_R_RETUR, S_R_RETURN,
    S_B_B, S_B_BR, S_B_BRE, S_B_BREA, S_B_BREAK,
    S_P_P, S_P_PR, S_P_PRI, S_P_PRIN, S_P_PRINT, S_P_PRINTF,
    S_NUM_DIGIT,
    S_DOT1, S_DOTDOT,
    S_COMPARATOR, S_PLUS, S_OPEN_PAREN, S_CLOSE_PAREN, S_OPEN_BRACE, S_CLOSE_BRACE, S_ASSIGN, S_COMMA, S_COLON_ONLY,
    DEAD // Dead state
};

#define NUM_STATES 58
#define NUM_INPUTS 16

// Input Mapping for the DFA (FIXED LOGIC)
int get_input(char c) {
    if (c == '#') return 0;
    // Grouping specific letters needed for multi-character tokens (i.e., keywords)
    if (strchr("intdecmaloprbfwuk", c) != NULL) return 1;
    if (islower(c)) return 2; // Remaining lowercase alphabet
    if (c == '_') return 3;
    if (isdigit(c)) return 4;
    if (c == '(') return 5;
    if (c == ')') return 6;
    if (c == '{') return 7;
    if (c == '}') return 8;
    if (c == '=') return 9;
    if (c == '<') return 10;
    if (c == '+') return 11;
    if (c == '.') return 12;
    if (c == ',') return 13;
    if (c == ':') return 14;
    return 15;
}

// DFA transition table: next_state[current_state][input_symbol] - CORRECTED
int next_state[NUM_STATES][NUM_INPUTS] = {
// 0:# 1:spec_alpha 2:other_alpha 3:_ 4:0-9 5:( 6:) 7:{ 8:} 9:= 10:< 11:+ 12:. 13:, 14:: 15:Other
/* S0 */ {S_INC, S_DATATYPE_I, S_FUNC_ALPHA, S_VAR_START, S_NUM_DIGIT, S_OPEN_PAREN, S_CLOSE_PAREN, S_OPEN_BRACE, S_CLOSE_BRACE, S_ASSIGN, S_COMPARATOR, S_PLUS, S_DOT1, S_COMMA, S_COLON_ONLY, DEAD},
/* S_INC */ {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD}, // #include<stdio.h> (TERMINAL - Handled in lex())

// int
/* S_DATATYPE_I */ {DEAD, S_DATATYPE_IN, S_FUNC_ALPHA, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
/* S_DATATYPE_IN */ {DEAD, S_DATATYPE_INT, S_FUNC_ALPHA, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
/* S_DATATYPE_INT */ {DEAD, DEAD, S_FUNC_ALPHA, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD}, // ACCEPT int
// dec
/* S_DATATYPE_D */ {DEAD, S_DATATYPE_DE, S_FUNC_ALPHA, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
/* S_DATATYPE_DE */ {DEAD, S_DATATYPE_DEC, S_FUNC_ALPHA, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
/* S_DATATYPE_DEC */ {DEAD, DEAD, S_FUNC_ALPHA, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD}, // ACCEPT dec

// VAR_NAME
/* S_VAR_START */ {DEAD, S_VAR_ALPHA, S_VAR_ALPHA, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
/* S_VAR_ALPHA */ {DEAD, S_VAR_ALPHA, S_VAR_ALPHA, DEAD, S_VAR_DIGIT, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
/* S_VAR_DIGIT */ {DEAD, S_VAR_END, S_VAR_END, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD}, // FIXED: digits can be followed by alpha/spec_alpha
/* S_VAR_END */ {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD}, // ACCEPT _var...

// FUNC_NAME (alpha...Fn) - Added loop to allow multiple characters
/* S_FUNC_ALPHA */ {DEAD, S_FUNC_ALPHA, S_FUNC_ALPHA, DEAD, S_FUNC_ALPHA, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
/* S_FUNC_F */ {DEAD, S_FUNC_FN, S_FUNC_ALPHA, DEAD, S_FUNC_ALPHA, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
/* S_FUNC_FN */ {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD}, // ACCEPT funcname

// main
/* S_MAIN_M */ {DEAD, S_MAIN_MA, S_FUNC_ALPHA, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
/* S_MAIN_MA */ {DEAD, S_MAIN_MAIN, S_FUNC_ALPHA, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
/* S_MAIN_MAIN */ {DEAD, DEAD, S_FUNC_ALPHA, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD}, // ACCEPT main (allows mainFn, main_loop, etc.)

// LOOP_KW
/* S_LOOP_L */ {DEAD, S_LOOP_LO, S_FUNC_ALPHA, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
/* S_LOOP_LO */ {DEAD, S_LOOP_LOOP, S_FUNC_ALPHA, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
/* S_LOOP_LOOP */ {DEAD, DEAD, S_FUNC_ALPHA, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD}, // ACCEPT loop (Allows loop_var)
/* S_LOOP_UNDERSCORE */ {DEAD, S_LOOP_ALPHA, S_LOOP_ALPHA, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
/* S_LOOP_ALPHA */ {DEAD, S_LOOP_ALPHA, S_LOOP_ALPHA, DEAD, S_LOOP_DIGIT1, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
/* S_LOOP_DIGIT1 */ {DEAD, DEAD, DEAD, DEAD, S_LOOP_DIGIT2, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
/* S_LOOP_DIGIT2 */ {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, S_LOOP_COLON, DEAD},
/* S_LOOP_COLON */ {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD}, // ACCEPT loop_...:

// WHILE
/* S_W_W */ {DEAD, S_W_WH, S_FUNC_ALPHA, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
/* S_W_WH */ {DEAD, S_W_WHI, S_FUNC_ALPHA, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
/* S_W_WHI */ {DEAD, S_W_WHIL, S_FUNC_ALPHA, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
/* S_W_WHIL */ {DEAD, S_W_WHILE, S_FUNC_ALPHA, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
/* S_W_WHILE */ {DEAD, DEAD, S_FUNC_ALPHA, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD}, // ACCEPT while

// RETURN
/* S_R_R */ {DEAD, S_R_RE, S_FUNC_ALPHA, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
/* S_R_RE */ {DEAD, S_R_RET, S_FUNC_ALPHA, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
/* S_R_RET */ {DEAD, S_R_RETU, S_FUNC_ALPHA, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
/* S_R_RETU */ {DEAD, S_R_RETUR, S_FUNC_ALPHA, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
/* S_R_RETUR */ {DEAD, S_R_RETURN, S_FUNC_ALPHA, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
/* S_R_RETURN */ {DEAD, DEAD, S_FUNC_ALPHA, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD}, // ACCEPT return

// BREAK
/* S_B_B */ {DEAD, S_B_BR, S_FUNC_ALPHA, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
/* S_B_BR */ {DEAD, S_B_BRE, S_FUNC_ALPHA, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
/* S_B_BRE */ {DEAD, S_B_BREA, S_FUNC_ALPHA, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
/* S_B_BREA */ {DEAD, S_B_BREAK, S_FUNC_ALPHA, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
/* S_B_BREAK */ {DEAD, DEAD, S_FUNC_ALPHA, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD}, // ACCEPT break

// PRINTF
/* S_P_P */ {DEAD, S_P_PR, S_FUNC_ALPHA, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
/* S_P_PR */ {DEAD, S_P_PRI, S_FUNC_ALPHA, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
/* S_P_PRI */ {DEAD, S_P_PRIN, S_FUNC_ALPHA, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
/* S_P_PRIN */ {DEAD, S_P_PRINT, S_FUNC_ALPHA, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
/* S_P_PRINT */ {DEAD, S_P_PRINTF, S_FUNC_ALPHA, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
/* S_P_PRINTF */ {DEAD, DEAD, S_FUNC_ALPHA, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD}, // ACCEPT printf

/* S_NUM_DIGIT */ {DEAD, DEAD, DEAD, DEAD, S_NUM_DIGIT, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD}, // ACCEPT 123
/* S_DOT1 */ {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, S_DOTDOT, DEAD, DEAD, DEAD},
/* S_DOTDOT */ {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD}, // ACCEPT ..

// Single Char Tokens
/* S_COMPARATOR */ {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
/* S_PLUS */ {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
/* S_OPEN_PAREN */ {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
/* S_CLOSE_PAREN */ {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
/* S_OPEN_BRACE */ {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
/* S_CLOSE_BRACE */ {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
/* S_ASSIGN */ {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
/* S_COMMA */ {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
/* S_COLON_ONLY */ {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
/* DEAD */ {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD}
};

// Map states to token types (Size 58)
TokenType state_to_token_type[NUM_STATES] = {
    TOKEN_ERROR, TOKEN_INCLUDE, // S_INC is the state after reading #
    TOKEN_ERROR, TOKEN_ERROR, TOKEN_KW_INT,
    TOKEN_ERROR, TOKEN_ERROR, TOKEN_KW_DEC,
    TOKEN_ERROR, TOKEN_ERROR, TOKEN_ERROR, TOKEN_VAR_NAME, // _var...
    TOKEN_FUNC_NAME, TOKEN_ERROR, TOKEN_FUNC_NAME, // funcname (S_FUNC_FN)
    TOKEN_ERROR, TOKEN_ERROR, TOKEN_MAIN_FUNC, // main (S_MAIN_MAIN)
    TOKEN_ERROR, TOKEN_ERROR, TOKEN_LOOP_KW, // loop (S_LOOP_LOOP)
    TOKEN_ERROR, TOKEN_ERROR, TOKEN_ERROR, TOKEN_ERROR, TOKEN_COLON, // loop_...: (S_LOOP_COLON)
    TOKEN_ERROR, TOKEN_ERROR, TOKEN_ERROR, TOKEN_ERROR, TOKEN_WHILE,
    TOKEN_ERROR, TOKEN_ERROR, TOKEN_ERROR, TOKEN_ERROR, TOKEN_ERROR, TOKEN_RETURN,
    TOKEN_ERROR, TOKEN_ERROR, TOKEN_ERROR, TOKEN_ERROR, TOKEN_BREAK,
    TOKEN_ERROR, TOKEN_ERROR, TOKEN_ERROR, TOKEN_ERROR, TOKEN_ERROR, TOKEN_PRINTF,
    TOKEN_NUMBER,
    TOKEN_ERROR, TOKEN_DOTDOT, // ..
    TOKEN_COMPARATOR, TOKEN_PLUS,
    TOKEN_OPEN_PAREN, TOKEN_CLOSE_PAREN, TOKEN_OPEN_BRACE, TOKEN_CLOSE_BRACE, TOKEN_ASSIGN,
    TOKEN_COMMA, TOKEN_COLON, // S_COLON_ONLY now maps to TOKEN_COLON
    TOKEN_ERROR // DEAD state
};

void lex(const char *program_text) {
    token_count = 0;
    int i = 0;

    // Special case for #include<stdio.h> - Lookahead logic correction
    while (isspace(program_text[i])) i++;
    if (strncmp(program_text + i, "#include<stdio.h>", 17) == 0) {
        if (token_count < MAX_TOKENS) {
            strcpy(tokens[token_count].lexeme, "#include<stdio.h>");
            tokens[token_count].type = TOKEN_INCLUDE;
            token_count++;
            i += 17;
        }
    } else {
        // If the code doesn't start with the required line, it will be rejected later
    }

    while (program_text[i] != '\0') {
        while (isspace(program_text[i])) {
            i++;
        }
        if (program_text[i] == '\0') break;

        int start = i;
        int state = S0;
        int last_accepting_pos = -1;
        int last_accepting_state = DEAD;

        int j = start;

        // DFA simulation
        while (program_text[j] != '\0') {
            int input = get_input(program_text[j]);
            int next = next_state[state][input];

            if (next == DEAD) {
                break;
            }

            // Custom handling for FuncName: if it's 'F' and the previous state was S_FUNC_ALPHA
            if (state == S_FUNC_ALPHA && program_text[j] == 'F') {
                next = S_FUNC_F; // Manually transition to S_FUNC_F for "Fn" check
            } else if (state == S_FUNC_F && program_text[j] == 'n') {
                next = S_FUNC_FN; // Final check for "Fn"
            }

            state = next;
            j++;

            // If the state is accepting, record it
            if (state_to_token_type[state] != TOKEN_ERROR) {
                last_accepting_pos = j;
                last_accepting_state = state;

                // Stop immediately for single-char and fixed multi-char tokens
                if (state_to_token_type[state] != TOKEN_VAR_NAME && state_to_token_type[state] != TOKEN_FUNC_NAME && state_to_token_type[state] != TOKEN_NUMBER) {
                    break;
                }
            }
        }

        if (last_accepting_pos != -1) {
            TokenType accepted_type = state_to_token_type[last_accepting_state];
            int len = last_accepting_pos - start;

            if (len >= MAX_TOKEN_LEN) len = MAX_TOKEN_LEN - 1;

            strncpy(tokens[token_count].lexeme, program_text + start, len);
            tokens[token_count].lexeme[len] = '\0';

            tokens[token_count].type = accepted_type;
            token_count++;
            i = last_accepting_pos;
        } else {
            // Error handling: unrecognized token
            fprintf(stderr, "Lexer Error: Unrecognized token starting at index %d: '%c'\n", i, program_text[i]);
            i++; // Move past the character to continue scanning
        }
    }

    // Add EOF
    tokens[token_count].type = TOKEN_EOF;
    strcpy(tokens[token_count].lexeme, "$");
    token_count++;
}

// =================================================================
// PART 2: PARSER (LL(1)-BASED) - CORRECTED
// =================================================================

#define NNT 16   // Number of Non-Terminals (0 to 15)
#define NTER 24  // Number of Terminals (0 to 23) (23 is ERROR)
#define MAXSTACK 500 // For the stack implementation

// Non-terminals (NT)
char *NT[] = {"Program","OptFuncs","FuncDef","FuncParams","DataType","StatementList","Statement","Assignment","Loop","MainFunc","FuncCall","PrintfCall","ReturnStmt","Expression","Term","Expr_Tail"};
// Terminals (T) - Must match the order of the TABLE
char *TERMINALS[] = {
    "#include<stdio.h>", "int", "dec", "_VAR_NAME", "FUNC_NAME", "main", "loop", "while", "(", ")", "{", "}", "..", "=", "<", "NUMBER", "+", "return", "printf", "break", ",", ":", "$", "ERROR"
};

// LL(1) Table Helper: Maps TokenType to TERMINALS index (FIXED)
int token_type_to_ter_index(TokenType type, const char* lexeme) {
    if (type == TOKEN_INCLUDE) return 0;
    if (type == TOKEN_KW_INT) return 1;
    if (type == TOKEN_KW_DEC) return 2;
    if (type == TOKEN_VAR_NAME) return 3;
    if (type == TOKEN_FUNC_NAME) return 4;
    if (type == TOKEN_MAIN_FUNC) return 5;
    if (type == TOKEN_LOOP_KW) return 6;
    if (type == TOKEN_WHILE) return 7;
    if (type == TOKEN_OPEN_PAREN) return 8;
    if (type == TOKEN_CLOSE_PAREN) return 9;
    if (type == TOKEN_OPEN_BRACE) return 10;
    if (type == TOKEN_CLOSE_BRACE) return 11;
    if (type == TOKEN_DOTDOT) return 12;
    if (type == TOKEN_ASSIGN) return 13;
    if (type == TOKEN_COMPARATOR) return 14;
    if (type == TOKEN_NUMBER) return 15;
    if (type == TOKEN_PLUS) return 16;
    if (type == TOKEN_RETURN) return 17;
    if (type == TOKEN_PRINTF) return 18;
    if (type == TOKEN_BREAK) return 19;
    if (type == TOKEN_COMMA) return 20;
    if (type == TOKEN_COLON) return 21;
    if (type == TOKEN_EOF) return 22;
    return 23; // ERROR
}

// Production Rules (RHS)
char *RHS[] = {
    "#include<stdio.h> OptFuncs MainFunc", // 1: Program
    "FuncDef OptFuncs", // 2: OptFuncs -> FuncDef OptFuncs
    "", // 3: OptFuncs -> epsilon (Follow: DataType of MainFunc)
    "DataType FUNC_NAME ( FuncParams ) { StatementList ReturnStmt }", // 4: FuncDef
    "DataType _VAR_NAME OptParams", // 5: FuncParams
    "", // 6: OptParams -> epsilon (Follow: ')')
    "int", // 7: DataType -> int
    "dec", // 8: DataType -> dec
    "Statement StatementList", // 9: StatementList -> Statement StatementList
    "", // 10: StatementList -> epsilon (Follow: return, '}')
    "Assignment ..", // 11: Statement -> Assignment .. (Assignment includes declarations)
    "Loop", // 12: Statement -> Loop
    "PrintfCall ..", // 13: Statement -> PrintfCall ..
    "break ..", // 14: Statement -> break ..
    "DataType _VAR_NAME = Expression", // 15: Assignment (Declaration and initialization)
    "DataType _VAR_NAME", // 16: Assignment (Declaration only)
    "_VAR_NAME = Expression", // 17: Assignment (Re-assignment)
    "loop _VAR_NAME : while ( _VAR_NAME < NUMBER ) { StatementList break .. }", // 18: Loop
    "printf ( _VAR_NAME )", // 19: PrintfCall
    "return Expression ..", // 20: ReturnStmt
    "Term Expr_Tail", // 21: Expression -> Term Expr_Tail (LL(1) for E -> T E')
    "NUMBER", // 22: Term -> NUMBER
    "_VAR_NAME", // 23: Term -> _VAR_NAME
    "DataType main ( ) { StatementList ReturnStmt }", // 24: MainFunc
    "+ Term Expr_Tail", // 25: Expr_Tail -> + Term Expr_Tail (E' -> + T E')
    "" // 26: Expr_Tail -> epsilon (E' -> epsilon)
};

// LL(1) Table (CORRECTED with Expression Fix)
int TABLE[NNT][NTER] = {
// 0:# 1:int 2:dec 3:_VAR 4:FNAME 5:main 6:loop 7:while 8:( 9:) 10:{ 11:} 12:.. 13:= 14:< 15:NUM 16:+ 17:ret 18:printf 19:break 20:, 21:: 22:$ 23:ERR
/* 0:Program */      {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
/* 1:OptFuncs */     {0, 2, 2, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
/* 2:FuncDef */      {0, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
/* 3:FuncParams */   {0, 5, 5, 0, 0, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
/* 4:DataType */     {0, 7, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
/* 5:StatementList */{0, 9, 9, 9, 0, 0, 9, 0, 0, 0, 0, 10, 0, 0, 0, 0, 0, 10, 9, 9, 0, 0, 0, 0}, // Follow(StatementList) is RETURN or }
/* 6:Statement */    {0, 11, 11, 17, 0, 0, 12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 13, 14, 0, 0, 0, 0}, // R11 for decl/init, R17 for reassign
/* 7:Assignment */   {0, 15, 15, 17, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
/* 8:Loop */         {0, 0, 0, 0, 0, 0, 18, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
/* 9:MainFunc */     {0, 24, 24, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
/* 10:FuncCall */    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
/* 11:PrintfCall */  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 19, 0, 0, 0, 0, 0},
/* 12:ReturnStmt */  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 20, 0, 0, 0, 0, 0, 0},
/* 13:Expression */  {0, 0, 0, 21, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 21, 0, 0, 0, 0, 0, 0, 0, 0}, // R21: E -> T E'
/* 14:Term */        {0, 0, 0, 23, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 22, 0, 0, 0, 0, 0, 0, 0, 0},
/* 15:Expr_Tail */   {0, 0, 0, 0, 0, 0, 0, 0, 0, 26, 0, 26, 26, 0, 0, 0, 25, 26, 0, 0, 26, 0, 0, 0} // E' -> + T E' | epsilon (Follows: ), }, .., RETURN, ,)
};


// --- Stack implementation (from LL(1).c) ---
char stack[MAXSTACK][MAXSYM];
int top = -1;

void push(char *s){ strcpy(stack[++top], s); }
char* pop(){ return (top>=0)?stack[top--]:NULL; }
void print_stack(){
    printf("[");
    for(int i=top;i>=0;i--){
        printf("%s",stack[i]);
        if(i>0) printf(", ");
    }
    printf("]");
}

int find_nt(char *x){ for(int i=0;i<NNT;i++) if(strcmp(NT[i],x)==0) return i; return -1; }
int find_t(char *x){ for(int i=0;i<NTER;i++) if(strcmp(TERMINALS[i],x)==0) return i; return -1; }

// Helper to split RHS into symbols
void split_rhs(char *rhs, char symbols[][MAXSYM], int *k) {
    *k = 0;
    char temp[200];
    strcpy(temp, rhs);
    char *p = strtok(temp," ");
    while(p){
        strcpy(symbols[(*k)++],p);
        p=strtok(NULL," ");
    }
}
// --- End Stack implementation ---

void parse() {
    printf("\n--- PARSER EXECUTION ---\n");
    push("$");
    push(NT[0]); // Start symbol: Program

    int ip = 0;
    printf("%-25s %-15s %-10s %-25s\n","Stack","Lookahead (Token)","Top","Production Applied");
    printf("----------------------------------------------------------------------------------\n");

    while (top >= 0 && ip < token_count) {
        char X[MAXSYM];
        if (top < 0) break;
        strcpy(X, pop());

        Token *a = &tokens[ip];
        int ter_idx = token_type_to_ter_index(a->type, a->lexeme);
        char *lookahead_name = (ter_idx >= 0 && ter_idx < NTER) ? TERMINALS[ter_idx] : "ERROR";

        // Custom mapping for generic terminals (VAR_NAME, FUNC_NAME, etc.)
        if (a->type == TOKEN_VAR_NAME) lookahead_name = TERMINALS[3];
        else if (a->type == TOKEN_FUNC_NAME) lookahead_name = TERMINALS[4];
        else if (a->type == TOKEN_MAIN_FUNC) lookahead_name = TERMINALS[5];
        else if (a->type == TOKEN_LOOP_KW) lookahead_name = TERMINALS[6];
        else if (a->type == TOKEN_KW_INT) lookahead_name = TERMINALS[1];
        else if (a->type == TOKEN_KW_DEC) lookahead_name = TERMINALS[2];
        else if (a->type == TOKEN_INCLUDE) lookahead_name = TERMINALS[0];
        else if (a->type == TOKEN_NUMBER) lookahead_name = TERMINALS[15];
        else if (a->type == TOKEN_RETURN) lookahead_name = TERMINALS[17];
        else if (a->type == TOKEN_PRINTF) lookahead_name = TERMINALS[18];
        else if (a->type == TOKEN_BREAK) lookahead_name = TERMINALS[19];


        // Print stack state
        if(find_nt(X) != -1) push(X);
        print_stack();
        if(find_nt(X) != -1) strcpy(X, pop());

        printf("%-25s %-15s %-10s ", "" , lookahead_name, X);

        // 1. Check if X is a terminal
        if (find_t(X) != -1) {
            if (ter_idx != 23 && strcmp(X, lookahead_name) == 0) {
                printf("%-25s\n", "match");
                ip++;
            }
            else {
                printf("REJECTED\n");
                fprintf(stderr, "PARSER ERROR: Expected terminal '%s', found '%s' (%s) at token index %d\n", X, a->lexeme, lookahead_name, ip);
                return;
            }
        }
        // 2. Check if X is a non-terminal
        else {
            int nt_idx = find_nt(X);
            if (nt_idx == -1 || ter_idx == 23) {
                printf("REJECTED\n");
                fprintf(stderr, "PARSER ERROR: Invalid symbol on stack or invalid lookahead\n");
                return;
            }

            int prod = TABLE[nt_idx][ter_idx];
            if (prod == 0) {
                printf("REJECTED\nNo rule for (%s, %s)\n", X, lookahead_name);
                return;
            }

            char *rhs = RHS[prod - 1];

            if (strlen(rhs) == 0) {
                printf("%-25s\n", "epsilon");
            } else {
                printf("%s -> %-25s\n", X, rhs);
            }

            // Push the production rule to stack (in reverse order)
            if (strlen(rhs) > 0) {
                char symbols[10][MAXSYM];
                int k;
                split_rhs(rhs, symbols, &k);
                for(int i=k-1;i>=0;i--) {
                    push(symbols[i]);
                }
            }
        }
    }

    if (top == 0 && strcmp(stack[0], "$") == 0 && ip == token_count - 1 && tokens[ip].type == TOKEN_EOF) {
        print_stack();
        pop();
        printf("%-25s %-15s %-10s %-25s\n", "" , "$", "$", "match");
        printf("\nSYNTAX ACCEPTED (Parser Structure Valid)\n");
    } else {
        printf("\nSYNTAX REJECTED: Input not fully consumed or Stack not empty\n");
    }
}

// =================================================================
// MAIN FUNCTION
// =================================================================

int main() {
    char filename[100];

    printf("Enter input file name (e.g., input_src): ");
    if (fgets(filename, sizeof(filename), stdin) == NULL) {
        fprintf(stderr, "Error reading filename.\n");
        return 1;
    }
    filename[strcspn(filename, "\n")] = '\0';

    FILE *fp = fopen(filename, "r");
    if (!fp) {
        printf("Error: Cannot open %s. Make sure the file is in the project folder (or bin/Debug).\n", filename);
        return 1;
    }

    char program_text[5000] = {0};
    size_t bytes_read = fread(program_text, 1, sizeof(program_text) - 1, fp);
    program_text[bytes_read] = '\0';
    fclose(fp);

    printf("\n--- INPUT PROGRAM ---\n%s\n", program_text);

    // 1. Lexical Analysis
    lex(program_text);

    printf("\n--- LEXER OUTPUT (Tokens) ---\n");
    for (int i = 0; i < token_count; i++) {
        printf("[%d: %s] ", tokens[i].type, tokens[i].lexeme);
    }
    printf("\n");

    // 2. Syntactic Analysis
    parse();

    return 0;
}
