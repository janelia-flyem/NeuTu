extern int Stack_Width(const Stack *stack);
extern int Stack_Height(const Stack *stack);
extern int Stack_Depth(const Stack *stack);
extern int Stack_Kind(const Stack *stack);
extern int Stack_Plane_Area(const Stack *stack);
extern size_t Stack_Volume(const Stack *stack);

extern void Stack_Set_Width(Stack *stack, int width);
extern void Stack_Set_Height(Stack *stack, int height);
extern void Stack_Set_Depth(Stack *stack, int depth);
extern void Stack_Set_Kind(Stack *stack, int kind);
extern void Stack_Set_Attribute(Stack *stack, int width, int height, int depth, 
    int kind);
extern int Stack_Channel_Number(const Stack *stack);
extern size_t Stack_Voxel_Number(const Stack *stack);
extern size_t Stack_Voxel_Bsize(const Stack *stack);
extern size_t Stack_Array_Bsize(const Stack *stack);
extern int Stack_Contain_Point(const Stack *stack, int x, int y, int z);
extern int Stack_Is_Empty(const Stack *stack);
extern int Stack_Is_Dark(const Stack *stack);
