#include <algorithm>
#include <iostream>
#include <vector>

struct node {
  char c;
  int freq;
  node *left, *right;
};

node *newNode(char data, int frequency) {
  node *nnode = new node;
  nnode->c = data;
  nnode->freq = frequency;
  nnode->left = nnode->right = NULL;
  return nnode;
}

bool cmp(node *a, node *b) {
  if (a->freq == b->freq) {
    if (a->c == b->c)
      return a > b;
    return a->c < b->c;
  }
  return a->freq < b->freq;
}

//-----------------------------------------------------------------------------------------------

node *huffman(std::vector<char> &chars, std::vector<int> &freqs) {
  std::vector<node *> nodes;

  for (int i = 0; i < chars.size(); i++) {
    node *n = newNode(chars[i], freqs[i]);
    nodes.push_back(n);
  }

  while (nodes.size() > 1) {
    sort(nodes.begin(), nodes.end(), cmp);

    node *left = nodes[0];
    node *right = nodes[1];

    node *parent = newNode('\0', left->freq + right->freq);
    parent->left = left;
    parent->right = right;

    nodes.erase(nodes.begin());
    nodes.erase(nodes.begin());

    nodes.push_back(parent);
  }

  node *root = nodes[0];

  return root;
}

