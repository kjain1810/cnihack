# Instructions to run the code

The test data files to be run must be present in this folder.

## Instructions to run MIP solution

## Instructions to run greedy

The following commands must be run to get the greedy solution with `num` between 1-5 to denote which test file/solution.
```
g++ greedy.cpp -o greedy
./greedy [number] > solution_00{number}.csv
```

## Instructions to run scorer

Assuming all the names of test file and solution are in the given format, run the following command to get the score on a specific test case:
```
python3 scorer.py [fileNumber]
```

Note that `fileNumber` should be between 1-5. For example, 
```
python3 scorer.py 3
```
outputs the score for solution of 3rd test data.
