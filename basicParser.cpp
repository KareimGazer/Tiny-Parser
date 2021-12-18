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
    int lineno;
    NodeKind nodekind;
    union { StmtKind stmt; ExpKind exp; }kind;
    union { TokenType op; int val; char* name; } attr;
    ExpType type; // for type checking for exps

    TreeNode() {
        left = nullptr; right = nullptr; middle = nullptr;
        sibling = nullptr;
        lineno = -1;
    }
};

// maps for transormations
map<char, char> symbols = { {'+', '+'}, {'-', '-'}, {'*', '*'}, {'/', '/'}, {'=', '='}, {'<', '<'}, {'(', '('}, {')', ')'}, {';', ';'} };

map<string, string> reservedWords = { {"if", "if"}, {"then", "then"}, {"else", "else"}, {"end", "end"}, {"repeat", "repeat"}, {"until", "until"}, {"read", "read"}, {"write", "write"},
};

map<char, TokenType> specialTypes = { {'+', PLUS}, {'-', MINUS}, {'*', MULT}, {'/', DIV}, {'=', ASSIGN}, {'<', LESSTHAN}, {'(', OPENBRACKET}, {')', CLOSEDBRACKET}, {';', SEMICOLON},
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
Token getToken();
bool isSymbol(char symbol);
bool isReservedWord(string word);
void printToken(Token t);

// parser
void error(void);
void match(TokenType expectedTokenType);

TreeNode* SimpleExp(void);
TreeNode* term(void);
TreeNode* factor(void);
/*****************************************
 *      Karim Amin functions prototypes  *
 *****************************************/
TreeNode* Exp(void);
TreeNode* WriteStmt(void);
TreeNode* ReadStmt(void);
TreeNode* AssignStmt(void);
TreeNode* RepeatStmt(void);
TreeNode* IfStmt(void);
TreeNode* Statement(void);
TreeNode* Stmt_Sequence(void);
TreeNode* Program(void);
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
    token = getToken(); // initialize the token
    root = SimpleExp(); // build the tree

     // print the tree
    inOrder(root);
    cout << endl;

    inputTextFile.close();
    return 0;
}

void match(TokenType expectedTokenType) {
    // advances the token to the next char
    if (token.tType == expectedTokenType) token = getToken();
    else error();
}

// not yet complete until looking how to correct parser errors
void error(void) {
    fprintf(stderr, "Error\n");
    exit(1);
}

TreeNode* SimpleExp(void) {
    TreeNode* temp = new TreeNode();

    temp = term();
    while (token.tType == PLUS || token.tType == MINUS) {

        TreeNode* newTemp = new TreeNode();
        newTemp->lineno = token.lineno; // take the line number
        newTemp->nodekind = ExpK; // expression node
        newTemp->type = Integer; // integer expression
        newTemp->kind.exp = OpK; // kind of exp
        newTemp->attr.op = token.tType;

        match(token.tType);
        newTemp->left = temp;
        newTemp->right = term();
        temp = newTemp;
    }
    return temp;
}

TreeNode* term(void) {
    TreeNode* temp = new TreeNode();

    temp = factor();
    while (token.tType == MULT) {
        TreeNode* newTemp = new TreeNode();
        newTemp->lineno = token.lineno; // take the line number
        newTemp->nodekind = ExpK; // expression node
        newTemp->type = Integer; // integer expression
        newTemp->kind.exp = OpK; // kind of exp
        newTemp->attr.op = token.tType;

        match(token.tType);
        newTemp->left = temp;
        newTemp->right = factor();
        temp = newTemp;
    }
    return temp;
}

// watch for newTemp
TreeNode* factor(void) {
    TreeNode* temp = new TreeNode();
    temp->lineno = token.lineno; // take the line number
    temp->nodekind = ExpK; // expression node
    temp->type = Integer; // integer expression

    if (token.tType == OPENBRACKET) {
        match(OPENBRACKET);
        temp = Exp();
        match(CLOSEDBRACKET);
    }
    else if (token.tType == NUMBER) {
        temp->attr.val = stoi(token.tVal);
        temp->kind.exp = ConstK; // kind of exp
        token = getToken();
    }
    else error();
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
            cout << spMap[node->attr.op] << " ";
            // cout << node->attr.op << endl;
            // cout << "done" <<endl;
            break;
        case ConstK:
            cout << node->attr.val << " ";
            // cout << "done" <<endl;
            break;
        case Idk:
            cout << node->attr.name << " ";
            break;

        }
    }
}

bool isSymbol(char symbol) { return symbols.find(symbol) != symbols.end(); }
bool isReservedWord(string word) { return reservedWords.find(word) != reservedWords.end(); }

Token getToken() {
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
            if (isalpha(inputText[nextIndex]) || isdigit(inputText[nextIndex]) || isSymbol(inputText[nextIndex]) || inputText[nextIndex] == ':') {
                output += inputText[nextIndex];
                inputTextIdx++;
            }
            else if (inputText[nextIndex] == ' ' || inputText[nextIndex] == '\t' || inputText[nextIndex] == '\n') inputTextIdx++;
            else if (inputText[nextIndex] == '{') inputTextIdx++;
            else inputTextIdx++; // error
            if (isSymbol(inputText[inputTextIdx])) {
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
            if (isReservedWord(output)) {
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
            else if (isSymbol(inputText[nextIndex])) nextState = DONE;
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

void printToken(Token t) {
    cout << t.lineno << ": " << t.tType << ", " << t.tVal << endl;
}
/*****************************************
 *    Karim Amin functions definitions   *
 *****************************************/
T/*
 * Describtion:  this function returns pointer to defien this grammar Rule Exp ---------> SimpleExp [ ( < | = ) SimpleExp ]
 */
    TreeNode* Exp(void) {
    /* Create new node */
    TreeNode* temp_ptr = new TreeNode();
    /* this will be the left child of the new node */
    /* after this line the token will point to the next token to be consumed */
    temp_ptr = SimpleExp();
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
        curr_ptr->attr.op = token.tType;
        /* advance the input token */
        match(token.tType);
        curr_ptr->left = temp_ptr;
        curr_ptr->right = SimpleExp();
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
TreeNode* WriteStmt(void) {
    /* Create new node */
    TreeNode* temp_ptr = new TreeNode();
    /* check non-terminal "write" */
    if (token.tType == WRITE) {
        TreeNode* new_root_ptr = new TreeNode();
        new_root_ptr->lineno = token.lineno;
        /* it is write statement */
        new_root_ptr->nodekind = StmtK;
        /* store the type of the statment*/
        new_root_ptr->kind.stmt = WriteK;
        /* consume the input token */
        match(token.tType);
        /* make the experssion middle child to this write statment */
        new_root_ptr->middle = Exp();
        temp_ptr = new_root_ptr;
    }
    else {
        /* display error message and abort the program */
        error();
    }
    return temp_ptr;
}
/*
 * Describtion:  this function returns pointer to define this grammar Rule ReadStmt ---------> Read identifier
 * this function does not have any children
 */
TreeNode* ReadStmt(void) {
    /* Create new node */
    TreeNode* temp_ptr = new TreeNode();
    /* check non-terminal "write" */
    if (token.tType == READ) {
        TreeNode* new_root_ptr = new TreeNode();
        new_root_ptr->lineno = token.lineno;
        /* it is Read statement */
        new_root_ptr->nodekind = StmtK;
        /* store the type of the statment*/
        new_root_ptr->kind.stmt = ReadK;
        /* consume the input token */
        match(token.tType);
        if (token.tType == IDENTIFIER) {
            /* store the name of the identifier as an attribute in the root pointer*/
            new_root_ptr->attr.name = &token.tVal[0];
        }
        else {
            /* display error message and abort the program */
            error();
        }
        temp_ptr = new_root_ptr;
    }
    else {
        /* display error message and abort the program */
        error();
    }
    /* return the root pointer */
    return temp_ptr;
}