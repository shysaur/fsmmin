
#include "fsm.h"
#include <vector>
#include <set>
#include <utility>


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
  std::set<int> states;
  std::set< std::set<int> > constraints;
  std::set< std::set<int> > coalescedConstraints(void) const;
  equivalence(const equivgraph& graph, std::set<int> newstates);
  void add(int newstate);
  void add(std::set<int> newstates);
  bool coversWithLessOrEqualConstraints(equivalence& c);
  friend inline bool operator< (const equivalence& lhs, const equivalence& rhs) 
    { return lhs.states < rhs.states; }
  friend inline bool operator==(const equivalence& lhs, const equivalence& rhs) 
    { return lhs.states == rhs.states; }
  friend std::ostream& operator<<(std::ostream& os, const equivalence& obj);
private:
  const equivgraph& graph;
};


class equivedge {
public:
  int state;
  std::set< std::set<int> > constraints;
  equivedge(void) {
    state = e_unknown;
  }
};


class equivgraph {
public:
  fsm machine;
  std::vector< std::vector<equivedge> > equiv;
  equivgraph(fsm& m);
  std::set<equivalence> maximalClasses(void);
  std::set<equivalence> primitiveClasses(void);
  void printEquivTable(std::ostream& s) const;
  void printEquivTableNeato(std::ostream& s) const;
private:
  std::set<equivalence> cliquesCache;
  std::set<equivalence> subcliquesCache;
  int paullUnger_(int s0, int s1, bool partial);
  void paullUnger(void);
  void bronKerbosch(std::set<equivalence>& cliq, std::set<int> r, 
                    std::set<int> p, std::set<int> x) const;
};


#endif
