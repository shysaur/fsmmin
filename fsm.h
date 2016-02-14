
#include <iostream>
#include <cstdio>
#include <vector>
#include <string>
#include <set>


#ifndef FSM_H
#define FSM_H


class fsmstate {
public:
  std::vector <int> next;
  std::vector <std::string> out;
  std::string label;
};


class fsm {
public:
  std::vector <fsmstate> states;
  bool partial;
  int numnext;
  fsm(std::istream& s);
  fsm(void) {}
  void printFsm(std::ostream& s) const;
  void printFsmDot(std::ostream& s) const;
  std::string formatSetOfStates(const std::set<int>& s) const;
  std::string formatSetOfClasses(const std::set< std::set<int> >& s) const;
};


#endif
