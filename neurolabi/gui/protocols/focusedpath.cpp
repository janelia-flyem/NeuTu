#include "focusedpath.h"

#include "dvid/zdvidannotation.h"


FocusedPath::FocusedPath(ZDvidAnnotation annotation)
{



}

// data:
// endpoints: zintpoint x 2
// edge points: qlist of zintpoint
// actual edges (?): map of points to edge class instance?
// probability: double

// methods:

// init: from dvid annotation json (one endpoint)?


// getters: endpoints, edges, probability
// setters for: probability (only thing we expect to change)
// save (for changing probability)
// reload/refresh (depending on how much stuff we load at init (eg, edges))

// below: really, its state is broken, connected, or unknown
// isvalid (has prob > 0)
// isbroken (at least one edge has been split and/or invalidated by user)
// isconnected (same body at body ends)


