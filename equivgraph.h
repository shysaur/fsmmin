
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


class equivgraph;


class equivalence {
public:
  set<int> states;
  set< set<int> > constraints;
  equivalence(const equivgraph& graph, set<int> newstates);
  void add(int newstate);
  void add(set<int> newstates);
  bool coversWithLessOrEqualConstraints(equivalence& c);
  friend inline bool operator< (const equivalence& lhs, const equivalence& rhs) 
    { return lhs.states < rhs.states; }
  friend inline bool operator==(const equivalence& lhs, const equivalence& rhs) 
    { return lhs.states == rhs.states; }
  friend ostream& operator<<(ostream& os, const equivalence& obj);
private:
  const equivgraph& graph;
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
  set<equivalence> maximalClasses(void);
  set<equivalence> primitiveClasses(void);
  void printEquivTable(ostream& s) const;
  void printEquivTableNeato(ostream& s) const;
private:
  set<equivalence> cliquesCache;
  set<equivalence> subcliquesCache;
  int paullUnger_(int s0, int s1, bool partial);
  void paullUnger(void);
  void bronKerbosch(set<equivalence>& cliq, set<int> r, set<int> p, set<int> x) const;
};


#endif
