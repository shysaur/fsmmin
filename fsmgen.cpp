
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <iomanip>
#include <vector>
#include <sstream>
#include <getopt.h>

using namespace std;


string stateName(int i)
{
  string res;
  int c;
  
  if (i < 0)
    return "-";

  while (i >= 0) {
    c = i % ('Z'-'A'+1);
    i = i / ('Z'-'A'+1) - 1;
    res = (char)('A' + c) + res;
  }
  return res;
}


vector<int> fillEntropyVect(int n, int p) {
  vector<int> res(n);
  int s = rand();
  
  srand(s);
  for (int i=0; i<n; i++) {
    if (i % p == 0)
      srand(s);
    res[i] = rand();
  }
  return res;
}


void generateRandomFsm(ostream& s, int nstates, int ninput, int noutput, int undef, int entropy)
{
  vector<int> states_entr = fillEntropyVect(nstates*ninput, entropy);
  vector<int> output_entr = fillEntropyVect(nstates*ninput*noutput, entropy);
  int pad = nstates / ('Z'-'A'+1) + 1;
  
  int se = 0, oe = 0;
  for (int i=0; i<nstates; i++) {
    s << setw(pad) << stateName(i) << " ";
    for (int j=0; j<ninput; j++) {
      s << setw(pad) << stateName(states_entr[se++] % (nstates + undef) - undef) << "/";
      for (int k=0; k<noutput; k++) {
        int t = output_entr[oe++] % (nstates + undef) - undef;
        s << (char)(t < 0 ? '-' : '0'+(t % 2));
      }
      s << " ";
    }
    s << "\n";
  }
}


void usage(char *me) {
  cout << "Random Finite State Machine Generator 1.0.0\n";
  cout << "Copyright (c) 2016 Daniele Cattaneo\n\n";
  cout << "Usage:\n  " << me << " [options]\n\n";
  cout << "Options:\n"
  "  -s --num-states=<num>       Set the number of states to <num>.\n"
  "  -i --num-inputs=<num>       Set the number of possible inputs to <num>.\n"
  "  -o --num-output-bits=<num>  Set the number of output bits to <num>.\n"
  "  -u --undefined-ratio=<num>  Sets an approximate ratio of undefined outputs\n"
  "                              or transitions.\n"
  "  -e --entropy=<num>          The result's randomness increases with <num>.\n"
  "  -h --help                   Display this help and exit.\n\n";
}


int main(int argc, char *argv[])
{
  unsigned nstates=16, ninput=4, noutput=2, undef=1, entropy=7;
  const struct option longopts[] = {
    {"num-states",      required_argument, NULL,     's'},
    {"num-inputs",      required_argument, NULL,     'i'},
    {"num-output-bits", required_argument, NULL,     'o'},
    {"undefined-ratio", required_argument, NULL,     'u'},
    {"entropy",         required_argument, NULL,     'e'},
    {"help",            no_argument,       NULL,     'h'},
    {NULL,              0,                 NULL,       0}
  };
  
  int c;
  while ((c = getopt_long(argc, argv, "s:i:o:u:e:h", longopts, NULL)) != -1) {
    switch (c) {
      case 's':
        try {
          istringstream sstr(optarg);
          sstr >> nstates;
        } catch (exception& e) {
          cout << e.what() << '\n';
          return 1;
        }
        break;
      case 'i':
        try {
          istringstream sstr(optarg);
          sstr >> ninput;
        } catch (exception& e) {
          cout << e.what() << '\n';
          return 1;
        }
        break;
      case 'o':
        try {
          istringstream sstr(optarg);
          sstr >> noutput;
        } catch (exception& e) {
          cout << e.what() << '\n';
          return 1;
        }
        break;
      case 'u':
        try {
          istringstream sstr(optarg);
          sstr >> undef;
        } catch (exception& e) {
          cout << e.what() << '\n';
          return 1;
        }
        break;
      case 'e':
        try {
          istringstream sstr(optarg);
          sstr >> entropy;
        } catch (exception& e) {
          cout << e.what() << '\n';
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
  
  srand(time(NULL));
  nstates = max((unsigned)1, nstates);
  ninput = max((unsigned)1, ninput);
  noutput = max((unsigned)1, noutput);
  undef = max((unsigned)0, undef);
  entropy = max((unsigned)1, entropy);
  generateRandomFsm(cout, nstates, ninput, noutput, undef, entropy);
  
  return 0;
}
