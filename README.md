# exbar

The implementation of the algorithm `exbar` based on the article [__Faster Algorithms for
Finding Minimal Consistent DFAs__](https://pdfs.semanticscholar.org/f74c/5462cec67439490bf73f652ecd7d5f3f2679.pdf).

The application should be compiled without any problems with G++ or MinGW. Depending on the arguments, the program will be display full or simplified logs.

Beware, this implementation works only on integers (characters indices).

## Compilation

The compilation with full logs:
```
g++ exbar.cpp libraries/easylogging/easylogging++.cc -std=c++11
```

The compilation with simplified logs:
```
g++ exbar.cpp libraries/easylogging/easylogging++.cc -std=c++11 -DELPP_DISABLE_DEBUG_LOGS
```

## Executing

```
exbar --file {input file path} [--verify] [--limit 1]
```
If you add the `--verify` parameter, then generated DFA will be checked with the input file's examples and counterexamples.
The `--limit` parameter sets the starting limit for the maximum number of red nodes, the default value is 1.

## Input file format

```
{The number of examples} {The size of dictionary}
{example: {is accepted} {the number of characters in a word} {indices separated with space}}
```

Example 1:
```
5 2
1 1 1
1 4 1 0 1 1
1 2 0 0
0 3 1 0 0
0 1 0
```

Example 2:
```
8 2
1 11 0 0 1 0 0 0 0 0 1 0 0
0 15 0 0 0 0 0 1 1 1 0 1 0 0 0 1 1
1 12 1 1 0 0 1 1 0 1 0 1 0 0
0 14 0 1 1 1 1 1 0 1 1 1 0 0 1 0
0 14 0 0 0 1 0 0 0 1 1 0 0 0 1 1
1 15 0 1 0 0 0 1 0 1 1 1 1 0 0 0 1
1 15 0 0 0 1 0 1 1 0 1 1 0 0 0 1 0
0 15 1 1 0 0 1 1 0 1 1 1 1 0 0 1 0
```

## Output file format

The application will generate a minified DFA as two files:  `dfa.txt` and `dfa.yaml`. In states transisions, there are lists of succeeding states indices, where the value -1 means there is no transition.

### First file (txt)

```
{the numberf of states}
{the size of alphabet}
{the number of accept states} {accept states}
{the number of reject states} {reject states}
{transitions for the state 0 in order of the alphabet's characters indices}
{transitions for the state 1 in order of the alphabet's characters indices}
...
```

Example:
```
3
2
2 0 1
1 2
1 1
0 2
2 0
```

### Second file (yaml)

Example:
```
number of states: 3
size of alphabet: 2
accepting states: [0, 1]
rejecting states: [2]
initial state: 0
transitions:
- [0, 1, 1]
- [1, 0, 2]
- [2, 2, 0]
```

## Quickstart

```
g++ exbar.cpp libraries/easylogging/easylogging++.cc -std=c++11 -DELPP_DISABLE_DEBUG_LOGS && ./a.out --file test/sample1.txt
```
