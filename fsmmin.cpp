
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
  
  set< set<int> > c = equiv.maximalClasses();
  
  char nl = 'a';
  set< set<int> >::iterator i = c.begin();
  while (i != c.end()) {
    set<int>::iterator j = (*i).begin();
    cout << nl++ << " = ";
    while (j != (*i).end()) {
      if (j != (*i).begin())
        cout << ", ";
      cout << infsm->states[*j].label;
      j++;
    }
    cout << '\n';
    i++;
  }
  
  return 0;
}
