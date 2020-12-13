from geopy.distance import geodesic
import pandas as pd

from collections import namedtuple
import sys, time 

from mip import *

Point = namedtuple("Point", ['x', 'y'])
District = namedtuple("District", ['index', 'demand', 'location'])
Lab = namedtuple("Lab", ['index', 'capacity', 'location', 'type', 'district'])
Centroid = namedtuple("Centroid", ['index', 'labs', 'location'])

districts = []
labs = []
totalSamples = 0 

def loadInput(FACTOR = 1):
	"""
	Loads the data from the input files (district and lab) 
	passed as system argument while running the python file from command line.

	Arguments:
	-FACTOR (optional): Scales the number of samples in district by 'FACTOR' amount.
			 Vary this number if you want to see how the MIP performs when the 
			 number of samples on some day is more/less than the overall capacity.
	"""
	global labs, districts

	district_data = pd.read_csv(sys.argv[1], index_col = 0)
	lab_data = pd.read_csv(sys.argv[2], index_col = 0)

	for idx, line in lab_data.iterrows():
		labs.append(Lab(idx, int(line['capacity'] - line['backlogs']), Point(float(line['lat']), float(line['lon']) ), int(line['lab_type']), int(line['district_id']) - 1))

	for idx, line in district_data.iterrows():
		districts.append(District(idx, int(line['samples'] * FACTOR), Point(float(line['lat']), float(line['lon']) ) ))

labsInside = {}
centroids = []
centroidsNearLab = {}
centroidsNearDistrict = {}
districtsNearCentroid = {}
centroidCnt = 0 
singleCentroidLabs = [] 
labsCentroid = set([])

def checkCentroidInDistrict(d, c):
	"""
	Helper function to check whether any of the labs 
	in the given centorid lies in the given district.
	Used by other functions when connecting a district to a 
	centorid to see whether it is a valid connection (ie. all
	labs in the centorid should lie outside the district)

	Arguments:
	-d: The district 
	-c: The centroid
	"""
	for lab in c.labs:
		if labs[lab].district == d.index :
			return True 
	return False 

def loadCentroids(fileName = "Centroidsv6.txt", addIndividualLabs = 1):
	"""
	Loads the labs prepared as centroid from the given text file.
	Also adds individual labs as centroids if specified.

	Arguments:
	-fileName (optional): The name of the file from which centroids
							will be read.

	-addIndividualLabs (optional): Flag to specify whether to add individual 
									labs as centroids.
	"""
	global centroids, centroidsNearLab, singleCentroidLabs, centroidCnt, labsCentroid
	print("Loading centroids from", fileName)

	try:
		fc = open(fileName, "r");
		num_centroids = int(fc.readline())

		for line in fc.readlines():
			nums = list(map(int, line.split()))
			sz = nums[0] 

			locX = 0 
			locY = 0
			labs_here = []

			for lab_idx in nums[1:]:
				idx = lab_idx - 1 
				labs_here.append(idx)

				locX += labs[idx].location.x 
				locY += labs[idx].location.y 

			locX /= float(len(labs_here))
			locY /= float(len(labs_here))

			labCopy = frozenset(labs_here)
			if labCopy in labsCentroid:
				# print("Redundant:", labs_here, fileName)
				continue 

			# Add centroid to labs 
			for lab_idx in nums[1:]:
				idx = lab_idx - 1 

				if idx not in centroidsNearLab :
					centroidsNearLab[idx] = [centroidCnt] 
				else :
					centroidsNearLab[idx].append(centroidCnt)

			labsCentroid.add(labCopy)
			centroids.append(Centroid(centroidCnt, labs_here, Point(locX, locY)))
			centroidCnt += 1 
	except:
		print("Invalid file given to read centroids!")


	if addIndividualLabs:
		for lab in labs:
			labCopy = frozenset([lab.index])
			if labCopy in labsCentroid:
				continue 

			centroids.append(Centroid(centroidCnt, [lab.index], lab.location))
			idx = lab.index 

			if idx not in centroidsNearLab :
				centroidsNearLab[idx] = [centroidCnt] 
			else :
				centroidsNearLab[idx].append(centroidCnt)

			centroidCnt += 1 

def addBangalore():
	"""
	Function to add Bangalore as a centroid separately 
	(since it is the largest hub so can be useful when 
	there is excess of samples some day).
	"""
	global centroidsNearLab, centroids, centroidCnt, labsCentroid
	print("Adding centroid of Bangalore")

	bangaloreX = 0
	bangaloreY = 0
	bangaloreLabs = []

	for lab in labs:
		if lab.district == 4: # Bangalore District ID 
			bangaloreLabs.append(lab.index)
			bangaloreX += lab.location.x 
			bangaloreY += lab.location.y 

	bangaloreX /= float(len(bangaloreLabs))
	bangaloreY /= float(len(bangaloreLabs))

	labCopy = frozenset(bangaloreLabs)
	if labCopy in labsCentroid:
		print("Bangalore Redundant:", labs_here)
		return

	for lab in labs:
		if lab.district == 4: # Bangalore District ID 
			centroidsNearLab[lab.index].append(len(centroids))

	labsCentroid.add(labCopy)
	centroids.append(Centroid(len(centroids), bangaloreLabs, Point(bangaloreX, bangaloreY)))
	centroidCnt += 1

def formDistrictCentroids():
	"""
	Helpfer function used in obtaining the *tighter lower bound*
	Please refer to the REPORT for more details.

	Form a centroid for all the labs in each district EXCEPT 
	21, 22 and 24, 5 as the distance across tinter-distrcit labs 
	is less than 40KMs hence, combined those districts together 
	to form one big centorid of all the labs from both the districts.
	"""
	global centroidsNearLab, centroids, centroidCnt, labsCentroid

	cross1 = [21, 22]
	cross2 = [5, 24]
	for i in range(n):
		if i in cross1 or i in cross2 :
			continue 

		locX = 0 
		locY = 0
		labs_here = []

		for idx in range(m):
			if labs[idx].district != i:
				continue

			labs_here.append(idx)

			# if idx not in centroidsNearLab :
			# 	centroidsNearLab[idx] = [centroidCnt] 
			# else :
			# 	centroidsNearLab[idx].append(centroidCnt)

			locX += labs[idx].location.x 
			locY += labs[idx].location.y 

		try:
			locX /= float(len(labs_here))
			locY /= float(len(labs_here))

			labCopy = frozenset(labs_here)
			if labCopy not in labsCentroid:
				labsCentroid.add(labCopy)

				for idx in range(m):
					if labs[idx].district != i:
						continue
					if idx not in centroidsNearLab :
						centroidsNearLab[idx] = [centroidCnt] 
					else :
						centroidsNearLab[idx].append(centroidCnt)

				centroids.append(Centroid(centroidCnt, labs_here, Point(locX, locY)))
				centroidCnt += 1 

		except:
			continue 

	# Add 22 and 21 
	locX = 0 
	locY = 0
	labs_here = []

	for idx in range(m):
		if labs[idx].district not in cross1 :
			continue

		labs_here.append(idx)

		locX += labs[idx].location.x 
		locY += labs[idx].location.y 

	try:
		locX /= float(len(labs_here))
		locY /= float(len(labs_here))


		labCopy = frozenset(labs_here)
		if labCopy not in labsCentroid:
			labsCentroid.add(labCopy)

			for idx in range(m):
				if labs[idx].district not in cross1:
					continue
				if idx not in centroidsNearLab :
					centroidsNearLab[idx] = [centroidCnt] 
				else :
					centroidsNearLab[idx].append(centroidCnt)

			centroids.append(Centroid(centroidCnt, labs_here, Point(locX, locY)))
			centroidCnt += 1 
	except:
		pass
	print("Added centroid for district 22 and 21")

	# Add 24 and 5 
	locX = 0 
	locY = 0
	labs_here = []

	for idx in range(m):
		if labs[idx].district not in cross2:
			continue

		labs_here.append(idx)

		locX += labs[idx].location.x 
		locY += labs[idx].location.y 

	try:
		locX /= float(len(labs_here))
		locY /= float(len(labs_here))

		labCopy = frozenset(labs_here)
		if labCopy not in labsCentroid:
			labsCentroid.add(labCopy)

			for idx in range(m):
				if labs[idx].district not in cross1:
					continue
				if idx not in centroidsNearLab :
					centroidsNearLab[idx] = [centroidCnt] 
				else :
					centroidsNearLab[idx].append(centroidCnt)

			centroids.append(Centroid(centroidCnt, labs_here, Point(locX, locY)))
			centroidCnt += 1 
	except:
		pass

	print("Added centroid for district 24 and 5")

distPairs = {}
def formAllPairs(index= 0, RADIUS = float('inf')):
	"""
	Forms pair between district and centroids if the conection is valid 
	(ie. all labs of centroid lie outside the district and distance between
	them is not more than the specified RADIUS). These pairs are later 
	used by the MIP model to form connections. 

	Arguments:
	-index (optional): The starting index form which the centroids will be 
						iterated to form paris with the districts.
	-RADIUS (optional): The maximum distance allowed to form a connection with
						 a centorid. Might be used to restrict inter-district
						 transfer to nearby (external) labs only.
	"""
	global centroidsNearDistrict, distPairs, districtsNearCentroid
	print("Forming pairs from index =", index, "with radial distance =", RADIUS)

	for d in districts:
		if d.index not in distPairs:
			distPairs[d.index] = {}
			centroidsNearDistrict[d.index] = []

		for c in centroids[index:]:
			if c.index in distPairs[d.index]:
				continue 
			if checkCentroidInDistrict(d, c):
				continue 

			gap = geodesic((d.location.x, d.location.y), (c.location.x, c.location.y)).km 
			distPairs[d.index][c.index] = gap 

			if gap <= RADIUS:
				centroidsNearDistrict[d.index].append(c.index)

				if c.index in districtsNearCentroid:
					districtsNearCentroid[c.index].append(d.index)
				else :
					districtsNearCentroid[c.index] = [d.index] 

def loadPairs(fileName = "connections.txt", RADIUS = float('inf')):
	"""
	Loads centroid-district pairs already prepared in a file.
	These connections are directly fed to MIP as input.

	Arguments:
	-fileName (optional): The name of the file from which the pairs
							will be loaded.
	-RADIUS (optional):	  The maximum distance allowed to form a pair.
	"""
	print("Loading pairs from file", fileName)

	f = open(fileName, "r")
	firstLine = f.readline()
	num_lines = int(firstLine.split('\n')[0])

	for i in range(num_lines):
		line = f.readline().split('\n')
		num_centroids = int(line[0])
		which_centroids = f.readline().split()

		d = districts[i] 
		if d.index not in distPairs:
			distPairs[d.index] = {}
			centroidsNearDistrict[d.index] = []

		which_centroids.append(len(centroids))
		num_centroids += 1 

		for j in range(num_centroids):
			k = int(which_centroids[j]) 
			c = centroids[k-1] 

			if c.index in distPairs[d.index]:
				continue 

			if checkCentroidInDistrict(d, c):
				continue 

			gap = geodesic((d.location.x, d.location.y), (c.location.x, c.location.y)).km 
			distPairs[d.index][c.index] = gap 

			if gap <= RADIUS:
				centroidsNearDistrict[d.index].append(c.index)

				if c.index in districtsNearCentroid:
					districtsNearCentroid[c.index].append(d.index)
				else :
					districtsNearCentroid[c.index] = [d.index] 

	f.close()

#### MIP Modelling! ####
LAMBDA = int(1e7)
PRECISION = 0.99

class Solver:
	def __init__(self, timeLimit = 300, which = GRB):
		"""
		Initializer the model, number of threads in it, 
		gap between optimality till which the solver should
		run and wrapper for all the decision variables 
		(refer to the REPORT for more details) and other
		useful class variables.

		Arguments:
		-timeLimit (optional): Maximum time till which the solver
								should run.
		-which (optional):	   The type of solver to be employed
							   for solving the MIP. 

		NOTE: Currently, we try to use Gurobi (GRB) if possible  
			  (since it is much faster than most others). By default, 
			  however, PythonMIP uses CBC (Coin Branch and Cut solver) 
			  which is open-source and decent but nowhere near the 
			  performance of Gurobi. 
		"""
		try:
			self.model = Model(solver_name=which)
		except:
			self.model = Model('Swabs2Labs')

		if which != GRB:
			self.model.threads = 4 

		self.model.max_min_gap = 0.01
		# 1 for FEASIBLITY (improve current solution) and 2 for OPTIMALITY (dual bound) and 0 for balance among both
		self.model.emphasis = 1 
		self.modelSize = 0
		self.timeLimit = timeLimit
		self.bestCost = float('inf')
		self.solutionFound = 0

		self.x, self.xCopy = [{}, {}]
		self.p, self.pCopy = [{}, {}]
		self.q, self.qCopy = [{}, {}]
		self.o, self.oCopy = [{}, {}]
		self.z, self.zCopy = [{}, {}]

		self.transfers = [[0 for j in range(m)] for i in range(n)]
		self.external = [[0 for j in range(m)] for i in range(n)]
		self.backlogs = [0 for i in range(n)]

	def addDecisionVariables(self):
		"""
		Preapres all the decision variables used in the MIP
		formulation. Please refer to the REPORT for more details.
		"""
		global labsInside 

		for i in range(n):
			self.x[i] = {}
			labsInside[i] = []

			for j in range(m):
				if labs[j].district != i:
					continue 

				self.x[i][j] = self.model.add_var('x[%i, %i]' % (i, j), 0.0, labs[j].capacity + 100, 0.0, INTEGER)
				labsInside[i].append(j)
				self.modelSize += 1

		for i in range(n):
			self.p[i] = {}
			self.z[i] = {}

			for j in centroidsNearDistrict[i]:
				self.p[i][j] = self.model.add_var('p[%i, %i]' % (i, j), 0, districts[i].demand, 0.0, INTEGER)
				self.z[i][j] = self.model.add_var('z[%i, %i]' % (i, j), 0, 1, 0.0, BINARY)

				# binary constraint
				self.model.add_constr(LAMBDA * self.z[i][j] >= self.p[i][j])
				self.modelSize += 3

		for j in range(m):
			self.o[j] = self.model.add_var('o[%i]' % j, 0, 100, 0.0, INTEGER)
			self.modelSize += 1 

		for i in range(c):
			self.q[i] = {}
			for j in centroids[i].labs:
				self.q[i][j] = self.model.add_var('q[%i, %i]' % (i, j), 0, labs[j].capacity, 0.0, INTEGER)
				self.modelSize += 1  

	def addConstraints(self):
		"""
		Forms all the constraints to be satisfied by the MIP model. 
		Please refer to the REPORT for more details.
		"""
		for i in range(n):
			self.model.add_constr( xsum(self.x[i][j] for j in labsInside[i]) + xsum(self.p[i][j] for j in centroidsNearDistrict[i]) <= districts[i].demand)
			self.model.add_constr( xsum(self.z[i][c] for c in centroidsNearDistrict[i] ) <= 1)
			self.modelSize += 2

		for j in range(m):
			self.model.add_constr( xsum(self.q[k][j] for k in centroidsNearLab[j]) <= labs[j].capacity)
			district_id = labs[j].district 
			self.model.add_constr( self.x[district_id][j] + xsum(self.q[k][j] for k in centroidsNearLab[j]) - 100 <= labs[j].capacity) 
			self.model.add_constr( self.x[district_id][j] + xsum(self.q[k][j] for k in centroidsNearLab[j]) - labs[j].capacity <= self.o[j])

			self.modelSize += 3 

		for i in range(c):
			self.model.add_constr( xsum(self.q[i][j] for j in centroids[i].labs) == xsum(self.p[j][i] for j in range(n) if i in centroidsNearDistrict[j]) )
			for j in centroids[i].labs:
				# self.model.add_constr( LAMBDA * self.q[i][j] >= xsum(self.p[k][i] for k in range(n) if i in centroidsNearDistrict[k]) )
				self.model.add_constr( self.q[i][j] >= xsum(self.z[k][i] for k in range(n) if i in centroidsNearDistrict[k] ))

				self.modelSize += 1
			self.modelSize += 1 

	def addObjectiveFunction(self):
		"""
		Preapres the objective function from the decision variables 
		which is to be minimized. Please refer to the REPORT for more details.
		"""
		objective_terms = []
		for i in range(n):
			for j in centroidsNearDistrict[i]:
				objective_terms.append(1000 * distPairs[i][j] * self.z[i][j])

		for j in range(m):
			objective_terms.append(5000 * self.o[j])

			if labs[j].type == 1:
				cost = 1600 
			else :
				cost = 800 

			district_id = labs[j].district 
			objective_terms.append(cost * self.x[district_id][j])

			for k in centroidsNearLab[j]:
				objective_terms.append(cost * self.q[k][j])

		for i in range(n):
			objective_terms.append(10000 * districts[i].demand)
			for j in labsInside[i]:
				objective_terms.append(-10000 * self.x[i][j])
			for j in centroidsNearDistrict[i]:
				objective_terms.append(-10000 * self.p[i][j])

		self.model.objective = minimize(xsum(objective_terms))

	def prepareOutputFileName(self, detailed = 0):
		"""
		Prepares the name of the file to which the output
		shall be redirecte depending upon the name of input files.

		For example:
		
			Input File Name(s)	| 	Output file name 
		district_test_data_001 -> test_data_001_solution
		lab_sample_data_003    -> test_data_003_solution

		Arguments:
		-detailed (optional): Whether the string detialed needs
								to be added to the name.
		
		Returns: The name of the file prepared.
		"""
		fileName = ""
		try:
			args = sys.argv[1].split('_')
			last = args[-1].split('.')
			finalNameList = args[1:-1]
			finalNameList.append(last[0])
			if detailed:
				finalNameList.append("detailed")
			finalNameList.append("solution")
			finalNameList.append(str(self.timeLimit) + "s")
			fileName = '_'.join(finalNameList)
		except:
			fileName = "solution"
			if detailed:
				fileName = "detailed_solution"

		return fileName 

	def outputSolution(self, fileName=""):
		"""
		Outputs solution in the expected format 
		in fileName.out file

		Arguments:
		-fileName (optinal): The name of the file 
							 to which the output shall
							 be printed.
		"""
		if self.solutionFound == 0:
			print("No solution found to print! Please run the solver again with larger time limit.")
			return 
			
		if fileName == "":
			fileName = self.prepareOutputFileName() + ".out"

		print("Outputing actual solution to", fileName)

		f = open(fileName, "w")
		for i in range(n):
			for j in range(m):
				if self.transfers[i][j] > 0:
					text = ' '.join(list(map(str, [0, i+1, j+1, self.transfers[i][j]])))
					f.write(text)
					f.write('\n')

			if self.backlogs[i] > 0:
				text = ' '.join(list(map(str, [1, i+1, i+1, self.backlogs[i]])))
				f.write(text)
				f.write('\n')

		f.close()

	def outputCSV(self, fileName=""):
		"""
		Outputs solution in the expected format in
		fileName.csv (the proper format)
		
		Arguments:
		-fileName (optinal): The name of the file 
							 to which the output shall
							 be printed.
		"""
		if self.solutionFound == 0:
			print("No solution found to print! Please run the solver again with larger time limit.")
			return 
			
		if fileName == "":
			fileName = self.prepareOutputFileName() + ".csv"

		print("Outputing transfers to", fileName)
		df = pd.DataFrame(columns=['transfer_type', 'source', 'destination', 'samples_transferred'])

		k = 0
		for i in range(n):
			for j in range(m):
				if self.transfers[i][j] > 0:
					df.loc[k] = [0, i+1, j+1, self.transfers[i][j]]
					k += 1

			if self.backlogs[i] > 0:
				df.loc[k] = [1, i+1, i+1, self.backlogs[i]]
				k += 1
		
		df.to_csv(fileName, index = False, header=True)

	def outputDetailedSolution(self, fileName=""):
		"""
		Outputs detailed solution (all the info about 
		overload, backlog, external and internal transfer)
		in fileName.out file 
		
		Arguments:
		-fileName (optinal): The name of the file 
							 to which the output shall
							 be printed.
		"""
		if self.solutionFound == 0:
			print("No solution found to print! Please run the solver again with larger time limit.")
			return 

		if fileName == "":
			fileName = self.prepareOutputFileName(detailed=1) + ".out"

		print("Outputing detialed solution to", fileName)
		before = sys.stdout 
		sys.stdout = open(fileName, "w")

		print("Optimal Cost =", self.bestCost)
		print("Model Size =", self.modelSize)
		print("Run solver for ", self.timeLimit, "seconds")
		print("\n\n")

		for j in range(m):
			tested = 0 
			district_id = labs[j].district 
			tested += self.xCopy[district_id][j]

			for k in centroidsNearLab[j]:
				tested += self.qCopy[k][j]

			print("Lab:", j, "\tType:", labs[j].type, "\tCapacity =", labs[j].capacity, "\tTested =", tested, "\tSamples overloaded =", self.oCopy[j])
		
		print("\n\n")
		for i in range(n):
			sent = 0
			for j in labsInside[i]:
				sent += self.xCopy[i][j]

			print("District headquater", i, "\t distributed", sent, "\tsamples internally, out of", districts[i].demand, "in the following manner:")
			for j in labsInside[i]:
				if self.xCopy[i][j] > 0:
					print("District", i, "\t has sent", self.xCopy[i][j], "\t samples to lab", j)

			for j in centroidsNearDistrict[i]:
				if self.pCopy[i][j] > 0 :
					sent += self.pCopy[i][j]

			backlog = districts[i].demand - sent 
			if backlog > 0:
				print("BACKLOG at district", i, "=", backlog, "\t with demand = ", districts[i].demand)
			
			print()

		print("\n\n")
		for i in range(n):
			for j in centroidsNearDistrict[i]:
				if j in self.pCopy[i]:
					if self.pCopy[i][j] > 0:
						print("Disrict", i, "\t has sent", self.pCopy[i][j], "\t samples to centroid",j,"\t with labs =", centroids[j].labs, "\t with distance =", distPairs[i][j])
						for lab in centroids[j].labs:
							print("District", i, "\t sent", self.external[i][lab], "\t samples to lab", lab)
						print()

		print("\n\n")
		for i in range(c):
			inc = sum(self.pCopy[j][i] for j in range(n) if i in centroidsNearDistrict[j])
			out = sum(self.qCopy[i][j] for j in centroids[i].labs)
			if inc > 0:
				print("At centroid", i, "\t incoming =", inc, "\t outgoing =", out, "\t capacity=", sum(labs[k].capacity for k in centroids[i].labs), "\t whose labs =", centroids[i].labs)

		sys.stdout.close()
		sys.stdout = before 

	def outputActualSolution(self, fileName=""):
		"""
		Outputs the solution actually found by MIP
		where the exact transfer between district to labs
		is not present rather only district to centroid. 
		
		The values also may not be fully integers.
		Arguments:
		-fileName (optinal): The name of the file 
							 to which the output shall
							 be printed.
		"""
		if self.solutionFound == 0:
			print("No solution found to print! Please run the solver again with larger time limit.")
			return 

		if fileName == "":
			fileName = self.prepareOutputFileName(detailed=1) + ".out"

		print("Outputing actual (solver's) solution to", fileName)
		before = sys.stdout 
		sys.stdout = open(fileName, "w")

		print("Optimal Cost =", self.bestCost)
		print("Model Size =", self.modelSize)
		print("Run solver for ", self.timeLimit, "seconds")
		print("\n\n")

		for j in range(m):
			tested = 0 
			district_id = labs[j].district 
			tested += self.x[district_id][j].x

			for k in centroidsNearLab[j]:
				tested += self.q[k][j].x

			print("Lab:", j, "\tType:", labs[j].type, "\tCapacity =", labs[j].capacity, "\tTested =", tested, "\tSamples overloaded =", self.o[j].x)
		
		print("\n\n")
		for i in range(n):
			sent = 0
			for j in labsInside[i]:
				sent += self.x[i][j].x

			print("District headquater", i, "\t distributed", sent, "\tsamples internally, out of", districts[i].demand, "in the following manner:")
			for j in labsInside[i]:
				if self.x[i][j].x > 0:
					print("District", i, "\t has sent", self.x[i][j].x, "\t samples to lab", j)

			for j in centroidsNearDistrict[i]:
				if self.p[i][j].x > 0 :
					sent += self.p[i][j].x

			backlog = districts[i].demand - sent 
			if backlog > 0:
				print("BACKLOG at district", i, "=", backlog, "\t with demand = ", districts[i].demand)
			
			print()

		print("\n\n")
		for i in range(n):
			for j in centroidsNearDistrict[i]:
				if j in self.p[i]:
					if self.p[i][j].x > 0:
						print("Disrict", i, "\t has sent", self.p[i][j].x, "\t samples to centroid",j,"\t with labs =", centroids[j].labs, "\t with distance =", distPairs[i][j])
						for lab in centroids[j].labs:
							print("District", i, "\t sent", self.q[j][lab].x, "\t samples to lab", lab)
						print()

		print("\n\n")
		for i in range(c):
			inc = sum(self.p[j][i].x for j in range(n) if i in centroidsNearDistrict[j])
			out = sum(self.q[i][j].x for j in centroids[i].labs)
			if inc > 0:
				print("At centroid", i, "\t incoming =", inc, "\t outgoing =", out, "\t capacity=", sum(labs[k].capacity for k in centroids[i].labs), "\t whose labs =", centroids[i].labs)

		sys.stdout.close()
		sys.stdout = before 

	def restoreSolution(self):
		"""
		As the values found by the solver may 
		not be truly integers, so it is necessary
		to round them to the closest integer for 
		the solution to be valid.
		"""
		# Recast (round) variables into integers 
		for i in range(n):
			self.xCopy[i] = {}
			for j in range(m):
				if labs[j].district != i:
					continue 

				self.xCopy[i][j] = round(self.x[i][j].x)

		for i in range(n):
			self.pCopy[i] = {}
			self.zCopy[i] = {}

			for j in centroidsNearDistrict[i]:
				self.pCopy[i][j] = round(self.p[i][j].x)
				self.zCopy[i][j] = round(self.z[i][j].x)

				if self.p[i][j].x >= PRECISION:
					if self.zCopy[i][j] != 1:
						print("Error1 in modelling: ", i, j, '%.10f' % self.p[i][j].x, self.z[i][j].x)

				else :
					if self.zCopy[i][j] != 0:
						print("Error2 in modelling: ", i, j, '%.10f' % self.p[i][j].x, self.z[i][j].x)

		for j in range(m):
			self.oCopy[j] = round(self.o[j].x)

		for i in range(c):
			self.qCopy[i] = {}
			for j in centroids[i].labs:
				self.qCopy[i][j] = round(self.q[i][j].x)

	def extractActualTransfers(self):
		"""
		The solution found by MIP only has the values of 
		samples send by each district to each centroid but
		not the individual labs outside.  As the expected
		solution format required precise transfer (internal
		and external), hence proper extraction of the actual
		solutin is required which is done here. 

		First one sample from each lab of the sent centroid 
		is taken to ensure that the centroid's position doesn't
		change due to external labs not getting sample. After
		this, samples are assigned to labs anyhow as long as the 
		distribution is valid.
		"""
		self.external = [[0 for j in range(m)] for i in range(n)]
		self.transfers = [[0 for j in range(m)] for i in range(n)]

		for i in range(n):
			sent = 0
			for j in labsInside[i]:
				sent += self.xCopy[i][j]
				self.transfers[i][j] = self.xCopy[i][j]

			for j in centroidsNearDistrict[i]:
				sent += self.pCopy[i][j]

			backlog = districts[i].demand - sent 
			if backlog > 0:
				self.backlogs[i] = backlog

		for i in range(n):
			for c in centroidsNearDistrict[i]:
				if self.pCopy[i][c] > 0:
					for j in centroids[c].labs:
						self.transfers[i][j] += 1
						self.external[i][j] += 1

						self.qCopy[c][j] -= 1
						self.pCopy[i][c] -= 1

		for i in range(n):
			for c in centroidsNearDistrict[i]:
				if self.pCopy[i][c] > 0:
					req = self.pCopy[i][c]
					for j in centroids[c].labs:
						sent = min(self.qCopy[c][j], req)
						self.qCopy[c][j] -= sent 
						self.pCopy[i][c] -= sent 

						self.transfers[i][j] += sent 
						self.external[i][j] += sent
						req -= sent 

					if req != 0:
						print("Error3: Flow still left from district", i, "to centroid", c)

		for c in centroids:
			for j in c.labs:
				if self.qCopy[c.index][j] != 0:
					print("Error4: Flow pending from centroid", c.index, "to lab", j)

	def run(self):
		"""
		Prepares the whole MIP model and runs
		the solver for the given time limit. 
		Also calls the necessary helper functions 
		to prapre the final solution, which might 
		be required to print later.
		"""
		self.addDecisionVariables()
		self.addConstraints()
		self.addObjectiveFunction()

		print("\n\n")
		print("Model Size:", self.modelSize)
		print("Running solver for", self.timeLimit, "seconds")
		print("\n\n")

		self.status = self.model.optimize(max_seconds = self.timeLimit)
		self.bestCost = self.model.objective_value  

		if self.status != OptimizationStatus.OPTIMAL and self.status != OptimizationStatus.FEASIBLE:
			print("No feasile solution found!! Please run again with larger time limit.")
			return 

		self.solutionFound = 1 

		self.restoreSolution()
		self.extractActualTransfers()
		self.restoreSolution()

loadInput()
n = len(districts)
m = len(labs)

argc = len(sys.argv)

if argc <= 3:
	# loadCentroids()
	loadCentroids("Centroidsv0.txt")
	loadPairs()
	c = len(centroids)
	loadCentroids("handcraftedCentroids.txt")
	formAllPairs(c)
else :
	loadCentroids(sys.argv[3])
	if argc >= 5:
		loadPairs(sys.argv[4])
	else :
		formAllPairs()

c = len(centroids)
TIMER = 600 # 10 mins 
obj = Solver(TIMER)	

obj.run()
obj.outputDetailedSolution()
obj.outputCSV()
