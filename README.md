# ece6740_logic_synth_project

This project implements a branch-and-bound solver for the binate covering problem
(BCP) for two-level logic synthesis. The solver reads a covering problem in a
simple text format, builds the corresponding ternary constraint matrix using
`1`, `0`, and don't-care entries, applies matrix reductions, and recursively
searches for a minimum-cost solution.

## Build

Run the following from this directory:

```bash
make
```

This produces the executable `main.out`.

## Run

Run the solver on one of the example inputs:

```bash
./main.out examples/f.txt
```

You can also use the convenience target:

```bash
make run
```

To run the solver on all example inputs in the `examples/` directory:

```bash
make test
```

## Input Format

Each input file contains one clause per line. Literals are separated by spaces.

- `x3` means variable `x3` appears in positive form
- `x3'` means variable `x3` appears in complemented form

Example:

```text
x1 x2
x1' x3
x2 x4
```

## Project Layout

- `main.cpp`, `main.h`: entry point, recursive BCP driver, and temporary
  helper functions used during development
- `SolutionState.cpp`, `SolutionState.h`: matrix representation, reductions,
  lower-bound heuristic, assignment logic, and branching-variable selection
- `Val.h`: ternary matrix value representation
- `examples/`: solver input examples and benchmark-style cases
- `tests/`: targeted manual test cases for individual heuristics such as
  `choose_var()`
- `blif_and_working_format_coversion/`: BLIF conversion and helper scripts

## Notes on Testing

The repository currently uses lightweight manual helper functions in `main.cpp`
for testing individual algorithm components. For example, `test_choose_var()`
can be temporarily called from `main()` to verify the branching heuristic on the
files in `tests/`.
