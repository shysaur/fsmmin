
#include "fsm.h"
#include "equivgraph.h"
#include <iostream>
#include <utility>

using namespace std;


string print(const set<int>& s, fsm& m)
{
  string out = "(";
  
  for (set<int>::iterator i=s.begin(); i!=s.end(); i++) {
    if (i != s.begin())
      out += ",";
    out += m.states[*i].label;
  }
  out += ")";
  return out;
}


string print(const set< set<int> >& s, fsm& m)
{
  string out = "(";
  
  for (set< set<int> >::iterator i=s.begin(); i!=s.end(); i++) {
    if (i != s.begin())
      out += ",";
    out += print(*i, m);
  }
  out += ")";
  return out;
}


fsm buildFsmWithClasses(fsm& ifsm, vector< set<int> >& selCl) 
{
  fsm newfsm;
  newfsm.numnext = ifsm.numnext;
  newfsm.partial = ifsm.partial;
  
  for (int i=0; i<selCl.size(); i++) {
    fsmstate state;
    state.label.push_back('a'+i);
    for (int j=0; j<newfsm.numnext; j++) {
      set<int> go;
      bool gound = true;
      string out = "";
      
      for (set<int>::iterator k=selCl[i].begin(); k!= selCl[i].end(); k++) {
        if (ifsm.states[*k].next[j] >= 0) {
          go.insert(ifsm.states[*k].next[j]);
          gound = false;
        }
        if (out == "") {
          out = ifsm.states[*k].out[j];
        } else {
          for (int l=0; l<out.length(); l++) {
            if (ifsm.states[*k].out[j][l] != '-')
              out[l] = ifsm.states[*k].out[j][l];
          }
        }
      }
      state.out.push_back(out);
      if (!gound) {
        int k;
        for (k=0; k<selCl.size(); k++) {
          if (includes(selCl[k].begin(), selCl[k].end(), go.begin(), go.end())) {
            state.next.push_back(k);
            break;
          }
        }
        if (k == selCl.size())
          throw;
      } else
        state.next.push_back(-1);
    }
    newfsm.states.push_back(state);
  }
  return newfsm;
}


int benefit(set<int>& s, set< set<int> >& c, set<int> cs, set< set<int> > sc)
{
  int res = 0;
  
  /* +1 for each newly covered state */
  for (set<int>::iterator i=s.begin(); i!=s.end(); i++) {
    if (cs.find(*i) == cs.end())
      res++;
  }
  
  /* -1 for each new constraint */
  for (set< set<int> >::iterator i=c.begin(); i!=c.end(); i++) {
    if (sc.find(*i) == sc.end())
      res--;
  }
  
  /* +1 for each newly covered constraint */
  for (set< set<int> >::iterator i=sc.begin(); i!=sc.end(); i++) {
    if (includes(s.begin(), s.end(), (*i).begin(), (*i).end()))
      res++;
  }
  
  return res;
}


fsm minimizeFSMWithPrimitiveClasses(fsm& ifsm)
{
  equivgraph equiv(ifsm);
  set<equivalence> tc = equiv.primitiveClasses();
  vector< set<int> > availCl;
  vector< set< set<int> > > availConstr;
  vector< set<int> > selCl;
  set< set<int> > selConstr;
  set<int> coveredStates;
  
  for (set<equivalence>::iterator i=tc.begin(); i!=tc.end(); i++) {
    availCl.push_back((*i).states);
    availConstr.push_back((*i).coalescedConstraints());
  }
  
  while (coveredStates.size() < ifsm.states.size() || selConstr.size() > 0) {
    int maxi = 0;
    int maxb = benefit(availCl[0], availConstr[0], coveredStates, selConstr);
    cout << "benefit for class " << print(availCl[0], ifsm) << " = " << maxb << "\n";
    for (int i=1; i<availCl.size(); i++) {
      int b = benefit(availCl[i], availConstr[i], coveredStates, selConstr);
      cout << "benefit for class " << print(availCl[i], ifsm) << " = " << b << "\n";
      if (b >= maxb) {
        maxb = b;
        maxi = i;
      }
    }
    cout << "selecting class " << print(availCl[maxi], ifsm) << "\n";
    
    set<int> s = availCl[maxi];
    coveredStates.insert(s.begin(), s.end());
    selCl.push_back(s);
    availCl.erase(availCl.begin()+maxi);
    
    selConstr.insert(availConstr[maxi].begin(), availConstr[maxi].end());
    availConstr.erase(availConstr.begin()+maxi);
    
    for (set< set<int> >::iterator i=selConstr.begin(); i!=selConstr.end();) {
      if (includes(s.begin(), s.end(), (*i).begin(), (*i).end()))
        selConstr.erase(i++);
      else
        i++;
    }
    for (int j=0; j<availConstr.size(); j++) {
      for (set< set<int> >::iterator i=availConstr[j].begin(); i!=availConstr[j].end();) {
        if (includes(s.begin(), s.end(), (*i).begin(), (*i).end()))
          availConstr[j].erase(i++);
        else
          i++;
      }
    }
  }
  
  return buildFsmWithClasses(ifsm, selCl);
}


fsm minimizeFSMWithMaximalClasses(fsm& ifsm)
{
  equivgraph equiv(ifsm);
  set<equivalence> tc = equiv.maximalClasses();
  vector< set<int> > selCl;
  
  for (set<equivalence>::iterator i=tc.begin(); i!=tc.end(); i++)
    selCl.push_back((*i).states);
    
  return buildFsmWithClasses(ifsm, selCl);
}


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
    cout << " coalesced constraints=" << print((*i).coalescedConstraints(), *infsm);
    cout << '\n';
    i++;
  }
  
  fsm newfsm = minimizeFSMWithMaximalClasses(*infsm);
  newfsm.printFsm(cout);
  
  return 0;
}
