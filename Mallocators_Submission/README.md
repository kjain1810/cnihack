# Instructions to run the code

All of the test data files (district and lab data) need to be be present in this folder. The outpus produced by MIP (using PythonMIP library's CBC solver) is present in the 'MIP 10mins Output' folder.

## Instructions to run MIP solution

To run the Mixed Integer Programming (MIP) solution, we first need to install the following libraries in Python (3.6+):

1. geopy
2. pandas
3. mip (PythonMIP)

To install each of the libary, run the following command on the terminal:

```
> python3 -m pip install [library_name]
```

where `library_name` is the name of the library, like `mip`.

To run the solver in its most basic form, run the following command
```
python3 MIPsolver.py [input_district_file] [input_lab_file] 
```

To vary the time limit of the solver, go to the bottom of the file and change `obj = Solver(300)` to `obj = Solver(X)` where `X` is the number of seconds needed to run the model.

### Giving custom centroids as input 

A specific list of centroids can be given as input, instead of using the default `centroidsv6.txt`. To achieve this, run the following command:

```
python3 MIPsolver.py [input_district_file] [input_lab_file] [centroids_file]
```

Notice that this is an optional argument and may be avoided if the default centroids are sufficient for the purposes.

### Giving pre-formed district-centroid connections to be directly fed to the MIP

By default, for a given centroids list, all the districts are connected to all the centroids. However, there may be redundancy in such a vast model and one may want to prepare special connections to cater these concerns. If you have a list of district centroid connections (for example, see `connections.txt` which specifies the format and has pairing of district with centroids based on `Centroidsv0.txt`), then you can pass that as an argument too to directly load the connections instead of forming them. These district-centroid connections loaded will be directly fed to MIP as input. 

To achieve this purpose, run the following command:

```
python3 MIPsolver.py [input_district_file] [input_lab_file] [centroids_file] [district_centroid_connections_file]
```

Notice that this is also an optional argument. Also note that the `centroids_file` should have all the clusters upon which `district_centroid_connection_file` should be based. 

##### NOTE: While PythonMIP (`mip`) is an open-source and good library to approach optimization problems, it is recommened to use Gurobi if you have a valid installation and a license as Gurobi is much faster than other most other solvers.  

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
