# Matbuilder [[Paulin2022]](https://projet.liris.cnrs.fr/matbuilder/matbuilder2022.pdf) OSSolver

Matbuilder implementation that supports any solvers, including open source ones. 

It supports (for now):

* GLPK (Open source)
* CPLEX (Proprietary, academic licenses availables)

Based on the code : https://github.com/loispaulin/matbuilder

## Building the project

Depending on the choosen solver, the building command is : 

```bash 
mkdir build;
cd build;
cmake -DCMAKE_BUILD_TYPE=Release -DCPLEX=ON -DGLPK=ON ..
make
```

If, for example, CPLEX is not desired, the option `-DCPLEX=ON` shoudl be ommitted.

## Launching optimisation

Each backend builds a different executable. For now, they have the same command line
interface. It is the same interface as [https://github.com/loispaulin/matbuilder](https://github.com/loispaulin/matbuilder). 

```bash
Matbuilder Solver (GLPK)
Usage: ./matbuilder_glpk [OPTIONS]

Options:
  -h,--help                   Print this help message and exit
  -i TEXT REQUIRED            Input file name
  -o TEXT                     Output file name
  -m INT                      Override file matrix size
  -n,--nbTrials INT           Number of tentative (def:50)
  --nbBacktrack INT           Number of backtracks before starting again (def:15)
  -s INT                      Override file number of dimensions
  --check                     Check properties (unit test)
  -b INT                      Override matrices basis
  --threads INT               Number of threads to use (def: all avalaible)
  --tolerance FLOAT           Error tolerance on objective value (def: 0.01)
  -t,--timeout FLOAT          Maximum time for each cplex solve (def: 10^10 s)
  --seed INT                  Program seed
  --no-seed                   Disables seed objective in optimizer
  --header                    Writes profile as comments at the beginning of matrix file
```

The option '--check' is not yet supported...

Profiles can be found here : [https://github.com/loispaulin/matbuilder](https://github.com/loispaulin/matbuilder). 

## Expand tool

If the solver you want to use does not have a C/C++ interface, you can use the expand tool. It
can be used to output the implied linear program to MPS or LP file. 

```bash
Matbuilder ILP export
Usage: ./matbuilder_expand [OPTIONS]

Options:
  -h,--help                   Print this help message and exit
  -i TEXT REQUIRED            Input file name
  -m TEXT REQUIRED            Matrices file name
  -o TEXT                     Output file name
  --seed INT                  Program seed
  --no-seed                   Disables seed objective in optimizer
  --format TEXT=LP            Output format (LP/MPS) 
```

The -m file expect the already solved matrices, without comments. The unknown
coefficient are the variables named x_{0} to x_{m * s} (in the order of dimension, 
from first to last row).

The typical usage of the tool is to start by feeding an empty file. 
Then solve the first ILP, then complete the matrices, and start again. 
For this reason, the --seed parameter should not changed between successive calls, 
the underlying PRNG is advanced automatically. 