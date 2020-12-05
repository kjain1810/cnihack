#include "../cpputils.hpp"
#include <bits/stdc++.h>

using namespace std;

const int MAX_TRAVEL = 1000;

int num_allotments = 0;

struct Allotment
{
    int districtID;
    int labID;
    int samples;
    int type;
} allotments[N * M];

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

bool comparelabsdemand(int lab1, int lab2)
{
    return labz[lab1].demand <= labz[lab2].demand;
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
        vector<int> labsidx;
        for (int b = 1; b <= num_labs; b++)
        {
            if (districts[a].id != labz[b].district)
                continue;
            labsidx.push_back(b);
        }
        if (districts[a].extraslots > 0)
        {
            sort(labsidx.begin(), labsidx.end(), comparelabsdemand);
            for (auto labidx : labsidx)
            {
                if (districts[a].samples <= 0)
                    break;
                int gone = min(districts[a].samples, labz[labidx].capacity);
                allot(labz[labidx].id, districts[a].id, gone, 0);
                districts[a].samples -= gone;
                labz[labidx].capacity -= gone;
            }
        }
        else
        {
            for (auto lab : labsidx)
            {
                if (districts[a].samples == 0)
                    break;
                allot(labz[lab].id, districts[a].id, labz[lab].capacity, 0);
                districts[a].samples -= labz[lab].capacity;
                labz[lab].capacity = 0;
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
            if (labz[idx].capacity <= -100)
                continue;
            int sending = min(min(100, labz[idx].initcapacity), districts[a].samples);
            if (labz[idx].capacity - sending < -100)
                continue;
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
    freopen("transactions.out", "w", stdout);
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