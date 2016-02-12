
#include "fsm.h"
#include <vector>
#include <set>
#include <utility>

using namespace std;


#ifndef EQUIVGRAPH_H
#define EQUIVGRAPH_H


enum {
  e_unknown = -1,
  e_incompatible,
  e_equivalent,
  e_maybe_compatible,
  e_compatible
};


class equivedge {
public:
  int state;
  set< set<int> > constraints;
  equivedge(void) {
    state = e_unknown;
  }
};


class equivgraph {
public:
  fsm machine;
  vector< vector<equivedge> > equiv;
  equivgraph(fsm& m);
  void printEquivTable(ostream& s) const;
  void printEquivTableNeato(ostream& s) const;
private:
  int paullUnger_(int s0, int s1, bool partial);
  void paullUnger(void);
};


#endif
