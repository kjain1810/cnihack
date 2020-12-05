#include <bits/stdc++.h>

using namespace std;

const double kmR = 6373.0;
const int N = 502, M = 52;
const int MAX_TRAVEL = 1000;

int num_districts = 0, num_labs = 0;
int num_allotments = 0;

struct Lab
{
    int id;
    double lat, lon;
    int district, type, capacity, backlog;
    int initcapacity;
    double demand;
} labz[N];

struct District
{
    int id;
    string name;
    double lat, lon;
    int samples;
    int total_capacity;
    int extraslots;
} districts[M];

struct Allotment
{
    int districtID;
    int labID;
    int samples;
    int type;
} allotments[N * M];

string get_parts(string &line, int &i)
{
    string word;
    while (i < line.length())
    {
        if (line[i] == ',')
        {
            i++;
            break;
        }
        else
            word += line[i];
        i++;
    }
    return word;
}

void get_lab(string &line, Lab &lab)
{
    int i = 0;
    lab.id = stoi(get_parts(line, i));
    lab.lat = stod(get_parts(line, i));
    lab.lon = stod(get_parts(line, i));
    lab.district = stoi(get_parts(line, i));
    lab.type = stoi(get_parts(line, i));
    lab.initcapacity = lab.capacity = stoi(get_parts(line, i));
    lab.backlog = stoi(get_parts(line, i));
    districts[lab.district].total_capacity += lab.capacity;
}

void get_district(string &line, District &d)
{
    int i = 0;
    d.id = stoi(get_parts(line, i));
    d.name = get_parts(line, i);
    d.lat = stod(get_parts(line, i));
    d.lon = stod(get_parts(line, i));
    d.samples = stoi(get_parts(line, i));
    d.total_capacity = 0;
}

void getinput()
{
    ifstream distin, labin;
    distin.open("districts_data_v0.csv");
    labin.open("lab_data_v0.csv");
    string line;

    getline(distin, line);
    while (getline(distin, line))
    {
        get_district(line, districts[++num_districts]);
    }

    getline(labin, line);
    while (getline(labin, line))
    {
        get_lab(line, labz[++num_labs]);
    }

    // for (int i = 1; i <= num_districts; i++)
    // {
    //     cout << districts[i].id << " " << districts[i].name << " " << districts[i].lat << " ";
    //     cout << districts[i].lon << " " << districts[i].samples << endl;
    // }
    // for (int i = 1; i <= num_labs; i++)
    // {
    //     cout << labz[i].id << " " << labz[i].lat << " " << labz[i].lon << " " << labz[i].district << " ";
    //     cout << labz[i].type << " " << labz[i].capacity << " " << labz[i].backlog << endl;
    // }
}

double toRadians(double degree)
{
    return (degree * M_PI) / 180;
}

double calcdist(double lat1d, double lon1d, double lat2d, double lon2d)
{
    double lat1r, lon1r, lat2r, lon2r, u, v;
    lat1r = toRadians(lat1d);
    lon1r = toRadians(lon1d);
    lat2r = toRadians(lat2d);
    lon2r = toRadians(lon2d);
    u = sin((lat2r - lat1r) / 2);
    v = sin((lon2r - lon1r) / 2);
    return 2.0 * kmR * asin(sqrt(u * u + cos(lat1r) * cos(lat2r) * v * v));
}

void getdifferences()
{
    for (int a = 1; a <= num_districts; a++)
        districts[a].extraslots = districts[a].total_capacity - districts[a].samples;
}

void calculatelabdemand()
{
    for (int a = 1; a <= num_labs; a++)
    {
        for (int b = 1; b <= num_districts; b++)
        {
            if (labz[a].district == districts[b].id)
                continue;
            if (districts[b].extraslots >= 0)
                continue;
            double disthere = calcdist(labz[a].lat, labz[a].lon, districts[b].lat, districts[b].lon);
            if (disthere <= MAX_TRAVEL)
                labz[a].demand += disthere;
        }
    }
}

bool comparelabsdemand(Lab lab1, Lab lab2)
{
    return lab1.demand <= lab2.demand;
}

void allot(int labid, int districtid, int capacity, int type)
{
    allotments[num_allotments].labID = labid;
    allotments[num_allotments].districtID = districtid;
    allotments[num_allotments].samples = capacity;
    allotments[num_allotments].type = type;
    num_allotments++;
}

void fillintralabs()
{
    for (int a = 1; a <= num_districts; a++)
    {
        vector<Lab> labshere;
        for (int b = 1; b <= num_labs; b++)
        {
            if (districts[a].id != labz[b].district)
                continue;
            labshere.push_back(labz[b]);
        }
        if (districts[a].extraslots > 0)
        {
            sort(labshere.begin(), labshere.end(), comparelabsdemand);
            for (auto lab : labshere)
            {
                if (districts[a].samples == 0)
                    break;
                allot(lab.id, districts[a].id, min(districts[a].samples, lab.capacity), 0);
                int gone = min(districts[a].samples, lab.capacity);
                districts[a].samples -= lab.capacity;
                lab.capacity -= gone;
            }
        }
        else
        {
            for (auto lab : labshere)
            {
                allot(lab.id, districts[a].id, lab.capacity, 0);
                districts[a].samples -= lab.capacity;
                lab.capacity = 0;
            }
        }
    }
}

int sourcedistrict;
bool comparelabidxdistr(int a, int b)
{
    return calcdist(labz[a].lat, labz[a].lon, districts[sourcedistrict].lat, districts[sourcedistrict].lon) <=
           calcdist(labz[b].lat, labz[b].lon, districts[sourcedistrict].lat, districts[sourcedistrict].lon);
}

void fillinterlabs()
{
    for (int a = 1; a <= num_districts; a++)
    {
        if (districts[a].samples <= 0)
            continue;
        vector<int> labidx;
        for (int b = 1; b <= num_labs; b++)
        {
            if (labz[b].district == districts[a].id)
                continue;
            labidx.push_back(b);
        }
        sourcedistrict = a;
        sort(labidx.begin(), labidx.end(), comparelabidxdistr);
        for (auto idx : labidx)
        {
            if (districts[a].samples <= 0)
                break;
            int sending = min(min(100, labz[idx].initcapacity), districts[a].samples);
            allot(labz[idx].id, districts[a].id, sending, 0);
            labz[idx].capacity -= sending;
            districts[a].samples -= sending;
        }
    }
}

void checkdistrictbacklogs()
{
    for (int a = 1; a <= num_districts; a++)
        if (districts[a].samples > 0)
            allot(districts[a].id, districts[a].id, districts[a].samples, 1);
}

void saveallotments()
{
    // freopen("output.csv", "w", stdout);
    // cout << "transfer_type,source,destination,samples transferred\n";
    for (int a = 0; a < num_allotments; a++)
    {
        cout << allotments[a].type << " " << allotments[a].districtID << " " << allotments[a].labID << " " << allotments[a].samples << "\n";
    }
}

signed main()
{
    getinput();
    getdifferences();
    calculatelabdemand();
    fillintralabs();
    fillinterlabs();
    checkdistrictbacklogs();
    saveallotments();
    return 0;
}