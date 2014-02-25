%module readstack
%{
  #include "tz_stack_lib.h"
  #include "tz_image_io.h"
  extern Stack* Read_Stack_U(const char *filePath);
%}

extern Stack* Read_Stack_U(const char *filePath);
