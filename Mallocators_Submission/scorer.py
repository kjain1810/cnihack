from geopy.distance import geodesic
from math import radians, sin, cos, acos
import pandas as pd
from copy import deepcopy
import math


def dist2(lat1, lon1, lat2, lon2):
    """
    Utility function for finding distance between two points given their 
    latitudes and longitudes
    """
    distance = geodesic((lat1, lon1), (lat2, lon2)).km
    return distance


# Loader for sample data
num = 1
# transactions = pd.read_csv("solution_00"+str(num)+".csv")
# transactions = pd.read_csv("transactions.csv")
# districts = pd.read_csv("district_sample_data_00"+str(num)+".csv")
# labs = pd.read_csv("lab_sample_data_00"+str(num)+".csv")


# Loader for test data
transactions = pd.read_csv("solution_00"+str(num)+".csv")
districts = pd.read_csv("district_test_data_00"+str(num)+".csv")
labs = pd.read_csv("lab_test_data_00"+str(num)+".csv")

labs['capacity'] -= labs['backlogs']

totcost = 0
num_districts = len(districts)
num_labs = len(labs)
overloaded_labs = set()
district_transfer = [[] for i in range(num_districts+1)]

lab_capacity = [0 for i in range(num_labs+1)]
outside_transfer_to_lab = [0 for i in range(num_labs+1)]

for i in range(1, num_labs+1):
    lab_capacity[i] = labs['capacity'][i-1]

# Iterate through all transactions
for i in range(len(transactions)):
    dist = transactions['source'][i]-1
    lab = transactions['destination'][i]-1
    amt = transactions['samples_transferred'][i]
    typ = transactions['transfer_type'][i]
    if typ == 0:
        # If transfer is inter-district, then add it to another list for further analysis
        if dist != labs['district_id'][lab]-1:
            district_transfer[dist].append(deepcopy(transactions.iloc[i]))
        if districts['samples'][dist] < amt:
            print("No samples left to send\n")
            exit(0)
        # If samples transferred but lab not overloaded
        if labs['capacity'][lab] >= amt:
            # Calculate cost for lab
            lab_cost = 800 if labs['lab_type'][lab] == 0 else 1600
            labs.loc[lab, 'capacity'] = labs['capacity'][lab] - amt
            districts.loc[dist, 'samples'] = districts['samples'][dist] - amt
            totcost += amt * lab_cost
        # If samples transferred and lab not overloaded
        elif labs['capacity'][lab] + 100 >= amt:
            # Calculate cost for lab
            lab_cost = 800 if labs['lab_type'][lab] == 0 else 1600
            init_amt = max(0, labs['capacity'][lab])
            over_amt = amt - init_amt  # Find overload amount
            districts.loc[dist, 'samples'] = districts['samples'][dist] - amt
            labs.loc[lab, 'capacity'] = labs['capacity'][lab] - amt
            totcost += lab_cost * init_amt + \
                (5000 + lab_cost) * over_amt  # Add all costs
            overloaded_labs.add(lab)
        else:
            print("Lab cant overload further\n")
            exit(0)
    else:
        # If backlog in districts
        if districts['samples'][dist] < amt:
            print("No samples left to carry over\n")
            exit(0)
        districts.loc[dist, 'samples'] = districts['samples'][dist] - amt
        totcost += 10000 * amt

# Calculate transfer costs
for i in range(num_districts):
    cnt = 0
    lat = 0
    lon = 0
    for tr in district_transfer[i]:
        dist = tr['source']-1
        lab = tr['destination']-1
        amt = tr['samples_transferred']
        cnt += 1
        lat += labs['lat'][lab]
        lon += labs['lon'][lab]
        # Check if labs are all 40km or less apart
        for tr2 in district_transfer[i]:
            lab2 = tr2['destination']-1
            if (dist2(labs['lat'][lab], labs['lon'][lab], labs['lat'][lab2], labs['lon'][lab2]) > 40):
                print("District " + str(dist) + " sending samples to labs " +
                      str(lab) + " and " + str(lab2) + " which are more than 40km apart\n")
                exit(0)
    if cnt == 0:
        continue
    lat = lat / cnt
    lon = lon / cnt
    dist = dist2(districts['lat'][i], districts['lon'][i], lat, lon)
    totcost += 1000 * dist  # Add transfer costs


print(totcost)
