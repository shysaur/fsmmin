
#include "equivgraph.h"
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <queue>

using namespace std;


equivalence::equivalence(const equivgraph& g, set<int> newstates)
{
  graph = &g;
  add(newstates);
}


set< set<int> > equivalence::coalescedConstraints(void) const
{
  set< set<int> > res;
  
  for (int i=0; i<graph->machine.numnext; i++) {
    set<int> t;
    for (set<int>::iterator j=states.begin(); j!=states.end(); j++) {
      int n = graph->machine.states[*j].next[i];
      if (n >= 0)
        t.insert(n);
    }
    if (t.size() > 1 && !includes(states.begin(), states.end(), t.begin(), t.end()))
      res.insert(t);
  }
  return res;
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
  for (set<int>::iterator i = states.begin(); i!=states.end(); i++) {
    if (graph->equiv[newstate][*i].state == e_incompatible)
      throw;
    constraints.insert(graph->equiv[newstate][*i].constraints.begin(), 
                       graph->equiv[newstate][*i].constraints.end());
  }
  
  states.insert(newstate);
  
  for (set< set<int> >::iterator j = constraints.begin(); j!=constraints.end(); ) {
    bool allin = true;
    
    for (set<int>::iterator k = (*j).begin(); k!=(*j).end(); k++) {
      if (states.find(*k) == states.end()) {
        allin = false;
        break;
      }
    }
    if (allin)
      constraints.erase(j++);
    else
      j++;
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
  string tmp, tmp2;
  
  tmp = formatSetOfStates(obj.states, obj.graph->machine);
  tmp2 = formatSetOfClasses(obj.constraints, obj.graph->machine);
  tmp.replace(tmp.length()-1, 0, "; constraints=" + tmp2);
  os << tmp;
  return os;
}


equivgraph::equivgraph(fsm& m) : machine(m)
{
  equiv = vector< vector<equivedge> >(m.states.size(), vector<equivedge>(m.states.size()));
  paullUnger();
}


int equivgraph::paullUnger_(int s0, int s1, bool partial)
{
  /* Every state is always equivalent to itself */
  if (s0 == s1) {
    equiv[s0][s0].state = e_equivalent;
    return e_equivalent;
  }
  
  /* Don't unnecessarily recompute compatibility statuses */
  if (equiv[s0][s1].state > e_unknown) {
    /* We have found a cycle! Mark where the circle closes, so that we
     * can make a decision after unwinding it all. */
    if (equiv[s0][s1].state < 0) {
      equiv[s0][s1].state = equiv[s1][s0].state = e_maybe_compatible_root;
      return e_unknown;
    }
    return equiv[s0][s1].state;
  }
    
  /* If two states, for every input, go to equivalent states and have the same
   * output, they are equivalent. If at least one output or at least one
   * next state is undefined, and all other states and output are equivalent
   * they are compatible. Otherwise, they must be incompatible. */
  
  /* Check if there are different or undefined outputs. */
  equiv[s0][s1].state = equiv[s1][s0].state = e_maybe_compatible;
  bool anyundef = false;
  for (int i=0; i<machine.numnext; i++) {
    string o0 = machine.states[s0].out[i];
    string o1 = machine.states[s1].out[i];
    
    for (int j=0; j<o0.length(); j++) {
      if (o0[j] != o1[j]) {
        if (o0[j] == '-' || o1[j] == '-')
          anyundef = true;
        else
          return equiv[s0][s1].state = equiv[s1][s0].state = e_incompatible;
      }
    }
  }
  
  /* Check if there are undefined next states. */
  for (int i=0; i<machine.numnext; i++) {
    if (machine.states[s0].next[i] < 0 || machine.states[s1].next[i] < 0)
      anyundef = true;
  }
  
  /* If there is at least one undefined next state, or at least one undefined
   * output, the two states can't be equivalent. */
  int f = (anyundef ? e_compatible : e_equivalent);
  bool anycycle = false;
  
  /* Check compatibility status of next states. Only if every pair of them is
   * compatible, then this pair is compatible. */
  for (int i=0; i<machine.numnext; i++) {
    int n0 = machine.states[s0].next[i];
    int n1 = machine.states[s1].next[i];
    
    if (n0 < 0 || n1 < 0)
      continue;
    if ((n0 == s0 && n1 == s1) || (n0 == s1 && n1 == s0))
      continue;
      
    int r = paullUnger_(n0, n1, anyundef);
    
    if (r == e_incompatible)
      /* s0 and s1 are incompatible; this also makes the cycle where they
       * might be part of invalid. */
      return equiv[s0][s1].state = equiv[s1][s0].state = e_incompatible;
    if (r == e_compatible || r == e_maybe_compatible)
      /* Downgrade equivalence to compatibility if a possible next state leads
       * to an undefined transition or output. */
      f = e_compatible;
    if (r < 0)
      /* This transition leads to a cycle. */
      anycycle = true;
      
    /* Update constraints. For this pair of states to be coalesced, all the
     * other compatible state pairs they lead to must be coalesced. */
    set<int> c;
    c.insert(n0);
    c.insert(n1);
    if (c.size() > 1)
      equiv[s0][s1].constraints.insert(c);
  }
  equiv[s1][s0].constraints = equiv[s0][s1].constraints;
  
  /* Note that, at this point, equiv[s0][s1].state == e_maybe_compatible_root 
   * iff s0,s1 is the first pair of states in the cycle that were examined. */
  if (anycycle && equiv[s0][s1].state != e_maybe_compatible_root) {
    /* One of our transitions is part of a cycle. The circle may be made of
     * compatible states, but at this point, not all states in the cycle were
     * fully checked to be compatible, so we can't say if that transition
     * leads to a pair of compatible states or not. */
    equiv[s0][s1].state = equiv[s1][s0].state = e_unknown;
    /* We just tell to the caller if there is some undefined transition or
     * output */
    return -f;
  }
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


void equivgraph::bronKerbosch(set<equivalence>& cliq, set<int>& r, set<int>& p, set<int>& x) const
{
  if (p.size() == 0 && x.size() == 0) {
    equivalence e(*this, r);
    cliq.insert(e);
    return;
  }
  
  while (p.size() > 0) {
    int v = *(p.begin());
    
    set<int> newr = r;
    newr.insert(v);
    
    set<int> newx = x;
    set<int> newp = p;
    for (int i=0; i<machine.states.size(); i++) {
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
  if (cliquesCache.size() > 0)
    return cliquesCache;
  
  set<int> r;
  set<int> p;
  set<int> x;
  for (int i=0; i<machine.states.size(); i++) 
    p.insert(i);
    
  bronKerbosch(cliquesCache, r, p, x);
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
  int zi = machine.states.size() > 1 ? 1 : 0;
  int maxw = machine.states[zi].label.length();
  for (int i=zi+1; i<machine.states.size(); i++) {
    maxw = max((int)machine.states[i].label.length(), maxw);
  }
  
  for (int i=zi; i<machine.states.size(); i++) {
    s << setw(maxw) << machine.states[i].label << " ";
    for (int j=0; j<=i-zi; j++) {
      int tw = machine.states[j].label.length();
      int w = max(1, (tw - 1)/2);
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
  for (int i=0; i<machine.states.size()-zi; i++) {
    s << machine.states[i].label << " "; 
  }
  s << '\n';
}


void equivgraph::printEquivTableNeato(ostream& s) const 
{
  s << "graph G {\n";
  for (int i=1; i<machine.states.size(); i++) {
    for (int j=0; j<i; j++) {
      if (equiv[i][j].state != e_incompatible) {
        s << machine.states[i].label << " -- " << machine.states[j].label;
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
        s << "\"];\n";
      }
    }
  }
  s << "}\n";
}




