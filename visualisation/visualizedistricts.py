import pandas as pd
import geopandas as gpd
import geoplot as gplt
from geopy.distance import geodesic
from shapely.geometry import Point, LineString, shape
import matplotlib.pyplot as plt

# get district wise map of karnataka
district_df = gpd.read_file(
    'c44c9b96-f570-4ee3-97f1-ebad64efa4c2202044-1-1rb4x6s.8xx6.shp')
district_df = district_df.loc[district_df['statename'] == 'KARNATAKA']

# get locations of labs
data_df = pd.read_csv('../data/lab_data_v0.csv')
data_df.drop(data_df.columns.difference(
    ['lat', 'lon', 'id', 'district_id']), 1, inplace=True)
gdf = gpd.GeoDataFrame(
    data_df, geometry=gpd.points_from_xy(data_df.lon, data_df.lat))

# get district hqs and data
gdf_hq = pd.read_csv('../data/districts_data_v0.csv')
gdf_hq = gpd.GeoDataFrame(
    gdf_hq, geometry=gpd.points_from_xy(gdf_hq.lon, gdf_hq.lat))

# get labs nearer than 40 km
x = gpd.GeoDataFrame({'geometry': []})
for idx1, row1 in gdf.iterrows():
    for idx2, row2 in gdf.iterrows():
        if idx1 == idx2:
            continue
        if row1['district_id'] == 5 and row2['district_id'] == 5:
            continue
        dist = geodesic((row2['lat'], row2['lon']),
                        (row1['lat'], row1['lon'])).km
        if dist <= 40:
            line = LineString(
                ([Point(row1['lon'], row1['lat']), Point(row2['lon'], row2['lat'])]))
            x.loc[len(x)] = [line]

# plot
ax = gplt.polyplot(district_df)
gplt.pointplot(gdf, ax=ax)
c = [1, 0, 0]
gplt.pointplot(gdf_hq, ax=ax, color=c)
gplt.sankey(x, ax=ax)

plt.show()
