
#include "fsm.h"


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
    do {
      cstate.label += c;
      c = s.get();
    } while (isalnum(c) && c != EOF);
    if (c == EOF)
      throw("Expected label.");
    
    tmpnexts.push_back(vector<string>(0));
    for (int j=0; ; j++) {
      string nextl;
      string out = "";
      
      while (isblank(c) && c != EOF) {
        c = s.get(); 
      }
      if (c == EOF || !(isalnum(c) || c == '-'))
        throw("Expected label.");
      if (c != '-') {
        do {
          nextl += c;
          c = s.get();
        } while (isalnum(c) && c != EOF);
        if (c == EOF)
          throw("Expected label or -.");
      } else {
        nextl = c;
        c = s.get();
      }
      tmpnexts[i].push_back(nextl);

      while (isblank(c) && c != EOF) {
        c = s.get(); 
      } 
      if (c != '/')
        throw("Expected '/'.");
      
      do {
        c = s.get();
      } while (isblank(c) && c != EOF);
      if (c == EOF || !(isdigit(c) || c == '-'))
        throw("Expected a string of numbers and -.");
      do {
        out += c;
        if (c == '-')
          partial = true;
        c = s.get();
      } while ((isdigit(c) || c == '-') && c != EOF);
      if (olen == 0)
        olen = out.length();
      else if (olen != out.length())
        throw("All output strings must be the same length.");
      cstate.out.push_back(out);
      
      while (isblank(c) && c != EOF) {
        c = s.get();
      }
      if (c == '\n' || c == EOF) {
        if (i == 0)
          numnext = j+1;
        else if (j+1 != numnext)
          throw("All states must have the same amount of transitions.");
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
        if (k == states.size())
          throw("Unknown label");
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



