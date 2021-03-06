
#include "fsm.h"
#include <sstream>
#include <stdexcept>
#include <iomanip>

using namespace std;


string fsm::formatSetOfStates(const set<int>& s) const
{
  string out = "(";
  
  for (set<int>::iterator i=s.begin(); i!=s.end(); i++) {
    if (i != s.begin())
      out += ",";
    out += states[*i].label;
  }
  out += ")";
  return out;
}


string fsm::formatSetOfClasses(const set< set<int> >& s) const
{
  string out = "(";
  
  for (set< set<int> >::iterator i=s.begin(); i!=s.end(); i++) {
    if (i != s.begin())
      out += ",";
    out += formatSetOfStates(*i);
  }
  out += ")";
  return out;
}


fsm::fsm(istream& s)
{
  stringstream estr;  
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
  if (states.size() == 0) {
	estr << "A valid machine has at least one state.";
	throw runtime_error(estr.str());
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
  int maxl = states[0].label.length();
  for (int i=0; i<states.size(); i++)
    if (states[i].label.length() > maxl)
      maxl = states[i].label.length();
  
  for (int i=0; i<states.size(); i++) {
    s << setw(maxl) << states[i].label << ' ';
    for (int j=0; j<states[i].next.size(); j++) {
      s << setw(maxl);
      s << (states[i].next[j] >= 0 ? states[states[i].next[j]].label : "-");
      s << '/' << states[i].out[j] << ' ';
    }
    s << '\n';
  }
}


void fsm::printFsmDot(ostream& s) const
{
  s << "digraph G {\n";
  for (int i=0; i<states.size(); i++) {
    s << states[i].label << ";\n";
    for (int j=0; j<numnext; j++) {
      bool outundef = true;
      
      for (int k=0; k<states[i].out[j].length(); k++) {
        if (states[i].out[j][k] != '-') {
          outundef = false;
          break;
        }
      }
      
      if (states[i].next[j] >= 0 || !outundef) {
        string nextlabel;
        
        if (states[i].next[j] >= 0)
          nextlabel = states[states[i].next[j]].label;
        else {
          stringstream t;
          t << "undefined_" << i << "_" << j;
          nextlabel = t.str();
          s << nextlabel << " [style=\"invisible\"];\n";
        }
        
        s << states[i].label << " -> " << nextlabel;
        s << " [label=\"" << j << '/' << states[i].out[j] << "\"];\n";
      }
    }
  }
  s << "}\n";
}



