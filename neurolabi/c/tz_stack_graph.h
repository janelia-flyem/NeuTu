/** @file tz_stack_graph.h
 *  @brief Stack graph operation.
 *  @author Ting Zhao
 *  @date 06-Aug-08
 */

#ifndef _TZ_STACK_GRAPH_H_
#define _TZ_STACK_GRAPH_H_

#include "tz_cdefs.h"
#include "tz_graph.h"
#include "tz_int_arraylist.h"

__BEGIN_DECLS

#include <image_lib.h>

/**@typedef double Weight_Func_t(void *)
 *
 * Weight_Func_t defines a weight function.
 */
typedef double Weight_Func_t(void *);

/**@struct Stack_Graph_Workspace
 *
 * Workspace for stack graph operations.
 */
#define STACK_GRAPH_WORKSPACE_ARGC 10
typedef struct _Stack_Graph_Workspace {
  int conn;             /**< neighborhood connectivity */
  int *range;           /**< range to build the graph */
  double resolution[3];        /**< resolution of a voxel */
  Weight_Func_t *wf;    /**< weight function */
  int sp_option;        /**< shortest path options */
  double argv[STACK_GRAPH_WORKSPACE_ARGC]; /**< arguments for wf*/
  Graph_Workspace *gw;  /**< graph workspace */
  Stack *group_mask;    /**< the mask to define same-group voxels. */
  Stack *signal_mask;   /**< the mask to define excluded background. */
  double *intensity; /**< intensity array */
  double value; /**< to save some real-value result */
  int virtualVertex; /**< the starting virtual vertex */
  BOOL including_signal_border; /**< Include background voxel touching the foreground*/
  double greyFactor;
  double greyOffset;
} Stack_Graph_Workspace;

Stack_Graph_Workspace* New_Stack_Graph_Workspace();
void Default_Stack_Graph_Workspace(Stack_Graph_Workspace *sgw);
void Delete_Stack_Graph_Workspace(Stack_Graph_Workspace *sgw);

void Clean_Stack_Graph_Workspace(Stack_Graph_Workspace *sgw);
void Kill_Stack_Graph_Workspace(Stack_Graph_Workspace *sgw);

void Stack_Graph_Workspace_Set_Range(Stack_Graph_Workspace *sgw, int x0, 
				     int x1, int y0, int y1, int z0, int z1);
void Stack_Graph_Workspace_Set_Range_M(Stack_Graph_Workspace *sgw, int x0, 
				       int x1, int y0, int y1, int z0, int z1,
				       int mx0, int mx1, int my0, int my1,
				       int mz0, int mz1);

void Stack_Graph_Workspace_Update_Range(Stack_Graph_Workspace *sgw,
					int x, int y, int z);

void Stack_Graph_Workspace_Expand_Range(Stack_Graph_Workspace *sgw, 
					int mx0, int mx1, 
					int my0, int my1,
					int mz0, int mz1);

void Stack_Graph_Workspace_Validate_Range(Stack_Graph_Workspace *sgw, 
					  int width, int height, int depth);

double Stack_Graph_Workspace_Dist(const Stack_Graph_Workspace *sgw, int index);

/*@brief Geodesic distance functions
 *
 */
/* argv[0]: d; argv[1]: v1; argv[2]: v2; additional parameters: ...*/
double Stack_Voxel_Weight(void *argv);
double Stack_Voxel_Weight_I(void *argv);
double Stack_Voxel_Weight_R(void *argv);
double Stack_Voxel_Weight_A(void *argv);
/* argv[3]: alpha; argv[4]: beta */
double Stack_Voxel_Weight_S(void *argv);

/* Reflected version of Stack_Voxel_Weight_S for bright background */
double Stack_Voxel_Weight_Sr(void *argv);
double Stack_Voxel_Weight_Srb(void *argv);
double Stack_Voxel_Weight_Srw(void *argv);

/* color weight */
double Stack_Voxel_Weight_C(void *argv);

Graph* Stack_Graph(const Stack *stack, int conn, const int *range, 
		   Weight_Func_t *wf);

Graph* Stack_Graph_W(const Stack *signal, Stack_Graph_Workspace *sgw);

/**@brief Find shortest paths in a stack.
 *
 * Stack_Shortest_Path() returns the shortest paths of the chosen voxels to 
 * \a start, which is the index of the starting voxel. Each element of the 
 * returned array is an index in the substack. Use Stack_Subindex() to get the
 * original index.
 *
 * @note The returned pointer is associated with \a sgw. So there is no need to
 * free it separately.
 */
int* Stack_Shortest_Path(const Stack *stack, int start, 
			 Stack_Graph_Workspace *sgw);

/**@brief Make the result from Stack_Shorest_Path() easier to understand.
 *
 * Parse_Stack_Shortest_Path() returns an array list that is the path from
 * \a start to \a end. Each element is an index in the original stack.
 */
Int_Arraylist* Parse_Stack_Shortest_Path(int *path, int start, int end,
					 int width, int height,
					 Stack_Graph_Workspace *sgw);

Int_Arraylist *Stack_Route(const Stack *stack, int start[], int end[],
			   Stack_Graph_Workspace *sgw);

__END_DECLS

#endif

 
