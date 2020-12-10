from geopy.distance import geodesic
from math import radians, sin, cos, acos
import pandas as pd
from copy import deepcopy
import math
# pd.options.mode.chained_assignment = None
kmR = 6373.0


def toRadians(degree):
    return (degree * math.pi) / 180


def distance(lat1d, lon1d,  lat2d, lon2d):
    # double lat1r, lon1r, lat2r, lon2r, u, v
    lat1r = toRadians(lat1d)
    lon1r = toRadians(lon1d)
    lat2r = toRadians(lat2d)
    lon2r = toRadians(lon2d)
    u = math.sin((lat2r - lat1r) / 2)
    v = math.sin((lon2r - lon1r) / 2)
    return 2.0 * kmR * math.asin(math.sqrt(u * u + math.cos(lat1r) * math.cos(lat2r) * v * v))


def dist2(lat1, lon1, lat2, lon2):
    # R = 6373.0

    # lat1a = math.radians(lat1)
    # lon1a = math.radians(lon1)
    # lat2a = math.radians(lat2)
    # lon2a = math.radians(lon2)

    # # dlon = lon2 - lon1

    # # dlat = lat2 - lat1

    # # a = math.sin(dlat / 2)**2 + math.cos(lat1) * math.cos(lat2) * math.sin(dlon / 2)**2

    # # c = 2 * math.atan2(math.sqrt(a), math.sqrt(1 - a))
    # # distance = R * c
    # distance = 6371.01 * acos(sin(lat1a)*sin(lat2a) +
    #                           cos(lat1a)*cos(lat2a)*cos(lon1a - lon2a))
    # # print(distance)

    distance = geodesic((lat1, lon1), (lat2, lon2)).km
    return distance


transactions = pd.read_csv("transactions.csv")
num = 3
districts = pd.read_csv("district_sample_data_00"+str(num)+".csv")
labs = pd.read_csv("lab_sample_data_00"+str(num)+".csv")
labs['capacity'] -= labs['backlogs']

totcost = 0
num_districts = len(districts)
num_labs = len(labs)
print(num_districts, num_labs)
overloaded_labs = set()
# vector<vector<pair<pair<int, int>, pair<int, int>>>> district_transfer(num_districts + 1);
district_transfer = [[] for i in range(num_districts+1)]

# vector < int > lab_capacity(num_labs + 1, 0)
lab_capacity = [0 for i in range(num_labs+1)]
# vector < int > outside_transfer_to_lab(num_labs + 1, 0)
outside_transfer_to_lab = [0 for i in range(num_labs+1)]

for i in range(1, num_labs+1):
    lab_capacity[i] = labs['capacity'][i-1]

for i in range(len(transactions)):
    dist = transactions['source'][i]-1
    lab = transactions['destination'][i]-1
    amt = transactions['samples_transferred'][i]
    typ = transactions['transfer_type'][i]
    if typ == 0:
        if dist != labs['district_id'][lab]-1:
            district_transfer[dist].append(deepcopy(transactions.iloc[i]))
        if districts['samples'][dist] < amt:
            print(districts['samples'][dist], amt, i)
            print("No samples left to send\n")
            exit(0)
        if labs['capacity'][lab] >= amt:
            lab_cost = 800 if labs['lab_type'][lab] == 0 else 1600
            if totcost == 83780000:
                print(lab_cost, "POOOO")
            labs.loc[lab, 'capacity'] = labs['capacity'][lab] - amt
            districts.loc[dist, 'samples'] = districts['samples'][dist] - amt
            totcost += amt * lab_cost
        elif labs['capacity'][lab] + 100 >= amt:
            lab_cost = 800 if labs['lab_type'][lab] == 0 else 1600
            init_amt = max(0, labs['capacity'][lab])
            over_amt = amt - init_amt
            districts.loc[dist, 'samples'] = districts['samples'][dist] - amt
            labs.loc[lab, 'capacity'] = labs['capacity'][lab] - amt
            totcost += lab_cost * init_amt + (5000 + lab_cost) * over_amt
            overloaded_labs.add(lab)
        else:
            print("Lab cant overload further\n")
            exit(0)
    else:
        if districts['samples'][dist] < amt:
            print("No samples left to carry over\n")
            exit(0)
        districts.loc[dist, 'samples'] = districts['samples'][dist] - amt
        totcost += 10000 * amt
    print(totcost)

print(totcost)

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
    totcost += 1000 * dist

print(totcost)
# for (auto tr: transactions)
# {
#     int dist = tr.first.second
#     int lab = tr.second.first
#     int amt = tr.second.second
#     // if (overloaded_labs.find(lab) != overloaded_labs.end())
#     // {
#         // if (labz[lab].district != dist)
#         // {
#             // cout << "Cant overload a lab from another district\n"
#             // exit(0)
#             // }
#         //}
#     if (tr.first.first != 0)
#     continue
#     if (dist != labz[lab].district)
#     {
#         outside_transfer_to_lab[lab] += amt
#         if (outside_transfer_to_lab[lab] > lab_capacity[lab])
#         {
#             cout << "Cant overload a lab from another district\n"
#             cout << "Lab " << lab << " overloaded by outside districts\n"
#             cout << "Lab capacity - " << lab_capacity[lab] << '\n'
#             exit(0)
#         }
#     }
#     // if (dist == 1)
#     // {
#         // cout << districts[1].samples << '\n'
#         //}
# }

# for (int i=1
#      i <= num_districts
#      i++)
# {
#     if (districts[i].samples > 0)
#     {
#         // cout << i << " " << districts[i].samples << '\n'
#         cout << "Unallocated swabs!\n"
#         exit(0)
#     }
# }
# return totcost
