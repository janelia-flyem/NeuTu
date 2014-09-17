%module neutube
%{
#include "tz_stack_lib.h"
#include "tz_image_io.h"
#include "zstack.hxx"
#include "zstackskeletonizer.h"
#include "zclosedcurve.h"
#include "zswctree.h"
#include "swctreenode.h"
#include "zobject3dscan.h"
#include "zstackfile.h"
#include "tz_darray.h"
#include "zpoint.h"
#include "zintpoint.h"
#include "zintcuboid.h"
%}

%include "std_vector.i"
%include "std_string.i"

namespace std {
  %template(Double_Array) vector<double>;
  %template(Byte_Array) vector<uint8_t>;
  %template(Char_Array) vector<char>;
  %template(Point_Array) vector<ZPoint>;
}

%inline %{
  struct PStack {
    int width;
    int height;
    int depth;
    uint8_t *array;
  };

  struct PStack Python_Stack(const Stack *stack) {
    struct PStack out;
    out.width = stack->width;
    out.height = stack->height;
    out.depth = stack->depth;
    out.array = stack->array;

    return out;
  }

  void DeleteStackObject(ZStack *stack) {
    delete stack;
  }

  void PrintDoubleArray(const std::vector<double> &array) {
    darray_print(&(array[0]), array.size());
  }

  void PrintCharArray(const std::vector<char> &array) {
    printf("%c\n", array[0]);
  }

  void PrintString(const std::string &str) {
    printf("%s\n", str.c_str());
  }

  std::vector<char> CharArrayTest() {
    std::vector<char> array(2);
    array[0] = 'h';
    array[1] = 'w';
    return array;
  }
%}

%include stack_io.i
%include stack_attribute.i
%include zstack.hxx
%include zstackobject.h
%include zswctree.i
%include zstackskeletonizer.h
%include zclosedcurve.h
%include zobject3dscan.i
%include darray.i
%include zstackfile.h
%include zpointarray.i
%include zflyemneuron.i
%include zflyemqualityanalyzer.i
%include zflyemhotspot.i
%include zintcuboid.h
