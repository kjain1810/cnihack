from geopy.distance import geodesic
import pandas as pd
import numpy as np 
import matplotlib.pyplot as plt 

from collections import namedtuple
import math
from ortools.linear_solver import pywraplp
from random import randrange as rr 
import sys, time 

lab_data = pd.read_csv("lab_data_v0.csv", index_col = 0)
district_data = pd.read_csv("districts_data_v0.csv", index_col = 0)

# print(lab_data.info())
# print(district_data.info())


Point = namedtuple("Point", ['x', 'y'])
District = namedtuple("District", ['index', 'demand', 'location'])
Lab = namedtuple("Lab", ['index', 'capacity', 'location', 'type', 'district'])
Centroid = namedtuple("Centroid", ['index', 'labs', 'location'])

districts = []
labs = []
centroids = []
centroidsNearLab = {}
centroidsNearDistrict = {}

totalSamples = 0 

for idx, line in lab_data.iterrows():
	# print(idx, line)
	labs.append(Lab(idx - 1, int(line['capacity']), Point(float(line['lat']), float(line['lon']) ), int(line['lab_type']), int(line['district_id']) - 1))

for idx, line in district_data.iterrows():
	# print(idx, line)
	districts.append(District(idx - 1, int(line['samples']), Point(float(line['lat']), float(line['lon']) ) ))

for lab in labs:
	centroids.append(Centroid(lab.index, [lab.index], lab.location))
	centroidsNearLab[lab.index] = [lab.index]

RADIUS = 80

distPairs = {}
for d in districts:
	totalSamples += d.demand 
	distPairs[d.index] = {}
	centroidsNearDistrict[d.index] = []

	best = -1
	for c in centroids:
		gap = geodesic((d.location.x, d.location.y), (c.location.x, c.location.y)).km 
		distPairs[d.index][c.index] = gap 

		if gap > best :
			best = gap 

		if gap <= RADIUS:
			centroidsNearDistrict[d.index].append(c.index)
			
	print(d.index, best)


#### MIP Modelling! ####

def tryModel():
	n = len(districts)
	m = len(labs)
	c = len(centroids)
	
	model = pywraplp.Solver.CreateSolver('cbc')

	x, p, q, z, o = [{}, {}, {}, {}, {}]
	labsInside = {}
	cnt = 0 		# Keep track of number of constraints fed into the solver (ie. size of input)
	LAMBDA = int(1e9)

	#### -----------------        Add decision variables       -------------- #######
	for i in range(n):
		x[i] = {}
		labsInside[i] = []

		for j in range(m):
			if labs[j].district != i:
				continue 
			x[i][j] = model.IntVar(0.0, labs[j].capacity + 100, 'x[%i, %i]' % (i, j))
			labsInside[i].append(j)
			cnt += 1 

	for i in range(n):
		p[i] = {}
		z[i] = {}

		for j in centroidsNearDistrict[i]:
			p[i][j] = model.IntVar(0, districts[i].demand, 'p[%i, %i]' % (i, j))

			z[i][j] = model.IntVar(0, 1, 'z[%i, %i]' % (i, j))
			# constraint
			model.Add(LAMBDA * z[i][j] >= p[i][j])
			cnt += 2

	for j in range(m):
		o[j] = model.IntVar(0, 100, 'o[%i]' % j)
		cnt += 1 

	for i in range(c):
		q[i] = {}
		for j in centroids[i].labs:
			q[i][j] = model.IntVar(0, labs[j].capacity, 'q[%i, %i]' % (i, j))
			cnt += 1 

	#### -----------------        Add constraints       -------------- #######
	for i in range(n):
		model.Add( sum(x[i][j] for j in labsInside[i]) + sum(p[i][j] for j in centroidsNearDistrict[i]) <= districts[i].demand)
		cnt += 1 

	for j in range(m):
		model.Add( sum(q[k][j] for k in centroidsNearLab[j]) <= labs[j].capacity)

		district_id = labs[j].district 
		model.Add( x[district_id][j] + sum(q[k][j] for k in centroidsNearLab[j]) - 100 <= labs[j].capacity) 
		# model.Add( x[district_id][j] - labs[j].capacity <= o[j])
		model.Add( x[district_id][j] + sum(q[k][j] for k in centroidsNearLab[j]) - labs[j].capacity <= o[j])

		cnt += 3 

	for i in range(c):
		model.Add( sum(q[i][j] for j in centroids[i].labs) == sum(p[j][i] for j in range(n) if i in centroidsNearDistrict[j]) )
		cnt += 1 

	print("Total Samples:", totalSamples)
	print("Model size:", cnt)

	# return 

	#### -----------------        Form final objective function       -------------- #######
	objective_terms = []
	for i in range(n):
		for j in centroidsNearDistrict[i]:
			objective_terms.append(1000 * distPairs[i][j] * z[i][j])
			# objective_terms.append(-LAMBDA * z[i][j])

	bangalore = 0 
	govn = 0
	priv = 0

	for j in range(m):
		objective_terms.append(5000 * o[j])

		if labs[j].type == 1:
			cost = 1600 
			priv += labs[j].capacity 
		else :
			cost = 800 
			govn += labs[j].capacity 

		district_id = labs[j].district 
		objective_terms.append(cost * x[district_id][j])

		for k in centroidsNearLab[j]:
			objective_terms.append(cost * q[k][j])

		if labs[j].district == 4 :
			bangalore += labs[j].capacity 


	for i in range(n):
		objective_terms.append(10000 * districts[i].demand)
		for j in labsInside[i]:
			objective_terms.append(-10000 * x[i][j])
		for j in centroidsNearDistrict[i]:
			objective_terms.append(-10000 * p[i][j])

	model.Minimize(sum(objective_terms))

    # SEC = 1000
    # MIN = 60 * SEC
    # model.SetTimeLimit(5 * MIN)
	status = model.Solve()
	print("Status:", status)

	if status != pywraplp.Solver.OPTIMAL and status != pywraplp.Solver.FEASIBLE :
		print("No feasible solution found!") 

	newCost = model.Objective().Value()
	print("Optimal value of objective function:", newCost)

	# for i in range(n):
	# 	for j in range(c):
	# 		newCost += LAMBDA * z[i][j].solution_value() // for earlier (wrong) model

	print("Best cost:", newCost)


	#### -----------------        Extract all the info from the solution found       -------------- #######
	for j in range(m):
		tested = 0 
		district_id = labs[j].district 
		tested += x[district_id][j].solution_value()

		for k in centroidsNearLab[j]:
			tested += q[k][j].solution_value()

		print("Lab:", j, "\tType:", labs[j].type, "\tCapacity =", labs[j].capacity, "\tTested =", tested, "\tSamples overloaded =", o[j].solution_value())
	
	print()
	for i in range(n):
		sent = 0
		for j in labsInside[i]:
			sent += x[i][j].solution_value()

		print("District headquater", i, "\t distributed", sent, "\tsamples out of", districts[i].demand)

		for j in centroidsNearDistrict[i]:
			sent += p[i][j].solution_value()


		backlog = districts[i].demand - sent 
		if backlog > 0:
			print("Backlog at district", i, "=", backlog, " with demand = ", districts[i].demand)

	print("\n\n")
	for i in range(n):
		for j in centroidsNearDistrict[i]:
			if j in labsInside[i] :
				continue 
			if j in z[i]:
				if z[i][j].solution_value() > 0:
					print("Disrict", i, "\t has sent", p[i][j].solution_value(), "\t samples to external lab",j,"\t with distance =", distPairs[i][j])

	print("\n\n")
	for i in range(c):
		inc = sum(p[j][i].solution_value() for j in range(n) if i in centroidsNearDistrict[j])
		out = sum(q[i][j].solution_value() for j in centroids[i].labs)
		district_id = labs[i].district
		default = x[district_id][i].solution_value()
		print("At centroid", i, "\t incoming =", inc, "\t outgoing =", out, "\t capacity=", labs[i].capacity, " in district", district_id)

	print("Total Government Capacity =", govn, "Total private labs' capacity =", priv)
	# print(bangalore)


# pipe 
sys.stdout = open("radial_output" + str(RADIUS) + ".out", "w")
tryModel()