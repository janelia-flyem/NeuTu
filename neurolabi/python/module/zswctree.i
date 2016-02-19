%{
#include "zswctree.h"
%}

%include zswctree.h

%inline %{
  std::vector<char> EncodeSwcTree(ZSwcTree &tree) {
    std::string str = tree.toString();
    std::vector<char> array(str.size());
    memcpy(&(array[0]), str.c_str(), array.size());
    return array;
  }

  ZSwcTree* DecodeSwcTree(const std::vector<char> &data) {
    ZSwcTree *tree = new ZSwcTree;
    tree->loadFromBuffer(&(data[0]));
    return tree;
  }

  ZSwcTree* CreateSwcTree() {
    return new ZSwcTree;
  }

  void DeleteSwcTree(ZSwcTree *tree) {
    delete tree;
  }
%}

