
#import "equivgraph.h"
#import "fsm.h"


#ifndef MINIMIZE_H
#define MINIMIZE_H


fsm minimizedFsmFromPrimitiveClasses(equivgraph &equiv, bool verbose);
fsm minimizedFsmFromMaximalClasses(equivgraph &equiv);


#endif

