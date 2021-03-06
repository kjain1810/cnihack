#include <bits/stdc++.h>

using namespace std;

const double kmR = 6373.0;
const int N = 502, M = 52;
double factor = 1;

// #include <math.h>

#define pi 3.14159265358979323846

/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
/*::  Function prototypes                                           :*/
/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
double deg2rad(double);
double rad2deg(double);

double distance(double lat1, double lon1, double lat2, double lon2, char unit)
{
    double theta, dist;
    if ((lat1 == lat2) && (lon1 == lon2))
    {
        return 0;
    }
    else
    {
        theta = lon1 - lon2;
        dist = sin(deg2rad(lat1)) * sin(deg2rad(lat2)) + cos(deg2rad(lat1)) * cos(deg2rad(lat2)) * cos(deg2rad(theta));
        dist = acos(dist);
        dist = rad2deg(dist);
        dist = dist * 60 * 1.1515;
        switch (unit)
        {
        case 'M':
            break;
        case 'K':
            dist = dist * 1.609344;
            break;
        case 'N':
            dist = dist * 0.8684;
            break;
        }
        return (dist);
    }
}

/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
/*::  This function converts decimal degrees to radians             :*/
/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
double deg2rad(double deg)
{
    return (deg * pi / 180);
}

/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
/*::  This function converts radians to decimal degrees             :*/
/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
double rad2deg(double rad)
{
    return (rad * 180 / pi);
}

int num_districts = 0, num_labs = 0;

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

struct Lab
{
    int id;
    double lat, lon;
    int district, type, capacity, backlog;
} labz[N];

struct District
{
    int id;
    string name;
    double lat, lon;
    int samples;
} districts[M];

vector<pair<pair<int, int>, pair<int, int>>> transactions;

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

string get_words(string &line, int &i)
{
    string word;
    while (i < line.length())
    {
        if (line[i] == ' ')
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
    lab.id = stoi(get_parts(line, i));
    lab.lat = stod(get_parts(line, i));
    lab.lon = stod(get_parts(line, i));
    lab.district = stoi(get_parts(line, i));
    lab.type = stoi(get_parts(line, i));
    lab.capacity = stoi(get_parts(line, i));
    lab.backlog = stoi(get_parts(line, i));
    lab.capacity -= lab.backlog;
}

void get_district(string &line, District &d)
{
    int i = 0;
    d.id = stoi(get_parts(line, i));
    d.id = stoi(get_parts(line, i));
    d.name = get_parts(line, i);
    d.lat = stod(get_parts(line, i));
    d.lon = stod(get_parts(line, i));
    d.samples = (stoi(get_parts(line, i)) * factor);
}

pair<pair<int, int>, pair<int, int>> get_trans(string &line)
{
    int i = 0;
    pair<pair<int, int>, pair<int, int>> ans;
    ans.first.first = stoi(get_words(line, i));
    ans.first.second = stoi(get_words(line, i));
    ans.second.first = stoi(get_words(line, i));
    ans.second.second = stoi(get_words(line, i));
    return ans;
}

double score()
{
    double totcost = 0;
    set<int> overloaded_labs;
    vector<vector<pair<pair<int, int>, pair<int, int>>>> district_transfer(num_districts + 1);

    vector<int> lab_capacity(num_labs + 1, 0);
    vector<int> outside_transfer_to_lab(num_labs + 1, 0);
    for (int i = 1; i <= num_labs; i++)
    {
        lab_capacity[i] = labz[i].capacity;
    }

    for (auto tr : transactions)
    {
        int dist = tr.first.second;
        int lab = tr.second.first;
        int amt = tr.second.second;
        if (tr.first.first == 0)
        {
            // if (dist == 1)
            //     cout << lab << " ++ ";
            if (dist != labz[lab].district)
            {
                district_transfer[dist].push_back(tr);
                // continue;
            }
            // if (dist == 1)
            //     cout << lab << " ++ ";
            if (districts[dist].samples < amt)
            {
                cout << "No samples left to send\n";
                exit(0);
            }
            // if (totcost == 83780000.0)
            // {
            //     cout << " POOO";
            //     // cout << lab_cost << " POOO";
            // }
            if (labz[lab].capacity >= amt)
            {
                int lab_cost = (labz[lab].type == 0) ? 800 : 1600;
                labz[lab].capacity -= amt;
                districts[dist].samples -= amt;
                totcost += amt * lab_cost;
            }
            else if (labz[lab].capacity + 100 >= amt)
            {
                int lab_cost = (labz[lab].type == 0) ? 800 : 1600;
                int init_amt = max(0, labz[lab].capacity);
                int over_amt = amt - init_amt;
                districts[dist].samples -= amt;
                labz[lab].capacity -= amt;
                totcost += lab_cost * init_amt + (5000 + lab_cost) * over_amt;
                overloaded_labs.insert(lab);
            }
            else
            {
                cout << "Lab cant overload further\n";
                exit(0);
            }
        }
        else
        {
            if (districts[dist].samples < amt)
            {
                cout << "No samples left to carry over\n";
                exit(0);
            }
            districts[dist].samples -= amt;
            totcost += 10000 * amt;
        }
        // cout << totcost << '\n';
    }
    // cout << totcost << '\n';
    for (int i = 1; i <= num_districts; i++)
    {
        int cnt = 0;
        double lat = 0;
        double lon = 0;
        for (auto tr : district_transfer[i])
        {
            int dist = tr.first.second;
            int lab = tr.second.first;
            int amt = tr.second.second;
            cnt++;
            lat += labz[lab].lat;
            lon += labz[lab].lon;
            for (auto tr2 : district_transfer[i])
            {
                int lab2 = tr2.second.first;
                if (distance(labz[lab].lat, labz[lab].lon, labz[lab2].lat, labz[lab2].lon, 'K') > 40)
                {
                    cout << "District " << dist << " sending samples to labs " << lab << " and " << lab2 << " which are more than 40km apart\n";
                    exit(0);
                }
            }
        }
        if (cnt == 0)
            continue;
        lat = lat / (double(cnt));
        lon = lon / (double(cnt));
        double dist = distance(districts[i].lat, districts[i].lon, lat, lon, 'K');
        totcost += 1000 * dist;
    }

    for (auto tr : transactions)
    {
        int dist = tr.first.second;
        int lab = tr.second.first;
        int amt = tr.second.second;
        // if (overloaded_labs.find(lab) != overloaded_labs.end())
        // {
        //     if (labz[lab].district != dist)
        //     {
        //         cout << "Cant overload a lab from another district\n";
        //         exit(0);
        //     }
        // }
        if (tr.first.first != 0)
            continue;
        if (dist != labz[lab].district)
        {
            outside_transfer_to_lab[lab] += amt;
            if (outside_transfer_to_lab[lab] > lab_capacity[lab])
            {
                cout << "Cant overload a lab from another district\n";
                cout << "Lab " << lab << " overloaded by outside districts\n";
                cout << "Lab capacity - " << lab_capacity[lab] << '\n';
                exit(0);
            }
        }
        // if (dist == 1)
        // {
        //     cout << districts[1].samples << '\n';
        // }
    }

    for (int i = 1; i <= num_districts; i++)
    {
        if (districts[i].samples > 0)
        {
            // cout << i << " " << districts[i].samples << '\n';
            cout << "Unallocated swabs!\n";
            exit(0);
        }
    }
    return totcost;
}

signed main()
{

    ifstream trans;
    trans.open("transactions.out"); // Change this
    string line;
    while (getline(trans, line))
    {
        transactions.push_back(get_trans(line));
    }
    for (int i = 0; i < transactions.size(); i++)
    {
        cout << transactions[i].first.first << " " << transactions[i].first.second << " " << transactions[i].second.first << " " << transactions[i].second.second << '\n';
    }

    ifstream distin, labin;
    string dist_data = "district_sample_data_00";
    string suf = ".csv";
    string lab_data = "lab_sample_data_00";
    vector<string> poss = {"1", "2", "3", "4", "5"};
    int num = 1;
    distin.open(dist_data + poss[num - 1] + suf);
    labin.open(lab_data + poss[num - 1] + suf);
    // string line;

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

    cout << "Your Score is = " << fixed << setprecision(10) << score() << '\n';
}