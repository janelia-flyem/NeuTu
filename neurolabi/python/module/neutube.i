%module neutube
%{
#include "tz_stack_lib.h"
#include "tz_image_io.h"
#include "zstack.hxx"
#include "zstackdrawable.h"
#include "zstackskeletonizer.h"
#include "zswctree.h"
#include "swctreenode.h"
%}

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
%}

%include stack_io.i
%include stack_attribute.i
%include zstack.hxx
%include zstackdrawable.h
%include zswctree.h
%include zstackskeletonizer.h
%include swctreenode.h
