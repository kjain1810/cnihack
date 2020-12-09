import pandas as pd
from geopy.distance import geodesic

f = pd.read_csv('data/lab_data_v0.csv')

labsin5 = []

for idx, row in f.iterrows():
    if row['district_id'] == 5:
        labsin5.append({'id': row['id'], 'lat': row['lat'], 'lon': row['lon']})

n = len(labsin5)

print(n)

edges = []

for i in range(n):
    here = []
    for j in range(n):
        dist = geodesic((labsin5[i]['lat'], labsin5[i]['lon']),
                        (labsin5[j]['lat'], labsin5[j]['lon'])).km
        if dist <= 40.0:
            here.append(1)
        else:
            here.append(0)
            print(
                f"Labs more than 40km apart: {labsin5[i]['id']}, {labsin5[j]['id']} at {dist}km")
    edges.append(here)

print(edges)
