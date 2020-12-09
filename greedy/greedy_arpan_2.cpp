#include <bits/stdc++.h>
using namespace std;
typedef long long int ll;

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
    vector<pair<int, int>> districts_rem;
    for (int i = 1; i <= num_districts; i++)
    {
        if (districts[i].samples > 0)
        {
            districts_rem.push_back({districts[i].samples, i});
        }
    }
    sort(districts_rem.rbegin(), districts_rem.rend());
    for (auto dr : districts_rem)
    {
        auto dno = dr.second;
        // cout << dr.first << " " << dr.second << '\n';
        int alpha = 0.5;
        int beta = 0.5;
        vector<pair<double, int>> considered_labs;
        for (auto lab : labz)
        {
            if (lab.capacity <= 0 || lab.district == dno)
                continue;
            double dist = calcdist(districts[dno].lat, districts[dno].lon, lab.lat, lab.lon);
            // int pen1 = max(0, lab.capacity - districts[dno].samples);
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
    for (int i = 1; i <= num_districts; i++)
    {
        if (districts[i].samples > 0)
        {
            // transactions.push_back({{1, districts[i].samples}, {i, i}});
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
    for (int i = 1; i <= num_districts; i++)
    {
        if (districts[i].samples > 0)
        {
            transactions.push_back({{1, districts[i].samples}, {i, i}});
        }
    }
}

signed main()
{
    ifstream distin, labin;
    // vector<string> data =
    string dist_data = "district_sample_data_00";
    string suf = ".csv";
    string lab_data = "lab_sample_data_00";
    vector<string> poss = {"1", "2", "3", "4", "5"};
    int num = 5;
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

    solve();
    for (int i = 0; i < transactions.size(); i++)
    {
        cout << transactions[i].first.first << " " << transactions[i].second.first << " " << transactions[i].second.second << " " << transactions[i].first.second << '\n';
    }

    // Format output as required
}
