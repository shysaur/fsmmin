
#include "minimize.h"
#include <set>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <sstream>
#include <climits>

using namespace std;


string stateName(int i)
{
  string res;
  int c;

  while (i >= 0) {
    c = i % ('z'-'a'+1);
    i = i / ('z'-'a'+1) - 1;
    res = (char)('a' + c) + res;
  }
  return res;
}


fsm buildFsmWithClasses(fsm& ifsm, vector< set<int> >& selCl) 
{
  fsm newfsm;
  newfsm.numnext = ifsm.numnext;
  newfsm.partial = ifsm.partial;
  
  for (int i=0; i<selCl.size(); i++) {
    fsmstate state;
    state.label = stateName(i);
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
        if (k == selCl.size()) {
          stringstream estr;
          estr << "Can't find class corresponding to " << ifsm.formatSetOfStates(go);
          estr << " for input " << j << " of state " << ifsm.formatSetOfStates(selCl[i]);
          throw runtime_error(estr.str());
        }
      } else
        state.next.push_back(-1);
    }
    newfsm.states.push_back(state);
  }
  return newfsm;
}


int benefit(int *cov, int *con, int *sol, set<int>& s, set< set<int> >& c, set<int> cs, set< set<int> > sc)
{
  int tcov=0, tcon=0, tsol=0;
  
  /* +1 for each newly covered state */
  for (set<int>::iterator i=s.begin(); i!=s.end(); i++) {
    if (cs.find(*i) == cs.end())
      tcov++;
  }
  
  /* -1 for each new constraint */
  for (set< set<int> >::iterator i=c.begin(); i!=c.end(); i++) {
    if (sc.find(*i) == sc.end())
      tcon--;
  }
  
  /* +1 for each newly covered constraint */
  for (set< set<int> >::iterator i=sc.begin(); i!=sc.end(); i++) {
    if (includes(s.begin(), s.end(), (*i).begin(), (*i).end()))
      tsol++;
  }
  
  if (cov) *cov = tcov;
  if (con) *con = tcon;
  if (sol) *sol = tsol;
  return tcov+tcon+tsol;
}


fsm minimizedFsmFromPrimitiveClasses(equivgraph &equiv, bool verbose)
{
  fsm& ifsm = equiv.machine;
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
    int maxi = -1;
    int maxb = INT_MIN, maxcov = INT_MIN, maxcon = INT_MIN, maxsol = INT_MIN;

    for (int i=0; i<availCl.size(); i++) {
      int cov, con, sol;
      int b = benefit(&cov, &con, &sol, availCl[i], availConstr[i], coveredStates, selConstr);
      
      if (verbose) {
        cout << "benefit for class " << ifsm.formatSetOfStates(availCl[i]) << " = ";
        cout << cov;
        if (con >= 0)
          cout << '+';
        cout << con << '+' << sol << " = " << b;
      }
      
      /* Ignore those states that bring nothing to the table. It's impossible
       * to ignore all the states because, if that could happen, every con-
       * straint and every state would be already covered, thus the loop must
       * have already ended (assuming no bugs elsewhere). */
      if (cov == 0 && sol == 0) {
        if (verbose)
          cout << " (ignored)\n";
        continue;
      }
      cout << '\n';
        
      /* Choose one of the states that introduce less constraints among:
       * {those that solve the most constraints among: {those that cover the 
       * most states among: {those that have the greatest score}}}. */
      if (b > maxb) {
        maxb = b; maxcov = cov; maxsol = sol; maxcon = con;
        maxi = i;
      } else if (b == maxb) {
        if (cov > maxcov) {
          maxcov = cov; maxsol = sol; maxcon = con;
          maxi = i;
        } else if (cov == maxcov) {
          if (sol > maxsol) {
            maxsol = sol; maxcon = con;
            maxi = i;
          } else if (sol == maxsol) {
            if (con >= maxcon) {
              maxcon = con;
              maxi = i;
            }
          }
        }
      }
    }
    if (maxi == -1) {
      stringstream serr;
      serr << "The primitive class generation algorithm has a bug because the ";
      serr << "sets it generated didn't cover the whole machine.";
      throw runtime_error(serr.str());
    }
    if (verbose)
      cout << "selecting class " << ifsm.formatSetOfStates(availCl[maxi]) << "\n";
    
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


fsm minimizedFsmFromMaximalClasses(equivgraph &equiv)
{
  fsm& ifsm = equiv.machine;
  set<equivalence> tc = equiv.maximalClasses();
  vector< set<int> > selCl;
  
  for (set<equivalence>::iterator i=tc.begin(); i!=tc.end(); i++)
    selCl.push_back((*i).states);
    
  return buildFsmWithClasses(ifsm, selCl);
}

