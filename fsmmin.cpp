
#include "fsm.h"
#include "equivgraph.h"
#include <iostream>

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
    cout << " coalesced constraints=(";
    set< set<int> > cc = (*i).coalescedConstraints();
    for (set< set<int> >::iterator j=cc.begin(); j!=cc.end(); j++) {
      for(set<int>::iterator k=(*j).begin(); k!=(*j).end(); k++) {
        cout << infsm->states[*k].label << " ";
      }
      cout << ", ";
    }
    cout << ")";
    cout << '\n';
    i++;
  }
  
  return 0;
}
