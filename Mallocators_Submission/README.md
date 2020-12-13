# Instructions to run the code

The test data files to be run must be present in this folder.

## Instructions to run MIP solution

To run the solver in its most basic form, 
```
python3 MIPsolver.py [input_district_file] [input_lab_file] 
```
### Giving custom centroids as input 

### Giving pre-formed district-centroid connections as input 

## Instructions to run greedy

The following commands must be run to get the greedy solution with `number` between 1-5 to denote which test file/solution.
```
g++ greedy.cpp -o greedy
./greedy [number] > solution_00{number}.csv
```

## Instructions to run scorer

Assuming all the names of test file and solution are in the given format, run the following command to get the score on a specific test case:
```
python3 scorer.py [number]
```

Note that `number` should be between 1-5. For example, 
```
python3 scorer.py 3
```
outputs the score for solution of 3rd test data.
