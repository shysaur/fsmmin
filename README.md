# Finite State Machine Minimizer

This is a small tool for analyzing complete or incomplete [Mealy finite state 
machines](https://en.wikipedia.org/wiki/Finite-state_machine).

It can:

 - generate a [Graphviz](http://www.graphviz.org)-format graph of the machine
   from a state transition table;
 - identify any equivalence or compatibility between states of a machine using
   [Moore's algorithm](https://en.wikipedia.org/wiki/Moore_reduction_procedure);
 - produce an (hopefully) smaller machine equivalent to the one given in
   input.
   
A separate tool (`fsmgen`) is included, which generates random finite state
machines. The machines it generates tend to be completely useless, but they
can be used to test `fsmmin`.

## Compiling

`fsmmin` and `fsmgen` can be compiled with [gcc](https://gcc.gnu.org/) or 
[clang](http://clang.llvm.org/) on Linux and OS X, or with 
[mingw-w64](http://mingw-w64.org/doku.php) on Windows. Just open a shell
and run `make`.

If you want to compile with Visual Studio on Windows, you'll have to make
the Visual Studio project yourself. It should work, but you'll need some
kind of implementation for `getopt_long`.

## Q & A

### How did the "prime" heuristic get to this result?

Use the `--verbose` option and see for yourself.

If you don't like the result, experiment with the `--prime-weights` option. For
example, by specifying `--prime-weights=1,2,1` when minimizing 
`test-machines/m9.txt` you'll get a much better result.

### This program is slow

If your input brings `fsmmin` to the knees, I bet you've produced it with
`fsmgen -e999 -s20 -u999` or something like that. Since almost the whole
machine is undefined, basically every state is compatible with all others;
this makes [Bron-Kerbosch's 
algorithm](https://en.wikipedia.org/wiki/Bron–Kerbosch_algorithm) take an
inordinate amount of time, because it runs in exponential time.

Also note that generating all primitive compatibility classes is also very
slow, because the algorithm is basically brute-force.

Try compiling it with `CXXFLAGS=-O3` and see if it improves. Probably, 
it will not. Otherwise you can wait a couple of years or kill the process
(trust me, the result is not going to be very interesting).

### "Incomplete" machine? What's that?

"Incomplete finite state machine" doesn't mean "*nondeterministic* finite 
state machine". "Incomplete" just means that some output or transition is not 
defined, and thus can be replaced with any other possible transition.

Incomplete finite state machines are used in hardware design to optimize away
those inputs that are believed to be impossible for some reason.

We're talking about another kind of nondeterminism here: since the machine
can go to any other state when performing an undefined transition, it's not
an "useful" kind of nondeterminism. If you're still confused, go read
[Wikipedia on nondeterministic finite state 
machines](https://en.wikipedia.org/wiki/Nondeterministic_finite_automaton).

