#include "Huffman.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <pthread.h>
#include <sstream>
#include <string>
#include <vector>

struct threadinfo {
  std::vector<int> positions; //list of positions for chars to be inserted in
  std::string *message; //used to insert the chars in
  node *treenode; //root of Huffman tree
  std::string trav; //binary code
  int threadnumber; //index
  int nthreads; //number of threads
  int *turn; //shared resource
  pthread_mutex_t *semB; //mutex
  pthread_cond_t *waitTurn; //conditional variable
};

node *traverse(node *tree, std::string tra, int index) { //traverse through tree to retrieve node for print

  if (tree == nullptr) {
    return nullptr;
  }

  if (tree->right == nullptr && tree->left == nullptr) {
    return tree;
  } else if (tra[index] == '0') {
    return traverse(tree->left, tra, index + 1);

  } else if (tra[index] == '1') {
    return traverse(tree->right, tra, index + 1);
  }
  return 0;
}

//----------------------------------------------------------------

void *decode(void *void_ptr) { //thread function

  threadinfo *arg = (threadinfo *)void_ptr;

  std::string binary = arg->trav;
  int threadnum = arg->threadnumber;
  std::vector<int> posit = arg->positions;
 
  
  pthread_mutex_unlock(arg->semB);

  node *info = traverse(arg->treenode, binary, 0); // node used to print and insert

  pthread_mutex_lock(arg->semB);  // second critical section

  while (threadnum != *(arg->turn)) {
    pthread_cond_wait(arg->waitTurn, arg->semB);
  }
  
  pthread_mutex_unlock(arg->semB);

  //-------------------------------------------------------------------------
  for (int i = 0; i < posit.size(); i++) {

    int pos = posit.at(i); // the position

    (*arg->message)[pos] = info->c; // insert char into position of string
  }

  std::cout << "Symbol: " << info->c << ", Frequency: " << info->freq
            << ", Code: " << binary << std::endl;

  //---------------------------------------------------------------------

  pthread_mutex_lock(arg->semB); //third critical section
  
  if((*(arg->turn)) >= arg->nthreads){ //update shared resource
      (*(arg->turn)) = 1;
  }else{
      (*(arg->turn)) = (*(arg->turn)) + 1;
  }

  pthread_cond_broadcast(arg->waitTurn);
  
  pthread_mutex_unlock(arg->semB);

  pthread_exit(NULL);
}

//-------------------------------------------------------------------

int main() {
  int symcount;
  std::cin >> symcount;
  std::cin.ignore();

  std::vector<char> chars;
  std::vector<int> freqs;

  std::string line;
  for (int i = 0; i < symcount; i++) { // extract chars and freqs
    std::getline(std::cin, line);

    char symbol = line[0];
    int value = std::stoi(line.substr(2));

    chars.push_back(symbol);
    freqs.push_back(value);
  }

  node *root = huffman(chars, freqs); // create the Huffman tree

  pthread_t *threadid = new pthread_t[symcount]; // pthread array

  int totfreq = 0;

  for (int i = 0; i < freqs.size(); i++) { // size of the message
    totfreq += freqs.at(i);
  }

  std::string message(totfreq, '_'); // the final result

  int j = 0;

  pthread_mutex_t mutex;
  pthread_mutex_init(&mutex, NULL);

  pthread_cond_t waitTurn =
      PTHREAD_COND_INITIALIZER; // Condition variable to control the turn

  int turn = 0; // initialize the turn here (shared resource)

  threadinfo *cont = new threadinfo;
  
  cont->message = &message;
  cont->treenode = root;
  cont->turn = &turn;
  cont->semB = &mutex;
  cont->waitTurn = &waitTurn;
  cont->nthreads = symcount; // size checker for the turn

  std::string cline;
   while (getline(std::cin, cline)) { // extract the traversal and positions

    pthread_mutex_lock(cont->semB); //first critical section
    
    std::vector<int> positions;
    std::string s1 = cline.substr(0, cline.find(' ')); // traversal string (binary)
    cont->trav = s1;

    int e = cline.find(' ');
    std::stringstream ss(cline.substr(e, cline.length())); // positions
    int n;

    while (ss >> n) { // get all positions
      positions.push_back(n);
    }
    
    cont->positions = positions;
    cont->threadnumber = j;
    
    if (pthread_create(&threadid[j], NULL, decode, (void *)cont)) {
      fprintf(stderr, "Error creating thread\n");
      return 1;
    }
    

    j++;
  }

  for (int i = 0; i < symcount; i++) {
    pthread_join(threadid[i], NULL);
  }

  //------------Message-------------------

  std::cout << "Original message: ";

  for (int i = 0; i < totfreq; i++) {
    std::cout << message[i];
  }

  delete[] threadid;
  delete cont;
  
  return 0;
}
