#include <bits/stdc++.h>

using namespace std;

const double kmR = 6373.0; //constant for distance calculation
const int N = 502, M = 52, K = 50;
//N=Max No. of Labs, M = Max No. of Districts, K = Max No. of time lab can come in pre-selected clusters 

int num_districts = 0, num_labs = 0; //total no. of districts and labs in input

//Structure for maintaining lab details
struct Lab
{
    int id;
    double lat, lon;
    int district, type, capacity, backlog;
    int booked = 0, overloadable = 0;
} labz[N];

//Structure for maintaining district details
struct District{
    int id;
    string name;
    double lat, lon;
    int samples, left;
} districts[M];

double Lmat[N][N]; //Matrix for storing lab distances
int vis[N]; //Labs visited in DFS on G_c

//Get one value from CSV
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

//Get details of a lab from CSV
void get_lab(string &line, Lab &lab){
    int i = 0;
    int index = stoi(get_parts(line, i));
    lab.id = stoi(get_parts(line, i));
    lab.lat = stod(get_parts(line, i));
    lab.lon = stod(get_parts(line, i));
    lab.district = stoi(get_parts(line, i));
    lab.type = stoi(get_parts(line, i));
    lab.capacity = stoi(get_parts(line, i));
    lab.backlog = stoi(get_parts(line, i));
}

//Get details of a district from CSV
void get_district(string &line, District &d){
    int i = 0;
    int index = stoi(get_parts(line, i));
    d.id = stoi(get_parts(line, i));
    d.name = get_parts(line, i);
    d.lat = stod(get_parts(line, i));
    d.lon = stod(get_parts(line, i));
    d.samples = stoi(get_parts(line, i));
}

//Convert degrees to radians
double to_radians(double degree)
{
    return (degree * M_PI) / 180;
}

//Calculate distance between 2 points
double calc_dist(double lat1d, double lon1d, double lat2d, double lon2d)
{
    double lat1r, lon1r, lat2r, lon2r, u, v;
    lat1r = to_radians(lat1d);
    lon1r = to_radians(lon1d);
    lat2r = to_radians(lat2d);
    lon2r = to_radians(lon2d);
    u = sin((lat2r - lat1r) / 2);
    v = sin((lon2r - lon1r) / 2);
    return 2.0 * kmR * asin(sqrt(u * u + cos(lat1r) * cos(lat2r) * v * v));
}

//Print all district details in input
void print_districts(){
    for(int i = 1; i <= num_districts; i++){
        cout << districts[i].id << " " << districts[i].name << " " << districts[i].lat << " ";
        cout << districts[i].lon << " " << districts[i].samples <<  endl;
    }
}

//Print all lab details in input
void print_labs(){
    for(int i = 1; i <= num_labs; i++){
        cout << labz[i].id << " " << labz[i].lat << " " << labz[i].lon << " " << labz[i].district << " ";
        cout << labz[i].type << " " << labz[i].capacity << " " << labz[i].backlog << endl;
    }
}

//Compute matrix of pairwise lab distances (Lmat)
void comp_lab_mat(){
    for(int i = 1; i <= num_labs; i++){
        for(int j = 1; j <= num_labs; j++){
            Lmat[i][j] = calc_dist(labz[i].lat, labz[i].lon, labz[j].lat, labz[j].lon);
        }
    }
}

//Sort cluster of labs by capacity comparator
struct sortbycap{
    bool operator()(vector<int> &a, vector<int> &b){
        int sa = 0, sb = 0;
        for(auto j: a) sa += labz[j].capacity;
        for(auto j: b) sb += labz[j].capacity;
        return sa > sb;
    }
};

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

//Backtracking search to compute cliques
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
            //subset=1; //Uncomment to prune subsets
            curr_nodes.push_back(flag);
            get_cliques(cliques, szleft-1, curr_nodes, i, nodeslist);
            curr_nodes.pop_back();
        }
    }

    if(subset==0 && curr_nodes.size()>0){
        cliques.push_back(curr_nodes);
    }
}

//Print frequency of a particular size of cliques
void print_freq(vector<vector<int>> &cliques, int max_num){
    vector<int> freq(max_num+1, 0);
    for(int i = 0; i < cliques.size(); i++){
        freq[cliques[i].size()]++;
    }
    cout << "<size> <no. of cliques of size>" << endl;
    for(int i = 1; i <= max_num; i++){
        cout << i << " " << freq[i] << endl; 
    }
}

//Empirical - Compute maximum distance between labs in BLR Urban
void comp_max_blr_urban(){
    int max = 0;
    for(int i = 1; i <= num_labs; i++){
        for(int j = i+1; j <= num_labs; j++){
            if(labz[i].district!=5 || labz[j].district!=5) continue;
            double distij = calc_dist(labz[i].lat, labz[i].lon, labz[j].lat, labz[j].lon);
            if(distij > max){
                cout << i << " " << j << " - " << distij << endl;
                max = distij;
            }
        }
    }
}

//Empirical - Computer no. of inter-district lab pairs within 40km
int comp_inter_district(){
    int num_edges = 0;
    for(int i = 1; i <= num_labs; i++){
        for(int j = i+1; j <= num_labs; j++){
            if(labz[i].district==labz[j].district) continue;
            double distij = calc_dist(labz[i].lat, labz[i].lon, labz[j].lat, labz[j].lon);
            if(distij < 40){
                cout << i << " " << j << " - " << labz[i].district << " " << labz[j].district << " - " << distij << endl;
                num_edges++;
            }
        }
    }
    return num_edges;
}

//Compute diameters of all provided cliques (in components)
vector<double> comp_maxclique_diameters(vector<vector<int>> &components, vector<int> &comp_of){
    int num_components = components.size();
    vector<double> max_pair_dist(num_components+1, 0);
    for(int i = 1; i <= num_labs; i++){
        for(int j = i+1; j <= num_labs; j++){
            int centroid_i = comp_of[i], centroid_j = comp_of[j];
            if(centroid_i != centroid_j) continue;
            double distij = Lmat[i][j];
            max_pair_dist[centroid_i] = max(max_pair_dist[centroid_i], distij);
        }
    }
    return max_pair_dist;
}

//Compute tight lower bound on the dataset using components as cliques (with possibly relaxed distance)
int get_tight_bound(vector<vector<int>> &components, vector<int> &comp_of){
    vector<double> max_pair_dist = comp_maxclique_diameters(components, comp_of);
    int centroid_shift_cost = 0;
    for(auto j: max_pair_dist){
        //cout << j << " ";
        centroid_shift_cost += j*1000;
    }
    //cout << endl << centroid_shift_cost << endl;
    return centroid_shift_cost;
}

//Select clusters s.t. there are maximum K per lab
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

//Print selected clusters
void print_clusters(vector<vector<int>> &selected, ofstream &fout, string file_name){
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
    fout.close();
}

signed main(int argc, char *argv[]){
    if(argc!=4){
        cerr << "Invalid number of arguments, 2 required!" << endl;
        cerr << "Usage: <exeuctable name> <district input CSV path> <lab input CSV path> <cluster output CSV path>" << endl;
        return 0;
    }
    ifstream distin, labin;
    distin.open(argv[1]);
    labin.open(argv[2]);
    string line;

    ofstream fout;
    getline(distin, line);
    while(getline(distin, line)){
        get_district(line, districts[++num_districts]);
        districts[num_districts].left = districts[num_districts].samples;
    }
    //cerr << num_districts << endl;

    getline(labin, line);
    while(getline(labin, line)){
        get_lab(line, labz[++num_labs]);
    }

    comp_lab_mat();

    //cout << comp_inter_district() << endl;

    vector<vector<int>> components;
    vector<int> comp_of(num_labs+1);
    get_components(components, comp_of);
    print_clusters(components, fout, "Centroids/Centroidsboundv0.txt"); //Print the centroids for calculating lower bound (MIP)
    //comp_max_blr_urban();
    cout << "Tight bound: " << get_tight_bound(components, comp_of) << endl;

    vector<vector<int>> cliques;
    vector<int> curr_nodes;
    vector<int> total, bangalore;
    for(int i = 1; i <= num_labs; i++){
        total.push_back(i);
        if(labz[i].district == 5) bangalore.push_back(i);
    }

    get_cliques(cliques, 6, curr_nodes, 0, bangalore); //get for overall size 5
    get_cliques(cliques, 6, curr_nodes, -1, total); //get for bangalore size 8
    int num_cliques = cliques.size();
    //print_freq(cliques, 7);
    cout << "Number of cliques computed: " << num_cliques << endl;
    
    vector<vector<int>> selected;
    sort(cliques.begin(), cliques.end(), sortbycap());
    select_uptoK_per_lab(selected, cliques);
    /* Uncomment to select all for(int i = 0; i < num_cliques; i++){
        selected.push_back(cliques[i]);
    }*/
    cout << "Number of cliques pre-selected: " << selected.size() << endl;
    print_clusters(selected, fout, argv[3]);
}