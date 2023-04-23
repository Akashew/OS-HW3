#include "Huffman.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <pthread.h>
#include <sstream>
#include <string>
#include <vector>

struct threadinfo {
  std::vector<int> positions;
  std::string *message;
  node *treenode;
  std::string trav;
  int threadnumber;
  int nthreads; 
  int *turn;
  pthread_mutex_t *semB;
  pthread_cond_t *waitTurn;
};

node *traverse(node *tree, std::string tra, int index) {

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

void *decode(void *void_ptr) {
    
  threadinfo *arg = (threadinfo *)void_ptr;

  pthread_mutex_lock(arg->semB);
  
  while(arg->threadnumber != *(arg->turn)){
        pthread_cond_wait(arg->waitTurn, arg->semB);
  }
  
  node *info = traverse(arg->treenode, arg->trav, 0); // info node

  //pthread_mutex_unlock(arg->semB);

  for (int i = 0; i < arg->positions.size(); i++) {

    int pos = arg->positions.at(i); // the position
  
    (*arg->message)[pos] = info->c;  //insert the char into the position of string
    
  }

  std::cout << "Symbol: " << info->c << ", Frequency: " << info->freq
            << ", Code: " << arg->trav << std::endl;

  //pthread_mutex_lock(arg->semB);
            
  if(*(arg->turn) >= arg->nthreads){
      *(arg->turn) = 1;
  }else{
      *(arg->turn) = *(arg->turn) + 1;
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

  std::string message(totfreq, '_'); //the final result

  int j = 0;
  
  pthread_mutex_t mutex;
  pthread_mutex_init(&mutex, NULL);
  
  pthread_cond_t waitTurn = PTHREAD_COND_INITIALIZER;  // Condition variable to control the turn

  int turn = 0; // initialize the turn here

  std::string cline;
  while (getline(std::cin, cline)) { //extract the traversal and positions

    threadinfo *cont = new threadinfo(); 
    cont->message = &message;
    cont->treenode = root;

    std::string s1 = cline.substr(0, cline.find(' ')); // traversal string
    cont->trav = s1;

    int e = cline.find(' ');
    std::stringstream ss(cline.substr(e, cline.length())); // positions
    int n;

    while (ss >> n) { //get all positions
      cont->positions.push_back(n);
    }
    
    cont->threadnumber = j;
    
    cont->turn = &turn;

    cont->semB = &mutex;
    
    cont->waitTurn = &waitTurn; 
    
    cont->nthreads = symcount; //size checker for the turn

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
  
  return 0;
}