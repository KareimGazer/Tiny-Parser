#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>

// omitted definitions
#define MAXCHILDREN 3
#define LEFT_CHILD 0
#define RIGHT_CHILD 2
#define MIDDLE_CHILD 1

using namespace std;

typedef enum {
    SEMICOLON, IF, THEN, ELSE, END, REPEAT, UNTIL, IDENTIFIER, ASSIGN, READ, WRITE, LESSTHAN, EQUAL, PLUS, MINUS, MULT, DIV, OPENBRACKET, CLOSEDBRACKET, NUMBER, ERR, eof
}TokenType;

typedef struct Token {
    TokenType tType;
    string tVal;
    int lineno;
}Token;

typedef enum { START, INCOMMENT, INNUM, INID, INASSIGN, DONE, ERROR }state;

typedef enum { StmtK, ExpK } NodeKind;
typedef enum { IfK, RepeatK, AssignK, ReadK, WriteK }StmtKind;
typedef enum { OpK, ConstK, Idk } ExpKind;

/* ExpType is used for type checking*/
typedef enum { Void, Integer, Boolean } ExpType;

class TreeNode {
public:
    // struct treeNode * child[MAXCHILDREN];
    // changed interface
    struct TreeNode* left;
    struct TreeNode* right;
    struct TreeNode* middle;
    struct TreeNode* sibling;
    union { StmtKind stmt; ExpKind exp; }kind;
    NodeKind nodekind;
    ExpType type; // for type checking for exps
    TokenType attr_op;
    int lineno;
    int attr_val;
    string attr_name;
    TreeNode() {
        left = nullptr; right = nullptr; middle = nullptr;
        sibling = nullptr;
        lineno = -1;
        attr_name = "";
        attr_val = -1;

    }
};

// maps for transormations
map<char, char> symbols = { {'+', '+'}, {'-', '-'}, {'*', '*'}, {'/', '/'}, {'=', '='}, {'<', '<'}, {'(', '('}, {')', ')'}, {';', ';'} };

map<string, string> reservedWords = { {"if", "if"}, {"then", "then"}, {"else", "else"}, {"end", "end"}, {"repeat", "repeat"}, {"until", "until"}, {"read", "read"}, {"write", "write"},
};

map<char, TokenType> specialTypes = { {'+', PLUS}, {'-', MINUS}, {'*', MULT}, {'/', DIV}, {'=', EQUAL}, {'<', LESSTHAN}, {'(', OPENBRACKET}, {')', CLOSEDBRACKET}, {';', SEMICOLON},
};

map<string, TokenType> reservedTypes = { {"if", IF}, {"then", THEN}, {"else", ELSE}, {"end", END}, {"repeat", REPEAT}, {"until", UNTIL}, {"read", READ}, {"write", WRITE} };

// not nessesary used only for visualization
map<TokenType, char> spMap = { {PLUS, '+'}, {MINUS, '-'}, {MULT, '*'}, {DIV, '/'}, {ASSIGN, '='}, {LESSTHAN, '<'}, {OPENBRACKET, '('}, {CLOSEDBRACKET, ')'}, {SEMICOLON, ';'},
};

map<TokenType, string> resMap = { {IF, "if"}, {THEN, "then"}, {ELSE, "else"}, {END, "end"}, {REPEAT, "repeat"}, {UNTIL, "until"}, {READ, "read"}, {WRITE, "write"} };



// scanner global variables
int inputTextIdx = 0;
int inputTextLimit = 0;
string inputText;
bool isError = false;
int lineNum = 1;

Token token; // the globla token variable (current next token)

// scanner
Token Scanner_getToken();
bool Scanner_IsSymbol(char symbol);
bool Scanner_IsReservedWord(string word);
void Scanner_PrintToken(Token t);

// parser
void Parser_Error(void);
void Parser_Match(TokenType expectedTokenType);

TreeNode* Parser_SimpleExp(void);
TreeNode* Parser_Term(void);
TreeNode* Parser_Factor(void);
/*****************************************
 *      Karim Amin functions prototypes  *
 *****************************************/
TreeNode* Parser_Exp(void);
TreeNode* Parser_WriteStmt(void);
TreeNode* Parser_ReadStmt(void);
TreeNode* Parser_AssignStmt(void);
TreeNode* Parser_RepeatStmt(void);
TreeNode* Parser_IfStmt(void);
TreeNode* Parser_Statement(void);
TreeNode* Parser_Stmt_Sequence(void);
TreeNode* Parser_Program(void);
// printing the exp of the tree
void inOrder(TreeNode* root); // for tree traversal
void printExpNode(TreeNode* node);

int main() {
    // initialize the scanner
    string filePath = "./test1.txt";
    inputText = "";
    string textLine = "";
    ifstream inputTextFile(filePath);
    while (getline(inputTextFile, textLine)) {
        inputText += textLine + "\n";
    }
    inputTextIdx = 0; inputTextLimit = inputText.size(); isError = false;

    TreeNode* root;
    token = Scanner_getToken(); // initialize the token
    root = Parser_Program(); // build the tree
    cout << root->attr_name << root->kind.stmt << endl;
    cout << root->sibling->kind.stmt << endl;
    cout << root->sibling->middle->kind.stmt << endl;
    cout << root->sibling->middle->sibling->kind.stmt << endl;
    cout << root->sibling->middle->sibling->sibling->kind.stmt << endl;
    // print the tree
  /* inOrder(root);
   cout << endl;*/

    inputTextFile.close();
    return 0;
}

void Parser_Match(TokenType expectedTokenType) {
    // advances the token to the next char
    if (token.tType == expectedTokenType) token = Scanner_getToken();
    else Parser_Error();
}

// not yet complete until looking how to correct parser errors
void Parser_Error(void) {
    fprintf(stderr, "Error\n");
    exit(1);
}

TreeNode* Parser_SimpleExp(void) {
    TreeNode* temp = new TreeNode();

    temp = Parser_Term();
    while (token.tType == PLUS || token.tType == MINUS) {

        TreeNode* newTemp = new TreeNode();
        newTemp->lineno = token.lineno; // take the line number
        newTemp->nodekind = ExpK; // expression node
        newTemp->type = Integer; // integer expression
        newTemp->kind.exp = OpK; // kind of exp
        newTemp->attr_op = token.tType;

        Parser_Match(token.tType);
        newTemp->left = temp;
        newTemp->right = Parser_Term();
        temp = newTemp;
    }
    return temp;
}

TreeNode* Parser_Term(void) {
    TreeNode* temp = new TreeNode();

    temp = Parser_Factor();
    while (token.tType == MULT) {
        TreeNode* newTemp = new TreeNode();
        newTemp->lineno = token.lineno; // take the line number
        newTemp->nodekind = ExpK; // expression node
        newTemp->type = Integer; // integer expression
        newTemp->kind.exp = OpK; // kind of exp
        newTemp->attr_op = token.tType;

        Parser_Match(token.tType);
        newTemp->left = temp;
        newTemp->right = Parser_Factor();
        temp = newTemp;
    }
    return temp;
}

// watch for newTemp
TreeNode* Parser_Factor(void) {
    TreeNode* temp = new TreeNode();
    temp->lineno = token.lineno; // take the line number
    temp->nodekind = ExpK; // expression node
    temp->type = Integer; // integer expression

    if (token.tType == OPENBRACKET) {
        Parser_Match(OPENBRACKET);
        temp = Parser_Exp();
        Parser_Match(CLOSEDBRACKET);
    }
    else if (token.tType == NUMBER) {
        temp->attr_val = stoi(token.tVal);
        temp->kind.exp = ConstK; // kind of exp
        Parser_Match(NUMBER);
    }
    else if (token.tType == IDENTIFIER) {
        temp->attr_name = token.tVal;
        temp->kind.exp = Idk;
        Parser_Match(IDENTIFIER);
    }
    else Parser_Error();
    return temp;
}

void inOrder(TreeNode* root) {
    if (root->left != nullptr) {
        inOrder(root->left);
    }
    // cout << root->attr.val << " ";
    printExpNode(root);
    if (root->right != nullptr) inOrder(root->right);
}

void printExpNode(TreeNode* node) {
    if (node->nodekind == ExpK) {
        switch (node->kind.exp) {
        case OpK:
            cout << spMap[node->attr_op] << " ";
            // cout << node->attr.op << endl;
            // cout << "done" <<endl;
            break;
        case ConstK:
            cout << node->attr_val << " ";
            // cout << "done" <<endl;
            break;
        case Idk:
            cout << node->attr_name << " ";
            break;

        }
    }
    else if (node->nodekind == StmtK) {
        switch (node->kind.stmt) {
        case WriteK:
            cout << "write";
            break;
        }
    }
}

bool Scanner_IsSymbol(char symbol) { return symbols.find(symbol) != symbols.end(); }
bool Scanner_IsReservedWord(string word) { return reservedWords.find(word) != reservedWords.end(); }

Token Scanner_getToken() {
    string output = ""; Token currentToken;
    inputTextIdx--;
    state currentState, nextState;
    int nextIndex;

    nextState = currentState = START;
    while (currentState != DONE) {
        currentState = nextState;
        nextIndex = inputTextIdx + 1;
        switch (currentState) {
        case START:
            if (isalpha(inputText[nextIndex]) || isdigit(inputText[nextIndex]) || Scanner_IsSymbol(inputText[nextIndex]) || inputText[nextIndex] == ':') {
                output += inputText[nextIndex];
                inputTextIdx++;
            }
            else if (inputText[nextIndex] == ' ' || inputText[nextIndex] == '\t' || inputText[nextIndex] == '\n') inputTextIdx++;
            else if (inputText[nextIndex] == '{') inputTextIdx++;
            else inputTextIdx++; // error
            if (Scanner_IsSymbol(inputText[inputTextIdx])) {
                currentToken.tType = specialTypes[inputText[inputTextIdx]];
                currentToken.lineno = lineNum;
            }
            break;
        case INCOMMENT:
            inputTextIdx++;
            break;
        case INASSIGN:
            if (inputText[nextIndex] == '=') {
                output += inputText[nextIndex];
                currentToken.tType = ASSIGN;
                currentToken.lineno = lineNum;
                inputTextIdx++;
            }
            break;
        case INID:
            if (isalpha(inputText[nextIndex]) || isdigit(inputText[nextIndex])) {
                output += inputText[nextIndex];
                inputTextIdx++;
            }
            currentToken.tType = IDENTIFIER;
            currentToken.lineno = lineNum;
            break;
        case INNUM:
            if (isdigit(inputText[nextIndex])) {
                output += inputText[nextIndex];
                inputTextIdx++;
            }
            currentToken.tType = NUMBER;
            currentToken.lineno = lineNum;
            break;
        case DONE:
            inputTextIdx++;
            if (Scanner_IsReservedWord(output)) {
                currentToken.tType = reservedTypes[output];
                currentToken.lineno = lineNum;
            }
            currentToken.tVal = output;
            return currentToken;
            break;
        case ERROR:
            if (inputTextIdx == inputTextLimit && output == "") {
                currentToken.tVal = "EOF";
                currentToken.tType = eof;
                currentToken.lineno = lineNum;
                return currentToken;
            }
            isError = true;
            currentToken.tType = ERR;
            currentToken.lineno = lineNum;
            currentToken.tVal = output;
            return currentToken;
        default: // error
            break;
        }

        // transition from each state
        switch (currentState) {
        case START:
            if (inputText[nextIndex] == '{') nextState = INCOMMENT;
            else if (inputText[nextIndex] == ':') nextState = INASSIGN;
            else if (isalpha(inputText[nextIndex])) nextState = INID;
            else if (isdigit(inputText[nextIndex])) nextState = INNUM;
            else if (Scanner_IsSymbol(inputText[nextIndex])) nextState = DONE;
            else if (inputText[nextIndex] == ' ' || inputText[nextIndex] == '\t' || inputText[nextIndex] == '\n')  nextState = START;
            else nextState = ERROR;
            break;
        case INCOMMENT:
            if (inputText[nextIndex] == '}') nextState = START;
            else nextState = INCOMMENT;
            break;
        case INASSIGN:
            if (inputText[nextIndex] == '=') nextState = DONE;
            else nextState = ERROR;
            break;
        case INID:
            if (isalpha(inputText[nextIndex]) || isdigit(inputText[nextIndex])) nextState = INID;
            else nextState = DONE;
            break;
        case INNUM:
            if (isdigit(inputText[nextIndex])) nextState = INNUM;
            else nextState = DONE;
        case DONE:
            nextState = DONE; // not sure
            break;
        case ERROR:
            nextState = ERROR; // no way out
            break;
        default: // error
            break;
        }
        if (inputTextIdx < inputTextLimit && inputText[inputTextIdx] == '\n') {
            lineNum++;
        }
    }
}

void Scanner_PrintToken(Token t) {
    cout << t.lineno << ": " << t.tType << ", " << t.tVal << endl;
}
/*****************************************
 *    Karim Amin functions definitions   *
 *****************************************/
 /*
  * Describtion:  this function returns pointer to define this grammar Rule Exp ---------> SimpleExp [ ( < | = ) SimpleExp ]
  */
TreeNode* Parser_Exp(void) {
    /* Create new node */
    TreeNode* temp_ptr = new TreeNode();
    /* this will be the left child of the new node */
    /* after this line the token will point to the next token to be consumed */
    temp_ptr = Parser_SimpleExp();
    if (token.tType == LESSTHAN || token.tType == EQUAL) {
        /* create node from type experssion (operation) */
        /* this will be the new root pointer */
        TreeNode* curr_ptr = new TreeNode;
        /* store the line number */
        curr_ptr->lineno = token.lineno;
        /* set the type to be experssion */
        curr_ptr->nodekind = ExpK;
        /* the operation is integer */
        curr_ptr->type = Integer;
        curr_ptr->kind.exp = OpK;
        /* the attribute will be ( < OR = )*/
        curr_ptr->attr_op = token.tType;
        /* advance the input token */
        Parser_Match(token.tType);
        curr_ptr->left = temp_ptr;
        curr_ptr->right = Parser_SimpleExp();
        /* to return Root pointer */
        temp_ptr = curr_ptr;
    }
    /* return the root ptr */
    return temp_ptr;
}
/*
 * Describtion:  this function returns pointer to define this grammar Rule WriteStmt ---------> Write Exp
 *
 */
TreeNode* Parser_WriteStmt(void) {
    /* Create new node */
    TreeNode* temp_ptr = new TreeNode();
    TreeNode* new_root_ptr = new TreeNode();
    new_root_ptr->lineno = token.lineno;
    /* it is write statement */
    new_root_ptr->nodekind = StmtK;
    /* store the type of the statment*/
    new_root_ptr->kind.stmt = WriteK;
    /* consume the input token and check non-terminal "write" */
    Parser_Match(WRITE);
    /* make the experssion middle child to this write statment */
    new_root_ptr->middle = Parser_Exp();
    temp_ptr = new_root_ptr;
    return temp_ptr;
}
/*
 * Describtion:  this function returns pointer to define this grammar Rule ReadStmt ---------> Read identifier
 * this function does not have any children
 */
TreeNode* Parser_ReadStmt(void) {
    /* Create new node */
    TreeNode* temp_ptr = new TreeNode();
    /* check non-terminal "write" */
    if (token.tType == READ) {
        TreeNode* new_root_ptr = new TreeNode();
        new_root_ptr->lineno = token.lineno;
        /* it is Read statement */
        new_root_ptr->nodekind = StmtK;
        /* store the type of the statment */
        new_root_ptr->kind.stmt = ReadK;
        /* consume the input token */
        Parser_Match(token.tType);
        /* store the name of the identifier as an attribute in the root pointer */
        new_root_ptr->attr_name = token.tVal;
        /* advance the input to be able to get the next token */
        Parser_Match(IDENTIFIER);
        temp_ptr = new_root_ptr;
    }
    else {
        /* display error message and abort the program */
        Parser_Error();
    }
    return temp_ptr;
}
/*
 * Describtion:  this function returns pointer to define this grammar Rule Assign_Stmt --------->  identifier := exp
 * this function has only one child which is experssion and the attribute of the statement is identifier
 */
TreeNode* Parser_AssignStmt(void) {
    /* Create new node */
    TreeNode* new_root_ptr = new TreeNode();
    /* store the name of the identifier as an attribute in the assign statment node */
    new_root_ptr->attr_name = token.tVal;
    /* advance the input after storing the identifier */
    Parser_Match(IDENTIFIER);
    /* check non-terminal ":=" equal operator */
    if (token.tType == ASSIGN) {
        /* store the line number */
        new_root_ptr->lineno = token.lineno;
        /* it is assign statement */
        new_root_ptr->nodekind = StmtK;
        /* store the type of the statment*/
        new_root_ptr->kind.stmt = AssignK;
        /* consume the input token */
        Parser_Match(token.tType);
        /* make the experssion middle child to this write statment */
        new_root_ptr->middle = Parser_Exp();
    }
    else {
        /* display error message and abort the program */
        Parser_Error();
    }
    return new_root_ptr;
}
/*
 * Describtion:  this function returns pointer to define this grammar Rule repeat_stmt --------->  repeat stmt_seqence until exp
 * this function has pointer to the body of the repeat statment and anthor pointer to the condition
 */
TreeNode* Parser_RepeatStmt(void) {
    /* Create new node */
    TreeNode* new_root_ptr = new TreeNode();
    /* store the line number */
    new_root_ptr->lineno = token.lineno;
    /* it is repeat statement */
    new_root_ptr->nodekind = StmtK;
    /* store the type of the statment*/
    new_root_ptr->kind.stmt = RepeatK;
    /* consume the input token and check non-terminal "repeat" */
    Parser_Match(REPEAT);
    /* make left child points to the sequence of the statement inside the repeat */
    new_root_ptr->left = Parser_Stmt_Sequence();
    /* check the non-termenial "until" and consume the input token */
    Parser_Match(UNTIL);
    /* make the condtion middle child to this reapeat statment */
    new_root_ptr->middle = Parser_Exp();
    return new_root_ptr;
}
/*
 * Describtion:  this function returns pointer to define this grammar Rule If_Stmt ---------> if Exp then Stmt_Sequence [else Stmt_Sequence] end
 * this function has three childern condition child , body child ,and else child (optional one)
 */
TreeNode* Parser_IfStmt(void) {
    /* Create new node */
    TreeNode* new_root_ptr = new TreeNode();
    /* store the line number */
    new_root_ptr->lineno = token.lineno;
    /* it is if statement */
    new_root_ptr->nodekind = StmtK;
    /* store the type of the statment*/
    new_root_ptr->kind.stmt = IfK;
    /* check non-terminal "if" */
    Parser_Match(IF);
    new_root_ptr->left = Parser_Exp();
    /* check non-termial "then" and advance the input token */
    Parser_Match(THEN);
    /* the middle pointer points to the body of if statment*/
    new_root_ptr->middle = Parser_Stmt_Sequence();
    /* check the optional part */
    if (token.tType == ELSE) {
        /* ADVANCE THE INPUT */
        Parser_Match(ELSE);
        /* make the right child pints to the optional part */
        new_root_ptr->right = Parser_Stmt_Sequence();
    }
    /* must match the end of the if statment */
    Parser_Match(END);
    return new_root_ptr;
}
/*
 * Describtion:  this function returns pointer to define this grammar Rule Statment ---> if_stmt | repeat_stmt | assign_stmt | read_stmt | write_stmt
 * this function selects between the types of the statments and returns pointer to this statment
 */
TreeNode* Parser_Statement(void) {
    /* create pointer to statment node */
    TreeNode* ptr2node = nullptr;
    switch (token.tType) {
    case IF:
        ptr2node = Parser_IfStmt();
        break;
    case REPEAT:
        ptr2node = Parser_RepeatStmt();
        break;
    case IDENTIFIER:
        ptr2node = Parser_AssignStmt();
        break;
    case WRITE:
        ptr2node = Parser_WriteStmt();
        break;
    case READ:
        ptr2node = Parser_ReadStmt();
        break;
    default:
        ptr2node = nullptr;
        Parser_Error();
        break;
    }
    return ptr2node;
}
/*
 * Describtion:  this function returns pointer to define this grammar Rule Stmt_Sequence -------> Statment {; Statment}
 * this function creates the statment and handle the relations between them (sibling pointer)
 */
TreeNode* Parser_Stmt_Sequence(void) {
    /* create pointer to statment node */
    TreeNode* first_stmt_ptr = nullptr;
    TreeNode* next_stmt_ptr = nullptr;
    TreeNode* prev_stmt_ptr = nullptr;
    /* creat the first statment */
    first_stmt_ptr = Parser_Statement();
    prev_stmt_ptr = first_stmt_ptr;
    while (token.tType == SEMICOLON) {
        Parser_Match(SEMICOLON);
        next_stmt_ptr = Parser_Statement();
        /* connect the statment by the sibling pointer */
        prev_stmt_ptr->sibling = next_stmt_ptr;
        /* move the pointer to the last created node */
        prev_stmt_ptr = next_stmt_ptr;
    }
    return first_stmt_ptr;
}
/*
 * Describtion:  this function returns pointer to define this grammar Rule Program -------> Stmt_Sequence
 * this function creates the statment sequence for the entire program
 */
TreeNode* Parser_Program(void) {
    TreeNode* program_ptr = nullptr;
    program_ptr = Parser_Stmt_Sequence();
    return program_ptr;
}