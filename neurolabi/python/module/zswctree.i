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

  void DeleteSwcTree(ZSwcTree *tree) {
    delete tree;
  }
%}

