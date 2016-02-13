
#include "fsm.h"
#include <strstream>

using namespace std;


string formatSetOfStates(const set<int>& s, const fsm& m)
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


string formatSetOfClasses(const set< set<int> >& s, const fsm& m)
{
  string out = "(";
  
  for (set< set<int> >::iterator i=s.begin(); i!=s.end(); i++) {
    if (i != s.begin())
      out += ",";
    out += formatSetOfStates(*i, m);
  }
  out += ")";
  return out;
}


fsm::fsm(istream& s)
{
  strstream estr;  
  vector< vector<string> > tmpnexts;
  int c, olen = 0;
  
  partial = false;

  for (int i=0; ; i++) {
    fsmstate cstate;
    
    do {
      c = s.get(); 
    } while ((isblank(c) || c == '\n') && c != EOF);
    if (c == EOF || !isalnum(c))
      break;
    if (c == '\n')
      continue;
    do {
      cstate.label += c;
      c = s.get();
    } while (isalnum(c) && c != EOF);
    if (c == EOF) {
      estr << "Expected label in line " << i+1;
      throw runtime_error(estr.str());
    }
    
    tmpnexts.push_back(vector<string>(0));
    for (int j=0; ; j++) {
      string nextl;
      string out = "";
      
      while (isblank(c) && c != EOF) {
        c = s.get(); 
      }
      if (c == EOF || !(isalnum(c) || c == '-')) {
        estr << "Expected label in line " << i+1;
        throw runtime_error(estr.str());
      }
      if (c != '-') {
        do {
          nextl += c;
          c = s.get();
        } while (isalnum(c) && c != EOF);
        if (c == EOF) {
          estr << "Expected label or - in line " << i+1;
          throw runtime_error(estr.str());
        }
      } else {
        nextl = c;
        c = s.get();
      }
      tmpnexts[i].push_back(nextl);

      while (isblank(c) && c != EOF) {
        c = s.get(); 
      } 
      if (c != '/') {
        estr << "Expected '/' in line " << i+1;
        throw runtime_error(estr.str());
      }
      
      do {
        c = s.get();
      } while (isblank(c) && c != EOF);
      if (c == EOF || !(isdigit(c) || c == '-')) {
        estr << "Expected a string of numbers and - in line " << i+1;
        throw runtime_error(estr.str());
      }
      do {
        out += c;
        if (c == '-')
          partial = true;
        c = s.get();
      } while ((isdigit(c) || c == '-') && c != EOF);
      if (olen == 0)
        olen = out.length();
      else if (olen != out.length()) {
        estr << "Output string of the wrong length in line " << i+1;
        throw runtime_error(estr.str());
      }
      cstate.out.push_back(out);
      
      while (isblank(c) && c != EOF) {
        c = s.get();
      }
      if (c == '\n' || c == EOF) {
        if (i == 0)
          numnext = j+1;
        else if (j+1 != numnext) {
          estr << "Machine state at line " << i+1 << " has too many transitions.";
          throw runtime_error(estr.str());
        }
        break;
      }
    }
    
    states.push_back(cstate);
  }
  
  /* resolve next pointers */
  for (int i=0; i<states.size(); i++) {
    for (int j=0; j<numnext; j++) {
      if (tmpnexts[i][j] == "-") {
        states[i].next.push_back(-1);
        partial = true;
      } else {
        int k;
        for (k=0; k<states.size(); k++) {
          if (tmpnexts[i][j] == states[k].label) {
            states[i].next.push_back(k);
            break;
          }
        }
        if (k == states.size()) {
          estr << "Undefined label \"" << tmpnexts[i][j] << "\"";
          throw runtime_error(estr.str());
        }
      }
    }
  }
}


void fsm::printFsm(ostream& s) const
{
  int i, j;
  
  for (i=0; i<states.size(); i++) {
    s << states[i].label;
    s << ' ';
    for (j=0; j<states[i].next.size(); j++) {
      s << (states[i].next[j] >= 0 ? states[states[i].next[j]].label : "-");
      s << '/';
      s << states[i].out[j];
      s << ' ';
    }
    s << '\n';
  }

}


void fsm::printFsmDot(ostream& s) const
{
  int i, j, k;
  
  s << "digraph G {\n";
  for (i=0; i<states.size(); i++) {
    s << states[i].label << ";\n";
    for (j=0; j<numnext; j++) {
      bool outundef = true;
      
      for (k=0; j<states[i].out[j].length(); j++) {
        if (states[i].out[j][k] != '-') {
          outundef = false;
          break;
        }
      }
      
      if (states[i].next[j] >= 0 || !outundef) {
        s << states[i].label << " -> ";
        s << (states[i].next[j] >= 0 ? states[states[i].next[j]].label : "-");
        s << " [label=\"" << j << '/';
        s << states[i].out[j];
        s << "\"];\n";
      }
    }
  }
  s << "}\n";
}



