
extern Stack* Read_Stack_U(const char *filePath);
%inline %{
  void Write_Stack_P(const Stack *stack, const char *filePath) {
    Write_Stack_U(filePath, stack, NULL);
  }
%}
extern Stack* Make_Stack(int kind, int width, int height, int depth);
extern void Kill_Stack(Stack *stack);

