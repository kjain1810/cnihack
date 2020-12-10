#include <bits/stdc++.h>

using namespace std;

const double kmR = 6373.0;
const int N = 502, M = 52, K = 50;

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

struct sortbycap{
    bool operator()(vector<int> &a, vector<int> &b){
        int sa = 0, sb = 0;
        for(auto j: a) sa += labz[j].capacity;
        for(auto j: b) sb += labz[j].capacity;
        return sa > sb;
    }
};

void get_cliques(vector<vector<int>> &cliques, int szleft, vector<int> curr_nodes, int last, vector<int> &nodeslist){
    if(szleft==0){
        cliques.push_back(curr_nodes);
        return;
    }
    int flag=0, subset=0;
    for(int i = last+1; i < nodeslist.size(); i++){
        flag = nodeslist[i];
        for(auto j: curr_nodes){
            if(Lmat[flag][j] > 40){
                flag=0;
                break;
            }
        }
        if(flag!=0){   
            //subset=1; 
            //cerr << flag << " ";
            curr_nodes.push_back(flag);
            get_cliques(cliques, szleft-1, curr_nodes, i, nodeslist);
            curr_nodes.pop_back();
        }
    }
    if(subset==0 && curr_nodes.size()>0){
        cliques.push_back(curr_nodes);
    }
}

vector<int> printFreq(vector<vector<int>> &cliques){
    vector<int> freq(15, 0);
    for(int i = 0; i < cliques.size(); i++){
        freq[cliques[i].size()]++;
    }
    return freq;
}

void comp_max_blr_urban(){
    int max = 0;
    for(int i = 1; i <= num_labs; i++){
        for(int j = i+1; j <= num_labs; j++){
            if(labz[i].district!=5 || labz[j].district!=5) continue;
            double distij = calcdist(labz[i].lat, labz[i].lon, labz[j].lat, labz[j].lon);
            if(distij > max){
                cout << i << " " << j << " - " << distij << endl;
                max = distij;
            }
        }
    }
}

int comp_inter_district(){
    int num_edges = 0;
    for(int i = 1; i <= num_labs; i++){
        for(int j = i+1; j <= num_labs; j++){
            if(labz[i].district==labz[j].district) continue;
            double distij = calcdist(labz[i].lat, labz[i].lon, labz[j].lat, labz[j].lon);
            //cout << i << " " << j << endl;
            if(distij < 40){
                cout << i << " " << j << " - " << labz[i].district << " " << labz[j].district << " - " << distij << endl;
                num_edges++;
            }
        }
    }
    return num_edges;
}

vector<double> comp_maxclique_diameters(vector<vector<int>> &components, vector<int> &comp_of){
    int num_components = components.size();
    vector<double> max_pair_dist(num_components+1, 0);
    for(int i = 1; i <= num_labs; i++){
        for(int j = i+1; j <= num_labs; j++){
            int centroid_i = comp_of[i], centroid_j = comp_of[j];
            if(centroid_i != centroid_j) continue;
            double distij = Lmat[i][j];
            //cout << i << " " << j << " - " << labz[i].district << " " << labz[j].district << " - " << distij << endl;
            max_pair_dist[centroid_i] = max(max_pair_dist[centroid_i], distij);
        }
    }
    return max_pair_dist;
}

void select_uptoK_per_lab(vector<vector<int>> &selected, vector<vector<int>> &cliques){
    int num_cliques = cliques.size();
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
    for(int i = 1; i <= num_labs; i++){
        if(vis[i]==0){
            selected.push_back(vector<int>(1, i));
            vis[i]++;
        }
    }
}

void print_centroids(vector<vector<int>> &selected, ofstream &fout, string file_name){
    int num_selected = selected.size();
    fout.open(file_name);
    fout << num_selected << endl;
    for(int i = 0; i < num_selected; i++){
        fout << selected[i].size() << " ";
        for(auto j: selected[i]){
            fout << j << " ";
        }
        fout << endl;
    }
}

void DFS(int u, vector<bool> &vis, vector<int> &component){
    vis[u] = true;
    component.push_back(u);
    for(int i = 1; i <= num_labs; i++){
        if(!vis[i] && Lmat[u][i] < 40){
            DFS(i, vis, component);
        }
    }
}

void get_components(vector<vector<int>> &components, vector<int> &comp_of){
    vector<bool> vis(num_labs+1, 0);
    for(int i = 1; i <= num_labs; i++){
        if(!vis[i]){
            vector<int> tmp;
            DFS(i, vis, tmp);
            components.push_back(tmp);
            for(auto j: tmp){
                comp_of[j] = components.size()-1;
            }
        }
    }
}

signed main(){
    ifstream distin, labin;
    distin.open("Data/districts_data_v0.csv");
    labin.open("Data/lab_data_v0.csv");
    string line;

    ofstream fout;
    getline(distin, line);
    while(getline(distin, line)){
        get_district(line, districts[++num_districts]);
        districts[num_districts].left = districts[num_districts].samples;
    }
    cerr << num_districts << endl;

    getline(labin, line);
    while(getline(labin, line)){
        get_lab(line, labz[++num_labs]);
    }

    comp_lab_mat();

    cout << comp_inter_district() << endl;

    vector<vector<int>> components;
    vector<int> comp_of(num_labs+1);
    get_components(components, comp_of);
    print_centroids(components, fout, "Centroids/Centroidsboundv0.txt");
    //comp_max_blr_urban();
    vector<double> max_pair_dist = comp_maxclique_diameters(components, comp_of);
    int centroid_shift_cost = 0;
    for(auto j: max_pair_dist){
        cout << j << " ";
        centroid_shift_cost += j*1000;
    }
    cout << endl << centroid_shift_cost << endl;
    return 0;

    vector<vector<int>> cliques;
    vector<int> curr_nodes;
    vector<int> bangalore, total;
    for(int i = 1; i <= num_labs; i++){
        total.push_back(i);
        if(labz[i].district == 5) bangalore.push_back(i);
    }
    /*cout << bangalore.size() << endl;
    vector<int> tempv;
    get_cliques(cliques, 4, curr_nodes, 0, bangalore); //get for overall size 5
    tempv = printFreq(cliques);
    for(int i = 1; i <= 8; i++){
        cout << i << " " << tempv[i] << endl; 
    }
    int temp = cliques.size();
    cout << temp << endl;
    return 0;*/
    get_cliques(cliques, 4, curr_nodes, -1, total); //get for bangalore size 8
    int num_cliques = cliques.size();
    cout << num_cliques << endl;
    //cout << temp << " " << num_cliques - temp << endl;
    vector<vector<int>> selected;
    sort(cliques.begin(), cliques.end(), sortbycap());
    
    for(int i = 0; i < num_cliques; i++){
        selected.push_back(cliques[i]);
    }
    
    print_centroids(selected, fout, "Centroids/Centroidsv11.txt");
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