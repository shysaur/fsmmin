
#import "equivgraph.h"
#import "fsm.h"


#ifndef MINIMIZE_H
#define MINIMIZE_H


fsm minimizedFsmFromPrimitiveClasses(fsm& ifsm, bool verbose);
fsm minimizedFsmFromMaximalClasses(fsm& ifsm);


#endif

