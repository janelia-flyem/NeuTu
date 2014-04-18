#ifndef ZWORKSPACEFACTORY_H
#define ZWORKSPACEFACTORY_H

#include "tz_workspace.h"
#include "tz_trace_defs.h"
#include "tz_graph_defs.h"

class ZWorkspaceFactory
{
public:
  ZWorkspaceFactory();

  Trace_Workspace* createTraceWorkspace(const Stack *stack) const;
  Connection_Test_Workspace* createConnectionTestWorkspace() const;


};

#endif // ZWORKSPACEFACTORY_H
