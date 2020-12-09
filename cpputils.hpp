#ifndef CPP_UTILS
#define CPP_UTILS

#include <bits/stdc++.h>

const int N = 502, M = 52;
int num_districts = 0, num_labs = 0;

struct Lab
{
    int id;
    double lat, lon;
    int district, type, capacity, backlog;
    int initcapacity;
    double demand;
};

struct District
{
    int id;
    std::string name;
    double lat, lon;
    int samples;
    int total_capacity;
    int extraslots;
};

Lab labz[N];
District districts[M];

std::string get_parts(std::string &line, int &i)
{
    std::string word;
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

void get_lab(std::string &line, Lab &lab)
{
    int i = 0;
    lab.id = stoi(get_parts(line, i));
    lab.id = stoi(get_parts(line, i));
    lab.lat = stod(get_parts(line, i));
    lab.lon = stod(get_parts(line, i));
    lab.district = stoi(get_parts(line, i));
    lab.type = stoi(get_parts(line, i));
    lab.initcapacity = lab.capacity = stoi(get_parts(line, i));
    lab.backlog = stoi(get_parts(line, i));
    districts[lab.district].total_capacity += lab.capacity;
}

void get_district(std::string &line, District &d)
{
    int i = 0;
    d.id = stoi(get_parts(line, i));
    d.id = stoi(get_parts(line, i));
    d.name = get_parts(line, i);
    d.lat = stod(get_parts(line, i));
    d.lon = stod(get_parts(line, i));
    d.samples = stoi(get_parts(line, i));
    d.total_capacity = 0;
}

void getinput(char *distpath, char *labpath)
{
    std::ifstream distin, labin;
    distin.open(distpath);
    labin.open(labpath);
    std::string line;

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
}

const double kmR = 6373.0;

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

#endif