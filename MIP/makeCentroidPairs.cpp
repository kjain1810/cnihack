#include <bits/stdc++.h>
#include "cpputils.hpp" 
using namespace std ;

#define reMin(a, b) a = min(a, b)
#define reMax(a, b) a = max(a, b)

#define lint long long
#define pb push_back
#define F first 
#define S second 
#define sz(x) (int)x.size()
#define all(x) begin(x), end(x)
#define SET(x, val) memset(x, val, sizeof(x))
#define fastio ios_base::sync_with_stdio(0); cin.tie(0); cout.tie(0)

typedef vector < int > vi ;
typedef pair < int, int > pii ;
typedef pair < double, double > point ;

const int C = 1e5 ; 
int n, m, c ;
vi centroids[C] ; 
int freq[C] ;
set < int > pairs[N] ;
int connections = 0 ;

void outputConnections() {
	freopen("connections.txt", "w", stdout);
	cout << num_districts << endl ; 
	for(int i = 1; i <= num_districts; i++) {
		cout << sz(pairs[i]) << endl ; 
		int j = 0;
		for(int x : pairs[i]) {
			cout << x ;
			if(j++ < (sz(pairs[i]) - 1)) cout << " " ; 
		} 
		if(i < num_districts) cout << endl ; 
	}	
}

int main()
{
	getinput(); srand(time(0));
	n = num_labs, m = num_districts ; 

	freopen("Centroidsv0.txt", "r", stdin);
	cin >> c ; int labs ;
	set < int > covered ;

	for(int i = 1; i <= c ; i++) {
		cin >> labs ; centroids[i].resize(labs);
		for(int j = 0; j < labs; j++) {
			cin >> centroids[i][j] ;
			covered.insert(centroids[i][j]);
		}
	}
	// cout << sz(covered) << endl ;
	for(int i = 1; i <= num_labs; i++)
		if(covered.find(i) == covered.end())
			cout << i << endl ;

	// cout << calcdist(labz[62].lat, labz[62].lon, labz[63].lat, labz[63].lon) << endl ; 
	// return 0;

	SET(freq, 0);
	// for(int k = 1; k <= num_labs; k++)
	// 	cout << freq[k] << " " ; 
	// cout << endl ; 
	vi ordering ;
	for(int i = 1; i <=c ;i++) ordering.pb(i);

	for(int i = 1; i <= num_districts; i++) {
		set < int > distinct ; int iter = 0 ;

		while(sz(distinct) != num_labs) {
			int best_centroid = 0; double best_score = 1e9 ;
			random_shuffle(all(ordering));

			// for(int j = 1; j <= c; j++) {
			for(int j : ordering) {
				double score = 0 ;
				for(int lab : centroids[j]) score += freq[lab] ;
				score /= sz(centroids[j]);

				if(score <= best_score) {
					if(score == best_score and rand() % 2 == 1) continue ; 
					best_score = score ;
					best_centroid = j ;
				}
			}
			int bef = sz(distinct);
			for(int lab : centroids[best_centroid]) freq[lab]++ ;

			for(int lab : centroids[best_centroid]) distinct.insert(lab);
			int aft = sz(distinct);

			pairs[i].insert(best_centroid);

			if(aft == bef) {
				// cout << i << " : Distinct labs = " << sz(distinct) << " Distinct centroids = " << sz(pairs[i]) << endl ; 
				// for(int k = 1; k <= num_labs; k++)
				// 	cout << freq[k] << " " ;
				// cout << endl ; 
			}

			// cout << sz(distinct) << endl ; 

			if(++iter > 1000) {
				cout << best_centroid << " : " << best_score << endl ;
				cout << "EXITING!" << endl ;
				return 0;
			}
		}
		cout << sz(pairs[i]) << endl ; 
		connections += sz(pairs[i]);
	}
	SET(freq, 0);
	for(int i = 1; i <= num_districts ; i++)
		for(int j : pairs[i])
			freq[j]++ ; 

	int non_zero = 0 ;
	for(int k = 1; k <= c; k++) {
		cout << freq[k] << " " ; 
		if(freq[k] == 0) non_zero++ ;
	}
	cout << endl ;
	cout << non_zero << endl ;

	outputConnections();
	return 0;
}