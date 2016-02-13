
#include "fsm.h"
#include "equivgraph.h"
#include "minimize.h"
#include <iostream>
#include <utility>

using namespace std;


int main(int argc, char *argv[]) {
  fsm *infsm;
  
  try {
    infsm = new fsm(cin);
  } catch (const char *e) {
    cout << e << '\n';
    return 1;
  }
  
  equivgraph equiv(*infsm);
  equiv.printEquivTableNeato(cout);
  
  set< equivalence > c = equiv.primitiveClasses();
  
  char nl = 'a';
  set< equivalence >::iterator i = c.begin();
  while (i != c.end()) {
    cout << nl++ << " = ";
    cout << *i;
    cout << " coalesced constraints=";
    cout << formatSetOfClasses((*i).coalescedConstraints(), *infsm);
    cout << '\n';
    i++;
  }
  
  fsm newfsm = minimizedFsmFromPrimitiveClasses(*infsm);
  newfsm.printFsm(cout);
  
  return 0;
}
