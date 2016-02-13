
#include "equivgraph.h"
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <queue>


#define UNKNOWN      (-1)
#define INCOMPATIBLE (0)
#define EQUIVALENT   (1)
#define COMPATIBLE   (2)


equivalence::equivalence(const equivgraph& graph, set<int> newstates) : graph(graph)
{
  add(newstates);
}


void equivalence::add(set<int> newstates)
{
  set<int>::iterator i = newstates.begin();
  for (; i!=newstates.end(); i++) {
    add(*i);
  }
}


void equivalence::add(int newstate)
{
  set<int>::iterator i = states.begin();
  for (; i!=states.end(); i++) {
    if (graph.equiv[newstate][*i].state == e_incompatible)
      throw;
    if (graph.equiv[newstate][*i].state == e_compatible) {
      constraints.insert(graph.equiv[newstate][*i].constraints.begin(), 
                         graph.equiv[newstate][*i].constraints.end());
    }
  }
  
  states.insert(newstate);
  
  vector< set< set<int> >::iterator > toremove;
  set< set<int> >::iterator j = constraints.begin();
  for (; j!=constraints.end(); j++) {
    bool allin = true;
    set<int>::iterator k = (*j).begin();
    for (; k!=(*j).end(); k++) {
      if (states.find(*k) == states.end()) {
        allin = false;
        break;
      }
    }
    if (allin)
      toremove.push_back(j);
  }
  
  int k;
  for (k=0; k<toremove.size(); k++) {
    constraints.erase(toremove[k]);
  }
}


bool equivalence::coversWithLessOrEqualConstraints(equivalence& c)
{ 
  if (states.size() <= c.states.size())
    return false;
  if (!includes(states.begin(), states.end(), c.states.begin(), c.states.end()))
    return false;
  return includes(c.constraints.begin(), c.constraints.end(), constraints.begin(), constraints.end());
}


ostream& operator<<(ostream& os, const equivalence& obj)
{
  os << "(";
  
  set<int>::iterator i = obj.states.begin();
  for (; i!=obj.states.end(); i++) {
    if (i != obj.states.begin())
      os << ",";
    os << obj.graph.machine.states[*i].label;
  }
  
  if (obj.constraints.size() > 0) {
    os << "; constraints=(";
    
    set< set<int> >::iterator i = obj.constraints.begin();
    for (; i!=obj.constraints.end(); i++) {
      if (i != obj.constraints.begin())
        os << ",";
      os << "(";
      set<int>::iterator j = (*i).begin();
      for (; j!=(*i).end(); j++) {
        if (j != (*i).begin())
          os << ",";
        os << obj.graph.machine.states[*j].label;
      }
      os << ")";
    }
    os << ")";
  }
  os << ")";
  return os;
}


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


void equivgraph::bronKerbosch(set<equivalence>& cliq, set<int> r, set<int> p, set<int> x) const
{
  int i, v;
  
  if (p.size() == 0 && x.size() == 0) {
    equivalence e(*this, r);
    cliq.insert(e);
    return;
  }
  
  while (p.size() > 0) {
    v = *(p.begin());
    
    set<int> newr = r;
    newr.insert(v);
    
    set<int> newx = x;
    set<int> newp = p;
    for (i=0; i<machine.states.size(); i++) {
      if (equiv[v][i].state == e_incompatible || v == i) {
        newp.erase(i);
        newx.erase(i);
      }
    }
    
    bronKerbosch(cliq, newr, newp, newx);
    
    p.erase(p.begin());
    x.insert(v);
  }
}


set<equivalence> equivgraph::maximalClasses(void)
{
  set<int> p;
  int i;
  
  if (cliquesCache.size() > 0)
    return cliquesCache;
  
  for (i=0; i<machine.states.size(); i++) 
    p.insert(i);
  bronKerbosch(cliquesCache, set<int>(), p, set<int>());
  return cliquesCache;
}


set<equivalence> equivgraph::primitiveClasses(void)
{
  if (subcliquesCache.size() > 0)
    return subcliquesCache;
    
  vector<equivalence> bigger;  
  queue<equivalence> gen;
  
  set<equivalence> maxc = maximalClasses();
  for (set<equivalence>::iterator i=maxc.begin(); i!=maxc.end(); i++) {
    gen.push(*i);
  }
  
  while (!gen.empty()) {
    equivalence e = gen.front();
    gen.pop();
    
    bool existsbigger = false;
    for (int i=0; i<bigger.size(); i++) {
      if (bigger[i].coversWithLessOrEqualConstraints(e)) {
        existsbigger = true;
        break;
      }
    }
    if (!existsbigger)
      subcliquesCache.insert(e);
    else
      subcliquesCache.erase(e);
    
    bigger.push_back(e);
    
    set<int>::iterator i = e.states.begin();
    for (; i != e.states.end(); i++) {
      set<int> t = e.states;
      t.erase(*i);
      if (!t.empty()) {
        equivalence f(*this, t);
        gen.push(f);
      }
    }
  }
  return subcliquesCache;
}


void equivgraph::printEquivTable(ostream& s) const
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


void equivgraph::printEquivTableNeato(ostream& s) const 
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




