#include "zworkspacefactory.h"
#include "tz_trace_utils.h"
#include "tz_locseg_chain.h"

ZWorkspaceFactory::ZWorkspaceFactory()
{
}

Trace_Workspace* ZWorkspaceFactory::createTraceWorkspace(
    const Stack *stack) const
{
  Trace_Workspace *ws = New_Trace_Workspace();
  Locseg_Chain_Default_Trace_Workspace(ws, stack);
  ws->refit = FALSE;
  ws->tune_end = TRUE;
  ws->add_hit = FALSE;
  ws->fit_workspace = New_Locseg_Fit_Workspace();
  ws->min_chain_length = 15;

  return ws;
}


Connection_Test_Workspace*
ZWorkspaceFactory::createConnectionTestWorkspace() const
{
  Connection_Test_Workspace *ws = New_Connection_Test_Workspace();
  ws->sp_test = 1;

  return ws;
}
