#include <bits/stdc++.h>
using namespace std;
typedef long long int ll;

// Required constants

const double kmR = 6373.0;
const int N = 502, M = 52;
double factor = 1;

int num_districts = 0, num_labs = 0;

struct Lab
{
    int id;
    double lat, lon;
    int district, type, capacity, backlog;
} labz[N];

double toRadians(double degree)
{
    return (degree * M_PI) / 180;
}

struct District
{
    int id;
    string name;
    double lat, lon;
    int samples;
} districts[M];

// Utility function to allow file reading
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

// Calculator for distance between two points on earth
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

// Utility function to allow file reading
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

// Utility function to allow file reading
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
vector<pair<pair<int, int>, pair<int, int>>> transactions;

void solve()
{
    vector<int> assigned_amt(num_labs + 1, 0);
    for (int i = 1; i <= num_districts; i++)
    {
        // Assign all public labs
        for (int j = 1; j <= num_labs; j++)
        {
            if (labz[j].district == i && labz[j].type == 0)
            {
                int num = min(labz[j].capacity, districts[i].samples);
                if (num != 0)
                {
                    districts[i].samples -= num;
                    labz[j].capacity -= num;
                    transactions.push_back({{0, num}, {i, j}});
                }
            }
        }
        // Assign all private labs
        for (int j = 1; j <= num_labs; j++)
        {
            if (labz[j].district == i && labz[j].type == 1)
            {
                int num = min(labz[j].capacity, districts[i].samples);
                if (num != 0)
                {
                    districts[i].samples -= num;
                    labz[j].capacity -= num;
                    transactions.push_back({{0, num}, {i, j}});
                }
            }
        }
    }
    // Find districts for which number of samples are in excess of capacity
    vector<pair<int, int>> districts_rem;
    for (int i = 1; i <= num_districts; i++)
    {
        if (districts[i].samples > 0)
        {
            districts_rem.push_back({districts[i].samples, i});
        }
    }
    // Sort districts according to descending order of number of samples are in excess of capacity
    sort(districts_rem.rbegin(), districts_rem.rend());
    // Find lab closest to the district and incurring least cost and send as many samples as possible
    for (auto dr : districts_rem)
    {
        auto dno = dr.second;
        vector<pair<double, int>> considered_labs;
        for (auto lab : labz)
        {
            if (lab.capacity <= 0 || lab.district == dno)
                continue;
            double dist = calcdist(districts[dno].lat, districts[dno].lon, lab.lat, lab.lon);
            int pen1 = max(0, districts[dno].samples - lab.capacity);
            double tot_pen = dist * 1000 + pen1 * 5000 + (districts[dno].samples - pen1) * ((lab.type == 0) ? 800 : 1600);
            considered_labs.push_back({tot_pen, lab.id});
        }
        if (considered_labs.size() == 0)
            continue;
        sort(considered_labs.begin(), considered_labs.end());
        int lab_choice = considered_labs[0].second;
        int tamt = min(labz[lab_choice].capacity, districts[dno].samples);
        districts[dno].samples -= tamt;
        labz[lab_choice].capacity -= tamt;
        transactions.push_back({{0, tamt}, {dno, lab_choice}});
    }
    // Overload the labs within the district now
    for (int i = 1; i <= num_districts; i++)
    {
        if (districts[i].samples > 0)
        {
            for (auto lab : labz)
            {
                if (lab.district != i || lab.capacity <= -100)
                    continue;
                int tamt = min(lab.capacity + 100, districts[i].samples);
                transactions.push_back({{0, tamt}, {i, lab.id}});
                lab.capacity -= tamt;
                districts[i].samples -= tamt;
                if (districts[i].samples == 0)
                    break;
            }
        }
    }
    // Send to backlog for the remaining swabs
    for (int i = 1; i <= num_districts; i++)
    {
        if (districts[i].samples > 0)
        {
            transactions.push_back({{1, districts[i].samples}, {i, i}});
        }
    }
}

signed main(int argc, char* argv[])
{
    // Reader for input files
    ifstream distin, labin;
    // string dist_data = "district_sample_data_00";
    string dist_data = "district_test_data_00";
    string suf = ".csv";
    // string lab_data = "lab_sample_data_00";
    string lab_data = "lab_test_data_00";
    vector<string> poss = {"1", "2", "3", "4", "5"};
    int num = stoi(argv[1]);
    distin.open(dist_data + poss[num - 1] + suf);
    labin.open(lab_data + poss[num - 1] + suf);
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

    solve();

    // Print into csv
    cout << "transfer_type,source,destination,samples_transferred\n";
    for (int i = 0; i < transactions.size(); i++)
    {
        cout << transactions[i].first.first << "," << transactions[i].second.first << "," << transactions[i].second.second << "," << transactions[i].first.second << '\n';
    }
}
