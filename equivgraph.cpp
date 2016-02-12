
#include "equivgraph.h"
#include <algorithm>
#include <iostream>
#include <iomanip>


#define UNKNOWN      (-1)
#define INCOMPATIBLE (0)
#define EQUIVALENT   (1)
#define COMPATIBLE   (2)


equivgraph::equivgraph(fsm& m) : machine(m)
{
  equiv = vector< vector<equivedge> >(m.states.size(), vector<equivedge>(m.states.size()));
  paullUnger();
}


int equivgraph::paullUnger_(int s0, int s1, bool partial)
{
  int i, j, r, f, n0, n1;
  bool anyundef;
  
  /* Every state is always equivalent to itself */
  if (s0 == s1) {
    equiv[s0][s0].state = e_equivalent;
    return e_equivalent;
  }
  
  /* Cycle: if in at least one previous state in the cycle an undefined next
   * state or output constraint was found, the cycle is made of pairs of 
   * compatible states; otherwise, the cycle must be part of a clique of
   * equivalent states. */
  if (equiv[s0][s1].state == e_maybe_compatible) {
    r = partial ? e_compatible : e_equivalent;
    equiv[s0][s1].state = equiv[s1][s0].state = r;
    return r;
  }
  
  /* Don't unnecessarily recompute compatibility statuses */
  if (equiv[s0][s1].state != e_unknown)
    return equiv[s0][s1].state;
    
  /* If two states, for every input, go to equivalent states and have the same
   * output, they are equivalent. If at least one output or at least one
   * next state is undefined, and all other states and output are equivalent
   * they are compatible. Otherwise, they must be incompatible. */
  
  /* Check if there are different or undefined outputs. */
  equiv[s0][s1].state = equiv[s1][s0].state = e_maybe_compatible;
  anyundef = false;
  for (i=0; i<machine.numnext; i++) {
    string o0 = machine.states[s0].out[i];
    string o1 = machine.states[s1].out[i];
    
    for (j=0; j<o0.length(); j++) {
      if (o0[j] != o1[j]) {
        if (o0[j] == '-' || o1[j] == '-')
          anyundef = true;
        else
          return equiv[s0][s1].state = equiv[s1][s0].state = e_incompatible;
      }
    }
  }
  
  /* Check if there are undefined next states. */
  for (i=0; i<machine.numnext; i++) {
    if (machine.states[s0].next[i] < 0 || machine.states[s1].next[i] < 0)
      anyundef = true;
  }
  
  /* If there is at least one undefined next state, or at least one undefined
   * output, the two states can't be equivalent. */
  f = anyundef ? e_compatible : e_equivalent;
  
  /* Check compatibility status of next states */
  for (i=0; i<machine.numnext; i++) {
    n0 = machine.states[s0].next[i];
    n1 = machine.states[s1].next[i];
    if (n0 < 0 || n1 < 0)
      continue;
      
    r = paullUnger_(n0, n1, anyundef);
    if (r == e_incompatible)
      return equiv[s0][s1].state = equiv[s1][s0].state = e_incompatible;
    if (r == e_compatible) {
      f = e_compatible;
      set<int> c;
      c.insert(n0);
      c.insert(n1);
      equiv[s0][s1].constraints.insert(c);
    }
  }
  equiv[s1][s0].constraints = equiv[s0][s1].constraints;
  
  return equiv[s0][s1].state = equiv[s1][s0].state = f;
}


void equivgraph::paullUnger(void)
{
  int i, j;
  
  for (i=0; i<machine.states.size(); i++)
    equiv[i][i].state = e_equivalent;
  for (i=0; i<machine.states.size(); i++) {
    for (j=i+1; j<machine.states.size(); j++) {
      paullUnger_(i, j, false);
    }
  }
}


void equivgraph::printEquivTable(ostream& s)
{
  int i, j, w, tw;
  int maxw;
  
  maxw = machine.states[1].label.length();
  for (i=2; i<machine.states.size(); i++) {
    maxw = max((int)machine.states[i].label.length(), maxw);
  }
  
  for (i=1; i<machine.states.size(); i++) {
    s << setw(maxw) << machine.states[i].label << " ";
    for (j=0; j<i; j++) {
      tw = machine.states[j].label.length();
      w = max(1, (tw - 1)/2);
      s << setw(w);
      switch (equiv[i][j].state) {
        case e_incompatible:   s << 'X'; break;
        case e_equivalent:     s << '~'; break;
        case e_compatible:     s << 'V'; break;
        default:               s << '?';
      }
      s << setw(tw - w + 1) << " ";
    }
    s << '\n';
  }
  s << setw(maxw) << " " << " ";
  for (i=0; i<machine.states.size()-1; i++) {
    s << machine.states[i].label << " "; 
  }
  s << '\n';
}


void equivgraph::printEquivTableNeato(ostream& s) 
{
  int i, j;
  
  s << "graph G {\n";
  for (i=1; i<machine.states.size(); i++) {
    for (j=0; j<i; j++) {
      if (equiv[i][j].state != e_incompatible) {
        s << machine.states[i].label << " -- " << machine.states[j].label;
        
        if (equiv[i][j].state == e_compatible) {
          s << " [label=\"";
          
          set< set<int> >::iterator k = equiv[i][j].constraints.begin();
          for (; k!=equiv[i][j].constraints.end(); k++) {
            s << "(";
            
            set<int>::iterator l = (*k).begin();
            for (; l!=(*k).end(); l++) {
              if (l != (*k).begin())
                s << ",";
              s << machine.states[*l].label;
            }
            
            s << ")\\n";
          }
          s << "\"]";
        }
        
        s << ";\n";
      }
    }
  }
  s << "}\n";
}




