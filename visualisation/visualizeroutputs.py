import pandas as pd
import geopandas as gpd
import geoplot as gplt
from geopy.distance import geodesic
from shapely.geometry import Point, LineString, shape
import matplotlib.pyplot as plt
import sys

if len(sys.argv) != 4:
    print("Usage: python3 visualizeroutputs.py <district_data> <lab_data> <transactions>")
    exit(0)

# get district wise map of karnataka
district_df = gpd.read_file(
    'c44c9b96-f570-4ee3-97f1-ebad64efa4c2202044-1-1rb4x6s.8xx6.shp')
district_df = district_df.loc[district_df['statename'] == 'KARNATAKA']

# get locations of labs
data_df = pd.read_csv(sys.argv[2])
data_df.drop(data_df.columns.difference(
    ['lat', 'lon', 'id', 'district_id']), 1, inplace=True)
gdf = gpd.GeoDataFrame(
    data_df, geometry=gpd.points_from_xy(data_df.lon, data_df.lat))

# get district hqs and data
gdf_hq = pd.read_csv(sys.argv[1])
gdf_hq = gpd.GeoDataFrame(
    gdf_hq, geometry=gpd.points_from_xy(gdf_hq.lon, gdf_hq.lat))

# load output file
fz = pd.read_csv(sys.argv[3])

# get interdistrict transfers
data = []
x2 = gpd.GeoDataFrame({'geometry': []})
rec = gpd.GeoDataFrame({'geometry': []})
for idx, row in fz.iterrows():
    a, b, c, d = row['transfer_type'], row['source'], row['destination'], row['samples_transferred']
    if d == 0:
        continue
    if a == 0:
        for _, row in gdf_hq.iterrows():
            if row['district_id'] != b:
                continue
            for _, row2 in gdf.iterrows():
                if row2['id'] != c:
                    continue
                if row2['district_id'] == row['district_id']:
                    break
                x2.loc[len(x2)] = [LineString(
                    [(row['geometry']), (row2['geometry'])])]
                rec.loc[len(rec)] = [
                    Point(row2['geometry'].x, row2['geometry'].y)]
                break
            break

# get labs nearer than 40 km
x = gpd.GeoDataFrame({'geometry': []})
for idx1, row1 in gdf.iterrows():
    for idx2, row2 in gdf.iterrows():
        if idx1 == idx2:
            continue
        if row1['district_id'] == 5 or row2['district_id'] == 5:
            continue
        dist = geodesic((row2['lat'], row2['lon']),
                        (row1['lat'], row1['lon'])).km
        if dist <= 40:
            line = LineString(
                ([Point(row1['lon'], row1['lat']), Point(row2['lon'], row2['lat'])]))
            x.loc[len(x)] = [line]

# plot
ax3 = gplt.sankey(x, figsize=(30, 30))
gplt.sankey(x2, ax=ax3, color=[1, 0, 0, 1])
gplt.polyplot(district_df, ax=ax3)
gplt.pointplot(rec, ax=ax3, color=[0, 1, 0])
gplt.pointplot(gdf, ax=ax3)
c = [1, 0, 0]
gplt.pointplot(gdf_hq, ax=ax3, color=c)

# label labs and hq
plt.rc('font', size=10)
for idx, row in gdf.iterrows():
    if row['district_id'] != 5:
        plt.text(s=row['id'], x=row['lon'], y=row['lat'])
for idx, row in district_df.iterrows():
    pt = row['geometry'].centroid
    lmao = pd.read_csv('../data/districts_data_v0.csv')
    for idx, row2 in lmao.iterrows():
        if row2['district_name'].lower() == row['distname'].lower():
            plt.text(s=row2['district_id'], x=pt.x, y=pt.y, color=c)
plt.show()
