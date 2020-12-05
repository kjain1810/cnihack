#include <bits/stdc++.h>

using namespace std;

const double kmR = 6373.0;
const int N = 502, M = 52, K = 30;

int num_districts = 0, num_labs = 0;

struct Lab
{
    int id;
    double lat, lon;
    int district, type, capacity, backlog;
    int booked = 0, overloadable = 0;
} labz[N];

struct District{
    int id;
    string name;
    double lat, lon;
    int samples, left;
} districts[M];

double Lmat[N][N];
int vis[N];
string get_parts(string &line, int &i){
    string word;
    while(i < line.length())
    {
        if(line[i]==',')
        {
            i++;
            break;
        }
        else word+=line[i];
        i++;
    }
    return word;
}

void get_lab(string &line, Lab &lab){
    int i = 0;
    lab.id = stoi(get_parts(line, i));
    lab.lat = stod(get_parts(line, i));
    lab.lon = stod(get_parts(line, i));
    lab.district = stoi(get_parts(line, i));
    lab.type = stoi(get_parts(line, i));
    lab.capacity = stoi(get_parts(line, i));
    lab.backlog = stoi(get_parts(line, i));
}

void get_district(string &line, District &d){
    int i = 0;
    d.id = stoi(get_parts(line, i));
    d.name = get_parts(line, i);
    d.lat = stod(get_parts(line, i));
    d.lon = stod(get_parts(line, i));
    d.samples = stoi(get_parts(line, i));
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

void get_least(){
    for(int i = 1; i <= num_districts; i++){
        for(int j = 1; j <= num_labs; j++){
            
        }
    }
}

void comp_lab_mat(){
    for(int i = 1; i <= num_labs; i++){
        for(int j = 1; j <= num_labs; j++){
            Lmat[i][j] = calcdist(labz[i].lat, labz[i].lon, labz[j].lat, labz[j].lon);
        }
    }
}

struct sortbysz{
    bool operator()(vector<int> &a, vector<int> &b){
        return a.size() > b.size();
    }
};

void get_cliques(vector<vector<int>> &cliques, int szleft, vector<int> curr_nodes, int last){
    if(szleft==0){
        cliques.push_back(curr_nodes);
        return;
    }
    int flag=0, subset=0;
    for(int i = last+1; i <= num_labs; i++){
        flag = i;
        for(auto j: curr_nodes){
            if(Lmat[i][j] > 40){
                flag=0;
                break;
            }
        }
        if(flag!=0){   
            subset=1; 
            //cerr << flag << " ";
            curr_nodes.push_back(flag);
            get_cliques(cliques, szleft-1, curr_nodes, flag);
            curr_nodes.pop_back();
        }
    }
    if(subset==0){
        cliques.push_back(curr_nodes);
    }
}

signed main(){
    ifstream distin, labin;
    distin.open("districts_data_v0.csv");
    labin.open("lab_data_v0.csv");
    string line;

    getline(distin, line);
    while(getline(distin, line)){
        get_district(line, districts[++num_districts]);
        districts[num_districts].left = districts[num_districts].samples;
    }
    
    getline(labin, line);
    while(getline(labin, line)){
        get_lab(line, labz[++num_labs]);
    }

    comp_lab_mat();
    
    vector<vector<int>> cliques;
    vector<int> curr_nodes;
    get_cliques(cliques, 4, curr_nodes, 0);
    
    int num_cliques = cliques.size();
    cout << num_cliques << endl;
    vector<vector<int>> selected;
    sort(cliques.begin(), cliques.end(), sortbysz());

    ofstream fout;
/*    fout << num_cliques << endl;
    for(int i = 0; i < num_cliques; i++){
        fout << cliques[i].size() << " ";
        for(auto j: cliques[i]){
            fout << j << " ";
        }
        fout << endl;
    }*/

    for(int i = 0; i < num_cliques; i++){
        bool flag = 1;
        for(auto j: cliques[i]){
            if(vis[j]>=K){
                flag = 0;
                break;
            }
        }
        if(flag==0) continue;
        selected.push_back(cliques[i]);
        for(auto j: cliques[i]){
            vis[j]++;
        }
    }
    int num_selected = selected.size();
    fout.open("Centroidsv0.txt");
    fout << num_selected << endl;
    for(int i = 0; i < num_selected; i++){
        fout << selected[i].size() << " ";
        for(auto j: selected[i]){
            fout << j << " ";
        }
        fout << endl;
    }
    //get_least();

    /*for(int i = 1; i <= num_districts; i++){
        cout << districts[i].id << " " << districts[i].name << " " << districts[i].lat << " ";
        cout << districts[i].lon << " " << districts[i].samples <<  endl;
    }*/
    /*for(int i = 1; i <= num_labs; i++){
        cout << labz[i].id << " " << labz[i].lat << " " << labz[i].lon << " " << labz[i].district << " ";
        cout << labz[i].type << " " << labz[i].capacity << " " << labz[i].backlog << endl;
    }*/
}