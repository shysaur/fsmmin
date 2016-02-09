
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <getopt.h>


#define MAX_NEXT (4)
#define MAX_STATES (16)


typedef struct {
  int next[MAX_NEXT];
  unsigned int out[MAX_NEXT];
  char label;
} fsmstate_t;

typedef struct {
  int numstates;
  int numnext;
  fsmstate_t states[MAX_STATES];
} fsm_t;


fsm_t *readFSM(FILE *fp) {
  fsm_t *res;
  fsmstate_t *this;
  char tmpnexts[MAX_STATES][MAX_NEXT];
  int c;
  int i, j, k;
  
  res = calloc(1, sizeof(fsm_t));

  for (i=0; ; i++) {
    this = res->states+i;
    
    do {
      c = getc(fp); 
    } while ((isblank(c) || c == '\n') && c != EOF);
    if (c == EOF)
      break;
      
    if (c < 'A' || 'Z' < c) {
      printf("Labels must be upper-case letters.\n");
      goto fail;
    }
    this->label = c;
    
    do {
      c = getc(fp); 
    } while (isblank(c) && c != EOF);
    
    for (j=0; j<MAX_NEXT; j++) {
      if (c == EOF || c < 'A' || 'Z' < c) {
        printf("Expected label.\n");
        goto fail;
      }
      tmpnexts[i][j] = c;

      do {
        c = getc(fp); 
      } while (isblank(c) && c != EOF);
      if (c != '/') {
        printf("Expected '/'.\n");
        goto fail;
      }
      
      do {
        c = getc(fp); 
      } while (isblank(c) && c != EOF);
      ungetc(c, fp);
      if (fscanf(fp, "%d", &(this->out[j])) < 1) {
        printf("Expected number.\n");
      }
      
      do {
        c = getc(fp);
      } while (isblank(c) && c != EOF);
      if (c == '\n' || c == EOF) {
        if (i == 0)
          res->numnext = j+1;
        else if (j+1 != res->numnext) {
          printf("All states must have the same amount of transitions.\n");
          goto fail;
        }
        break;
      } else if (j == MAX_NEXT-1) {
        printf("More than %d transitions not allowed.\n", MAX_NEXT);
        goto fail;
      }
    }
    res->numstates++;
  }
  
  /* resolve next pointers */
  for (i=0; i<res->numstates; i++) {
    for (j=0; j<res->numnext; j++) {
      for (k=0; k<res->numstates; k++) {
        if (tmpnexts[i][j] == res->states[k].label) {
          res->states[i].next[j] = k;
          break;
        }
      }
      if (k == res->numstates) {
        printf("Unknown label %c\n", tmpnexts[i][j]);
        goto fail;
      }
    }
  }
  
  return res;
fail:
  free(res);
  return NULL;
}


void printFSM(fsm_t *fsm) {
  int i, j;
  fsmstate_t *this;
  
  for (i=0; i<fsm->numstates; i++) {
    this = fsm->states+i;
    putchar(this->label);
    putchar(' ');
    for (j=0; j<fsm->numnext; j++) {
      printf("%c/%d ", fsm->states[this->next[j]].label, this->out[j]);
    }
    putchar('\n');
  }
}


void printFSMDot(fsm_t *fsm) {
  int i, j;
  fsmstate_t *this;
  
  printf("digraph G {\n");
  for (i=0; i<fsm->numstates; i++) {
    this = fsm->states+i;
    printf("%c;\n", this->label);
    for (j=0; j<fsm->numnext; j++) {
      printf("%c -> %c [label=\"%d/%d\"];\n", this->label, fsm->states[this->next[j]].label, j, this->out[j]);
    }
  }
  printf("}\n");
}


/* O ok; X no; W loop breaker; \0 not seen yet */
char paullUnger_(fsm_t *fsm, char equiv[][MAX_STATES], int s0, int s1) {
  int i;
  
  if (s0 == s1) {
    equiv[s0][s0] = 'O';
    return 'O';
  }
  if (equiv[s0][s1] == 'W') {
    equiv[s0][s1] = 'O';
    equiv[s1][s0] = 'O';
    return 'O';
  }
  if (equiv[s0][s1] != '\0') {
    return equiv[s0][s1];
  }
  
  equiv[s0][s1] = equiv[s1][s0] = 'W';
  for (i=0; i<fsm->numnext; i++) {
    if (fsm->states[s0].out[i] != fsm->states[s1].out[i])
      return equiv[s0][s1] = equiv[s1][s0] = 'X';
  }
  for (i=0; i<fsm->numnext; i++) {
    if (paullUnger_(fsm, equiv, fsm->states[s0].next[i], fsm->states[s1].next[i]) != 'O')
      return equiv[s0][s1] = equiv[s1][s0] = 'X';
  }
  return equiv[s0][s1] = equiv[s1][s0] = 'O';
} 


void paullUnger(fsm_t *fsm, char equiv[][MAX_STATES]) {
  int i, j;
  
  memset(equiv, 0, MAX_STATES*MAX_STATES*sizeof(char));
  for (i=0; i<fsm->numstates; i++)
    equiv[i][i] = 'O';
  for (i=0; i<fsm->numstates; i++) {
    for (j=i+1; j<fsm->numstates; j++) {
      paullUnger_(fsm, equiv, i, j);
    }
  }
}


void printEquivTable(fsm_t *fsm, char equiv[][MAX_STATES]) {
  int i, j;
  
  for (i=1; i<fsm->numstates; i++) {
    putchar(fsm->states[i].label);
    putchar(' ');
    for (j=0; j<i; j++) {
      putchar(equiv[i][j]);
    }
    putchar('\n');
  }
  putchar(' ');
  putchar(' ');
  for (i=0; i<fsm->numstates-1; i++) {
    putchar(fsm->states[i].label);
  }
  putchar('\n');
}


void printEquivTableNeato(fsm_t *fsm, char equiv[][MAX_STATES]) {
  int i, j;
  
  printf("graph G {\n");
  for (i=1; i<fsm->numstates; i++) {
    for (j=0; j<i; j++) {
      if (equiv[i][j] == 'O')
        printf("%c -- %c;\n", fsm->states[i].label, fsm->states[j].label);
    }
  }
  printf("}\n");
}


void makeEquivClasses(fsm_t *fsm, char equiv[][MAX_STATES], int classes[][MAX_STATES+1]) {
  int covered[MAX_STATES];
  int i, j, k, l;
  
  for (i=0; i<fsm->numstates; i++)
    covered[i] = 0;
  for (i=0; i<MAX_STATES; i++)
    classes[i][0] = -1;
    
  k = 0;
  for (i=0; i<fsm->numstates; i++) {
    if (covered[i])
      continue;
      
    l = 0;
    classes[k][l++] = i;
    covered[i] = 1;
    for (j=i; j<fsm->numstates; j++) {
      if (covered[j])
        continue;
      if (equiv[i][j] == 'O') {
        covered[j] = 1;
        classes[k][l++] = j;
      }
    }
    classes[k][l] = -1;
    k++;
  }
  classes[k][0] = -1;
}


void printEquivClasses(fsm_t *fsm, int classes[][MAX_STATES+1]) {
  int i, j;
  char l;
  
  l = 'a';
  for (i=0; classes[i][0] != -1; i++) {
    printf("%c = ", l);
    for (j=0; classes[i][j] != -1; j++) {
      putchar(fsm->states[classes[i][j]].label);
    }
    putchar('\n');
    l++;
  }
}


fsm_t *reduceFSM(fsm_t *old, int classes[][MAX_STATES+1]) {
  int oldtonew[MAX_STATES];
  fsm_t *new;
  int i, j, oldi;
  
  new = calloc(1, sizeof(fsm_t));
  new->numnext = old->numnext;
  
  for (i=0; classes[i][0] != -1; i++) {
    for (j=0; classes[i][j] != -1; j++) {
      oldtonew[classes[i][j]] = i;
    }
  }
  
  for (i=0; classes[i][0] != -1; i++) {
    new->numstates++;
    new->states[i].label = 'a'+i;
    oldi = classes[i][0];
    for (j=0; j<new->numnext; j++)
      new->states[i].out[j] = old->states[oldi].out[j];
    for (j=0; j<new->numnext; j++)
      new->states[i].next[j] = oldtonew[old->states[oldi].next[j]];
  }
  return new;
}


void usage(char *me) {
  printf("Usage:\n  %s [options] [--] [input file]\n\n", me);
  puts(
  "Options:\n"
  "  -i --input-fsm      Print the input FSM.\n"
  "  -t --equiv-table    Print the equivalence table of the input FSM.\n"
  "  -c --equiv-classes  Print the list of equivalent states in the input FSM.\n"
  "  -r --reduced-fsm    Print a minimal FSM equivalent to the input FSM.\n"
  "  -g --graphviz       Output the equivalent states list and the FSMs as\n"
  "                      graphs, in graphviz format.\n"
  "  -h --help           Display this help and exit.\n\n"
  "Finite State Machine Format:\n"
  "  <machine>       := <state><machine>|<state>\n"
  "  <state>         := <state_label> <transitions> \\n\n"
  "  <state_label>   := A|B|C| ... |X|Y|Z\n"
  "  <transitions>   := <transition>|<transitions> <transition>\n"
  "  <transition>    := <state_label> / <output_symbol>\n"
  "  <output_symbol> := (any decimal number)\n\n"
  "Example of FSM:\n"
  "  A  A/0  B/1\n"
  "  B  A/1  A/0\n\n"
  "If no input file is given, the input FSM is read from standard input.\n"
  "This program is capable of reading Mealy machines only (Moore machines can\n"
  "be expressed as Mealy machines though).\n"
  );
}


int main(int argc, char *argv[]) {
  char equiv[MAX_STATES][MAX_STATES];
  int classes[MAX_STATES+1][MAX_STATES+1];
  fsm_t *fsm, *rfsm;
  FILE *fpin;
  int graphviz=0, pequiv=0, pclasses=0, prfsm=0, pfsm=0, c;
  struct option longopts[] = {
    {"equiv-classes", no_argument,      NULL,  'c'},
    {"input-fsm",     no_argument,      NULL,  'i'},
    {"graphviz",      no_argument,      NULL,  'g'},
    {"reduced-fsm",   no_argument,      NULL,  'r'},
    {"equiv-table",   no_argument,      NULL,  't'},
    {"help",          no_argument,      NULL,  'h'},
    {NULL,            0,                NULL,  0}
  };
  
  while ((c = getopt_long(argc, argv, "cgirth", longopts, NULL)) != -1) {
    switch (c) {
      case 'c':
        pclasses = 1;
        break;
      case 'g':
        graphviz = 1;
        break;
      case 'i':
        pfsm = 1;
        break;
      case 'r':
        prfsm = 1;
        break;
      case 't':
        pequiv = 1;
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
  if (argc > 0) {
    fpin = fopen(argv[optind], "r");
    if (!fpin) {
      printf("Unable to open %s.\n", argv[optind]);
      return 1;
    }
  } else
    fpin = stdin;
  
  if (pequiv || pclasses || prfsm || pfsm) {
    fsm = readFSM(fpin);
    if (!fsm)
      return 1;
      
    if (pfsm) {
      if (graphviz)
        printFSMDot(fsm);
      else
        printFSM(fsm);
    }
    if (pequiv || pclasses || prfsm) {
      paullUnger(fsm, equiv);
      if (pequiv) {
        if (graphviz)
          printEquivTableNeato(fsm, equiv);
        else
          printEquivTable(fsm, equiv);
      }
      if (pclasses || prfsm) {
        makeEquivClasses(fsm, equiv, classes);
        if (pclasses)
          printEquivClasses(fsm, classes);
        if (prfsm) {
          rfsm = reduceFSM(fsm, classes);
          if (graphviz)
            printFSMDot(rfsm);
          else
            printFSM(rfsm);
        }
      }
    }
  } else
    usage(argv[0]);
  return 0;
}

