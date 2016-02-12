
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
  set< set<int> > maximalClasses(void);
  void printEquivTable(ostream& s) const;
  void printEquivTableNeato(ostream& s) const;
private:
  set< set<int> > cliquesCache;
  int paullUnger_(int s0, int s1, bool partial);
  void paullUnger(void);
  void bronKerbosch(set< set<int> >& cliq, set<int> r, set<int> p, set<int> x) const;
};


#endif
