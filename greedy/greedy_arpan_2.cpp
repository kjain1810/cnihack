#include <bits/stdc++.h>
using namespace std;
typedef long long int ll;

const double kmR = 6373.0;
const int N = 502, M = 52;

int num_districts = 0, num_labs = 0;

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
    lab.capacity = stoi(get_parts(line, i));
    lab.backlog = stoi(get_parts(line, i));
}

void get_district(string &line, District &d)
{
    int i = 0;
    d.id = stoi(get_parts(line, i));
    d.name = get_parts(line, i);
    d.lat = stod(get_parts(line, i));
    d.lon = stod(get_parts(line, i));
    d.samples = stoi(get_parts(line, i));
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
        cout << dr.first << " " << dr.second << '\n';
        // for (auto lab : labz)
        // {
        //     for (auto lab : labz)
        //     {
        //     }
        // }
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

    solve();
    for (int i = 0; i < transactions.size(); i++)
    {
        cout << transactions[i].first.first << " " << transactions[i].second.first << " " << transactions[i].second.second << " " << transactions[i].first.second << '\n';
    }

    // Format output as required
}
