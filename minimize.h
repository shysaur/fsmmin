
#include "equivgraph.h"
#include "fsm.h"


#ifndef MINIMIZE_H
#define MINIMIZE_H


fsm minimizedFsmFromPrimitiveClasses(equivgraph &equiv, bool verbose, int wcov, int wcon, int wsol);
fsm minimizedFsmFromMaximalClasses(equivgraph &equiv);


#endif

