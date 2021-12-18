/* Simple integer arithmetic syntax tree
 * according to the EBNF:
 
 exp -> term {addop term}
 addop -> + | -
 term -> factor {mulop factor}
 mulop -> *
 factor -> (exp) | number
 Inputs a line of text from stdin
 Outputs "Error" or the result
*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <iostream>

using namespace std;

char token; // global token variable

class Node{
public:
    Node * parent;
    Node * left;
    Node * right;
    char data;
    Node(){
      // parent = nullptr; 
      left = nullptr; right = nullptr;
      data = 'r';
    }
    Node(Node * p, Node * l, Node * r, char k)
    {
        parent = p;
        left = l;
        right = r;
        data = k;
    }
};

void error(void);
void match(char expectedToken);

Node* Exp(void);
Node* term(void);
Node* factor(void); 

void inOrder(Node * root);

int main(void) {
  Node * root;
  cout << "write simple exp: " << endl;
  token = getchar();
  root = Exp();
  if(token == '\n') printf("Tree constructed\n");
  else error();
  inOrder(root);
  cout << endl;
  return 0;
}

void error(void){
  fprintf(stderr, "Error\n");
  exit(1);
}

void match(char expectedToken){
  // advances the token to the next char
  if(token==expectedToken) token = getchar();
  else error();
}

Node* Exp(void){
  Node *temp, *newTemp;
  temp = term();
  while(token == '+' || token == '-'){
    newTemp = new Node(nullptr, nullptr, nullptr, token);
    match(token);
    newTemp->left = temp;
    newTemp->right = term();
    temp = newTemp;
  }
  return temp;
}

Node* term(void){
  Node *temp, *newTemp;
  temp = factor();
  while(token=='*'){
    newTemp = new Node(nullptr, nullptr, nullptr, token);
    match(token); // should always be *
    newTemp->left = temp;
    newTemp->right = factor();
    temp = newTemp;
  }
  return temp;
}

// watch for newTemp
Node* factor(void){
  Node *temp, *newTemp;
  temp = new Node();
  newTemp = new Node();

  if(token =='('){
    match('(');
    temp = Exp();
    match(')');
  }
  else if(isdigit(token)){
    temp->data = token;
    token = getchar();
  }
  else error();
  return temp;
}

void inOrder(Node * root){
  if(root->left != nullptr) {
    inOrder(root->left);
  }
  cout << root->data << " ";
  if(root->right != nullptr) inOrder(root->right);
}