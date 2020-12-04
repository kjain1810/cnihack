from geopy.distance import geodesic
import pandas as pd
import numpy as np 

from collections import namedtuple
import math
from random import randrange as rr 
import sys, time 

import gurobipy as gp
from gurobipy import GRB

lab_data = pd.read_csv("lab_data_v0.csv", index_col = 0)
district_data = pd.read_csv("districts_data_v0.csv", index_col = 0)

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
	labs.append(Lab(idx - 1, int(line['capacity']), Point(float(line['lat']), float(line['lon']) ), int(line['lab_type']), int(line['district_id']) - 1))

for idx, line in district_data.iterrows():
	districts.append(District(idx - 1, int(line['samples'] * 1.2), Point(float(line['lat']), float(line['lon']) ) ))

for lab in labs:
	centroids.append(Centroid(lab.index, [lab.index], lab.location))
	centroidsNearLab[lab.index] = [lab.index]


# Only those centroid pairs are made whose distance from lab is <= RADIUS 
RADIUS = 100

# Form district-centroid distance pairs to be used later 
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
			
	# print(d.index, best)


#### MIP Modelling! ####

def tryModel():
	n = len(districts)
	m = len(labs)
	c = len(centroids)
	
	model = gp.Model('CNI_Hack')
	model.Params.TimeLimit = 5*60 # 5 MINS 

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
			# model.addVar(lower_bound, upper_bound, objective_coefficient, variable_type, name) 
			x[i][j] = model.addVar(0.0, labs[j].capacity + 100, 0.0, GRB.INTEGER, 'x[%i, %i]' % (i, j))
			labsInside[i].append(j)
			cnt += 1 

	for i in range(n):
		p[i] = {}
		z[i] = {}

		for j in centroidsNearDistrict[i]:
			p[i][j] = model.addVar(0, districts[i].demand, 0.0, GRB.INTEGER, 'p[%i, %i]' % (i, j))

			z[i][j] = model.addVar(0, 1, 0.0, GRB.INTEGER, 'z[%i, %i]' % (i, j))
			# constraint
			model.addConstr(LAMBDA * z[i][j] >= p[i][j])
			cnt += 2

	for j in range(m):
		o[j] = model.addVar(0, 100, 0.0, GRB.INTEGER, 'o[%i]' % j)
		cnt += 1 

	for i in range(c):
		q[i] = {}
		for j in centroids[i].labs:
			q[i][j] = model.addVar(0, labs[j].capacity, 0.0, GRB.INTEGER, 'q[%i, %i]' % (i, j))
			cnt += 1 


	#### -----------------        Add constraints       -------------- #######
	for i in range(n):
		model.addConstr( sum(x[i][j] for j in labsInside[i]) + sum(p[i][j] for j in centroidsNearDistrict[i]) <= districts[i].demand)
		cnt += 1 

	for j in range(m):
		model.addConstr( sum(q[k][j] for k in centroidsNearLab[j]) <= labs[j].capacity)
		district_id = labs[j].district 
		model.addConstr( x[district_id][j] + sum(q[k][j] for k in centroidsNearLab[j]) - 100 <= labs[j].capacity) 
		model.addConstr( x[district_id][j] + sum(q[k][j] for k in centroidsNearLab[j]) - labs[j].capacity <= o[j])

		cnt += 3 

	for i in range(c):
		model.addConstr( sum(q[i][j] for j in centroids[i].labs) == sum(p[j][i] for j in range(n) if i in centroidsNearDistrict[j]) )
		cnt += 1 

	print("\nTotal Samples:", totalSamples)
	print("Model size:", cnt)
	print("\n\n")

	bangalore = 0 
	govn = 0
	priv = 0

	#### -----------------        Form final objective function       -------------- #######
	objective_terms = []
	for i in range(n):
		for j in centroidsNearDistrict[i]:
			objective_terms.append(1000 * distPairs[i][j] * z[i][j])

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

	model.setObjective(sum(objective_terms), GRB.MINIMIZE)
	status = model.optimize()
	newCost = model.objVal 

	print("\nOptimal value of objective function:", newCost)

	# print("Best cost:", newCost)
	print()

	if model.SolCount <= 0:
		print("No feasile solution found!! Please run again with larger time limit.")
		return 

	# return 

	#### -----------------        Extract all the info from the solution found       -------------- #######
	for j in range(m):
		tested = 0 
		district_id = labs[j].district 
		tested += x[district_id][j].X

		for k in centroidsNearLab[j]:
			tested += q[k][j].X

		print("Lab:", j, "\tType:", labs[j].type, "\tCapacity =", labs[j].capacity, "\tTested =", tested, "\tSamples overloaded =", o[j].X)
	
	print()
	for i in range(n):
		sent = 0
		for j in labsInside[i]:
			sent += x[i][j].X

		print("District headquater", i, "\t distributed", sent, "\tsamples out of", districts[i].demand)

		for j in centroidsNearDistrict[i]:
			sent += p[i][j].X


		backlog = districts[i].demand - sent 
		if backlog > 0:
			print("BACKLOG at district", i, "=", backlog, " with demand = ", districts[i].demand)

	print("\n\n")
	for i in range(n):
		for j in centroidsNearDistrict[i]:
			if j in labsInside[i] :
				continue 
			if j in z[i]:
				if z[i][j].X > 0:
					print("Disrict", i, "\t has sent", p[i][j].X, "\t samples to external lab",j,"\t with distance =", distPairs[i][j])

	print("\n\n")
	for i in range(c):
		inc = sum(p[j][i].X for j in range(n) if i in centroidsNearDistrict[j])
		out = sum(q[i][j].X for j in centroids[i].labs)
		district_id = labs[i].district
		default = x[district_id][i].X
		# print("At centroid", i, "\t incoming =", inc, "\t outgoing =", out, "\t capacity=", labs[i].capacity, "  x[i][j] = ", default, " in district", district_id)
		print("At centroid", i, "\t incoming =", inc, "\t outgoing =", out, "\t capacity=", labs[i].capacity, "\tin district", district_id)

	print("\nTotal Government Capacity =", govn, "\nTotal private labs' capacity =", priv)



# to pipe the output 
sys.stdout = open("testing" + str(RADIUS) + ".out", "w")
tryModel()