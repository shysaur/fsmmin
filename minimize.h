
#import "equivgraph.h"
#import "fsm.h"

using namespace std;


#ifndef MINIMIZE_H
#define MINIMIZE_H


fsm minimizedFsmFromPrimitiveClasses(fsm& ifsm);
fsm minimizedFsmFromMaximalClasses(fsm& ifsm);


#endif

