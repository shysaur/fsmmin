
#include "fsm.h"
#include "equivgraph.h"
#include "minimize.h"
#include <iostream>
#include <fstream>
#include <utility>
#include <getopt.h>

using namespace std;


void usage(char *me) {
  cout << "Usage:\n  " << me << " [options] [--] [input file]\n\n";
  cout << "Options:\n"
  "  -i --input-fsm          Print the input FSM.\n"
  "  -t --equiv-table        Print the equivalence table of the input FSM.\n"
  "  -c --max-classes        Print the list of all the maximal sets of compatible\n"
  "                          states in the input FSM.\n"
  "     --prime-classes      Print the list of all the primitive sets of compatible\n"
  "                          states in the input FSM.\n"
  "  -r --reduced-fsm=<heur> Print a minimal FSM equivalent to the input FSM,\n"
  "                          using the <heur> method (optional).\n"
  "  -v --verbose            Prints additional logs, when available.\n"
  "  -g --graphviz           Output the equivalent states graph and the FSMs\n"
  "                          in graphviz format (otherwise they are output as\n"
  "                          tables).\n"
  "  -o --output=<file>      Write output to <file>. Logs are always output on\n"
  "                          stdout.\n"
  "  -h --help               Display this help and exit.\n\n"
  "Finite State Machine Reduction Methods:\n"
  "  max [default]   The new states correspond to one maximal compatibility\n"
  "                  class each.\n"
  "  prime           The new states are chosen among all primitive compatibility\n"
  "                  classes using an heuristic.\n\n"
  "Finite State Machine Format:\n"
  "  <machine>       := <state>+\n"
  "  <state>         := <state_label> <transition>+ \\n\n"
  "  <state_label>   := ([A-Z]+|\\-)\n"
  "  <transition>    := <state_label> / <output_symbol>\n"
  "  <output_symbol> := [0-9\\-]+\n\n"
  "Example of FSM:\n"
  "  A  A/0  B/1\n"
  "  B  A/1  A/-\n"
  "(dashes indicates an undefined next state or output symbol)\n\n"
  "If no input file is given, the input FSM is read from standard input.\n"
  "This program is capable of reading Mealy machines only (Moore machines can\n"
  "be expressed as Mealy machines though).\n";
}


int main(int argc, char *argv[]) {
  int graphviz=0, pequiv=0, prfsm=0, pfsm=0, pmax=0, pprime=0, verb=0;
  int rmethod=0;
  struct option longopts[] = {
    {"input-fsm",     no_argument,       NULL,     'i'},
    {"graphviz",      no_argument,       NULL,     'g'},
    {"reduced-fsm",   optional_argument, NULL,     'r'},
    {"equiv-table",   no_argument,       NULL,     't'},
    {"max-classes",   no_argument,       NULL,     'c'},
    {"prime-classes", no_argument,       &pprime,    1},
    {"verbose",       no_argument,       NULL,     'v'},
    {"output",        required_argument, NULL,     'o'},
    {"help",          no_argument,       NULL,     'h'},
    {NULL,            0,                 NULL,       0}
  };
  
  int c;
  ostream *fout = &cout;
  while ((c = getopt_long(argc, argv, "itcr::vgho:", longopts, NULL)) != -1) {
    switch (c) {
      case 0:
        break;
      case 'c':
        pmax = 1;
        break;
      case 'g':
        graphviz = 1;
        break;
      case 'i':
        pfsm = 1;
        break;
      case 'r':
        prfsm = 1;
        if (optarg) {
          if (strcmp("max", optarg) == 0)
            rmethod=0;
          else if (strcmp("prime", optarg) == 0)
            rmethod=1;
          else {
            cout << "Unknown reduction method " << optarg << "\n";
            return 1;
          }
        }
        break;
      case 't':
        pequiv = 1;
        break;
      case 'v':
        verb = 1;
        break;
      case 'o':
        if (optarg) {
          fout = new fstream(optarg, ifstream::out);
          if (!fout || fout->rdstate() != 0) {
            cout << "Unable to open " << optarg << ".\n";
            return 1;
          }
        } else {
          cout << "Output file not specified.\n";
          return 1;
        }
        break;
      case 'h':
        usage(argv[0]);
        return 0;
      default:
        usage(argv[0]);
        return 1;
    }
  }
  argc -= optind;
  
  if (!(pequiv || prfsm || pfsm || pmax || pprime)) {
    usage(argv[0]);
    return 0;
  }
  
  istream *fin;
  if (argc > 0) {
    fin = new fstream(argv[optind], ifstream::in);
    if (!fin || fin->rdstate() != 0) {
      cout << "Unable to open " << argv[optind] << ".\n";
      return 1;
    }
  } else
    fin = &cin;
  
  fsm *infsm;
  try {
    infsm = new fsm(*fin);
  } catch (exception& e) {
    cout << e.what() << '\n';
    return 1;
  }
  
  if (pfsm) {
    if (graphviz)
      infsm->printFsmDot(*fout);
    else
      infsm->printFsm(*fout);
  }
  if (pequiv || pmax || pprime) {
    equivgraph equiv(*infsm);
  
    if (pequiv) {
      if (graphviz)
        equiv.printEquivTableNeato(*fout);
      else
        equiv.printEquivTable(*fout);
    }
  
    if (pmax) {
      set<equivalence> d = equiv.maximalClasses();
      int j = 1;
      for (set<equivalence>::iterator i=d.begin(); i!=d.end(); i++) {
        *fout << "maxclass " << j << " = " << *i;
        if (verb) {
          *fout << " coalesced constraints=";
          *fout << formatSetOfClasses((*i).coalescedConstraints(), *infsm);
        }
        *fout << '\n';
        j++;
      }
    }
    if (pprime) {
      set<equivalence> d = equiv.primitiveClasses();
      int j = 1;
      for (set<equivalence>::iterator i=d.begin(); i!=d.end(); i++) {
        *fout << "primeclass " << j << " = " << *i;
        if (verb) {
          *fout << " coalesced constraints=";
          *fout << formatSetOfClasses((*i).coalescedConstraints(), *infsm);
        }
        *fout << '\n';
        j++;
      }
    }
  }
  
  if (prfsm) {
    fsm newfsm;
    if (rmethod == 0)
      newfsm = minimizedFsmFromMaximalClasses(*infsm);
    else
      newfsm = minimizedFsmFromPrimitiveClasses(*infsm, verb);
    if (graphviz)
      newfsm.printFsmDot(*fout);
    else
      newfsm.printFsm(*fout);
  }

  if (fout != &cout) {
    ((fstream*)fout)->close();
    delete(fout);
  }
  if (fin != &cin) {
    ((fstream*)fin)->close();
    delete(fin);
  }
  return 0;
}
