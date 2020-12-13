import pandas as pd 
import numpy as np
import math

DEBUG_PRINTS = False


COST_GOVT_TEST = 800
COST_PVT_TEST = 1600
COST_LAB_OVERLOAD = 5000
COST_BACKLOG = 10000
LAB_EXCESS_CAPACITY = 100
LAB_MAX_RADIUS = 40
RADIUS_TOLERANCE = 0
OUTSIDE_DISTRICT_TRANSPORTATION_COST = 1000


LAB_TYPE_GOVT = 0

def backlog_same_distrct_constraint(df_solution):
    #Check if backlogs are restricted to same district.
    df_temp = df_solution[df_solution['transfer_type']==1].copy()
    valid = df_temp['source'].equals(df_temp['destination'])
    print("backlog_same_distrct_constraint: ", valid)
    return valid

def all_district_samples_accounted(df_districts,df_solution):
    valid = True
    for district_index in df_districts.index:
        district_id = df_districts.loc[district_index,'district_id']
        caseload = df_districts.loc[district_index,'samples']
        allocated = df_solution[df_solution['source']==district_id]['samples_transferred'].sum()
        valid = valid and (caseload == allocated)
    print("all_district_samples_accounted: ", valid)
    return valid

def lab_capacity_accounted(df_labs,df_solution):
    valid = True
    for lab_index in df_labs.index.values:
        lab_id = df_labs.loc[lab_index,'id']
        lab_district_id = df_labs.loc[lab_index,'district_id']
        lab_capacity = df_labs.loc[lab_index,'capacity']-df_labs.loc[lab_index,'backlogs']
        outside_district_allocated = df_solution[(df_solution['transfer_type']==0) & (df_solution['destination']==lab_id) & (df_solution['source']!=lab_district_id)]['samples_transferred'].sum()
        total_allocated = df_solution[(df_solution['transfer_type']==0) & (df_solution['destination']==lab_id)]['samples_transferred'].sum()
        #print("lab_id: ", lab_id, ".lab_district_id", lab_district_id, ". lab_capacity: ",lab_capacity,". outside_district_allocated: ", outside_district_allocated,". total_allocated",total_allocated)
        if(outside_district_allocated > lab_capacity):
            valid = False
            if(DEBUG_PRINTS) : print("outside_district_allocated exceeds capacity")
        if(total_allocated > lab_capacity+LAB_EXCESS_CAPACITY):
            valid = False
            if(DEBUG_PRINTS) : print("Total allocation more than capacity +",LAB_EXCESS_CAPACITY)
    print("Lab allocation constraints :", valid)

    return valid

def distance_haversine(lat1, lon1, lat2, lon2):
    radius = 6371 # km

    dlat = math.radians(lat2-lat1)
    dlon = math.radians(lon2-lon1)
    a = math.sin(dlat/2) * math.sin(dlat/2) + math.cos(math.radians(lat1)) * math.cos(math.radians(lat2)) * math.sin(dlon/2) * math.sin(dlon/2)
    c = 2 * math.atan2(math.sqrt(a), math.sqrt(1-a))
    d = radius * c

    return d

def labs_within_radius(outside_district_transfer_lab_locations):
    valid = True
    if(len(outside_district_transfer_lab_locations) <= 1):
        return valid
    
    centroid = np.mean(outside_district_transfer_lab_locations,axis=0)
    for lab_location in outside_district_transfer_lab_locations:
        for lab_location1 in outside_district_transfer_lab_locations:
            distance_labs = distance_haversine(lab_location1[0],lab_location1[1], lab_location[0],lab_location[1])
            if(DEBUG_PRINTS) : print("lab_location1",lab_location1, "lab_location:", lab_location,"distance_between_labs:",distance_labs)
            if(distance_labs > (LAB_MAX_RADIUS+RADIUS_TOLERANCE)):
                valid=False
    return valid

def outside_district_lab_distances(df_labs,df_solution):
    valid = True
    for district_id in df_solution[(df_solution['transfer_type']==0)]['source'].unique():
        district_transfer_labs = df_solution[(df_solution['transfer_type']==0) & (df_solution['source']==district_id)]['destination'].unique()
        outside_district_transfer_labs=[]
        outside_district_transfer_lab_locations=[]
        for lab in district_transfer_labs:
            if df_labs[df_labs['id']==lab]['district_id'].values[0] !=district_id:
                outside_district_transfer_labs.append(lab)
                outside_district_transfer_lab_locations.append((df_labs[df_labs['id']==lab]['lat'].values[0],df_labs[df_labs['id']==lab]['lon'].values[0]))
        valid = valid and labs_within_radius(outside_district_transfer_lab_locations)
        if(DEBUG_PRINTS) : print("district_id: ",district_id, "outside_district_labs: ",outside_district_transfer_labs,"outside_district_transfer_lab_locations :",outside_district_transfer_lab_locations)
        
    print("outside_district_lab_distances : ",valid)
    return valid

def evaluate(lab_info_filename,district_info_filename,solution_filename):
    df_labs = pd.read_csv(lab_info_filename).astype({"id":"int64","district_id":"int64","lab_type":"int64","capacity":"int64","backlogs":"int64"})
    df_districts = pd.read_csv(district_info_filename).astype({"district_id":"int64","samples":"int64"})
    df_solution = pd.read_csv(solution_filename).astype({"transfer_type":"int64","source":"int64","destination":"int64","samples_transferred":"int64"})
    
        
    valid_solution = True
    valid_solution = valid_solution and backlog_same_distrct_constraint(df_solution)
    valid_solution = valid_solution and all_district_samples_accounted(df_districts,df_solution)
    valid_solution = valid_solution and lab_capacity_accounted(df_labs,df_solution)
    valid_solution = valid_solution and outside_district_lab_distances(df_labs,df_solution)    
    
    if(not valid_solution):
        print("Constraints not met")
        return
    
    
    return valid_solution



# input_dir = 'test_files/'
input_dir = ''
lab_file_prefix = 'lab_test_data_'
district_file_prefix='district_test_data_'
solution_file_prefix = 'solution_'

for count in range(1,6):
    lab_input_filename=input_dir+lab_file_prefix+ "%03d" % count +".csv"
    district_input_filename = input_dir+district_file_prefix+ "%03d" % count +".csv"
    solution_filename = input_dir+solution_file_prefix+ "%03d" % count +".csv"
    

    valid_allocation = evaluate(lab_input_filename,district_input_filename,solution_filename)
    print(solution_filename , ": Constraints Met : ", valid_allocation)