# Instructions to run the code

To run the file, we first need to install the following libraries in Python (3.6+):

1. geopy
2. pandas
3. mip (PythonMIP)

To install each of the libary, run the following command on the terminal:

```
> python3 -m pip install [libraryName]
```

where `libraryName` is the name of the library, like `mip`.

The test data files to be run must be present in this folder.

## Instructions to run MIP solution

To run the solver in its most basic form, run the followig command
```
python3 MIPsolver.py [input_district_file] [input_lab_file] 
```

To vary the time limit of the solver, go to the bottom of the file and change `obj = Solver(300)` to `obj = Solver(X)` where `X` is the number of seconds needed to run the model.

### Giving custom centroids as input 



### Giving pre-formed district-centroid connections to be directly fed to the MIP

#### NOTE: While the currently 

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
