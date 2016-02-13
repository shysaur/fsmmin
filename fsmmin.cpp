
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
    cout << '\n';
    i++;
  }
  
  return 0;
}
