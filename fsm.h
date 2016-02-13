
#include <iostream>
#include <vector>
#include <string>
#include <set>

using namespace std;


#ifndef FSM_H
#define FSM_H


class fsmstate {
public:
  vector <int> next;
  vector <string> out;
  string label;
};


class fsm {
public:
  vector <fsmstate> states;
  bool partial;
  int numnext;
  fsm(istream& s);
  fsm(void) {}
  void printFsm(ostream& s) const;
  void printFsmDot(ostream& s) const;
};


string formatSetOfStates(const set<int>& s, const fsm& m);
string formatSetOfClasses(const set< set<int> >& s, const fsm& m);


#endif
