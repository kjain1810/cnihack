# Instructions to run the code

The test data files to be run must be present in this folder.

## Instructions to run MIP solution

## Instructions to run greedy

The varable `num` in line #198 of greedy.cpp must be changed to the required test file number.

The following commands must be run to get the greedy solution.
```
g++ greedy.cpp -o greedy
./greedy > solution_00{num}.csv
```

## Instructions to run scorer

The varable `num` in line #18 of scorer.py must be changed to the required test file number.

```
python3 scorer.py
```
