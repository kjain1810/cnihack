#include <bits/stdc++.h>
#include "cpputils.hpp" 
using namespace std ;

// MACROS 
#define pb push_back
#define sz(x) (int)x.size()
#define all(x) begin(x), end(x)
#define SET(x, val) memset(x, val, sizeof(x))

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
	// Outputs all the distrcit-centroid connections formed
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

int main(int argc, char* argv[])
{
	getinput(); srand(time(0));
	num_labs = 86 ; num_districts = 30 ; 
	n = num_labs, m = num_districts ; 

	// File to read the list of centroids (L_c) 
	// change filename if required 
	if(argc > 1)
		freopen(argv[1], "r", stdin);
	else 
		freopen("Centroidsv0.txt", "r", stdin);
	
	cin >> c ; int labs ;

	// covered keeps track of how many distinct labs are covered by given cliques
	set < int > covered ;

	for(int i = 1; i <= c ; i++) {
		cin >> labs ; centroids[i].resize(labs);
		for(int j = 0; j < labs; j++) {
			cin >> centroids[i][j] ;
			covered.insert(centroids[i][j]);
		}
	}
	for(int i = 1; i <= num_labs; i++)
		if(covered.find(i) == covered.end())
			cout << "Lab " << i << " not covered!" << endl ;

	SET(freq, 0);
	vi ordering ;
	for(int i = 1; i <=c ;i++) ordering.pb(i);

	for(int i = 1; i <= num_districts; i++) {
		for(int times = 0; times < 3; times++) {
			set < int > distinct ; int iter = 0 ;

			// keep selecting while all labs are not covered by the current 
			// centroids to be conected with the district 
			while(sz(distinct) != num_labs) {

				/* Radnomly shuffle the centroids and choose the one whose
				labs are least chosen overall thus maximizing uniformity */

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
				// Update frequency of labs (as they got chosen) and also
				// update total number of distinct labs covered by this district
				for(int lab : centroids[best_centroid]) freq[lab]++ ;
				for(int lab : centroids[best_centroid]) distinct.insert(lab);

				pairs[i].insert(best_centroid);
			}
		}
		cout << sz(pairs[i]) << endl ; 
		connections += sz(pairs[i]);
	}
	// Reset frequency of labs to track frequency of centroids being chosen next
	SET(freq, 0);
	for(int i = 1; i <= num_districts ; i++)
		for(int j : pairs[i])
			freq[j]++ ; 

	// Count the number of times each centroid was chosen by some district
	int non_zero = 0 ;
	for(int k = 1; k <= c; k++) {
		cout << freq[k] << " " ; 
		if(freq[k] == 0) non_zero++ ;
	}
	cout << endl ;
	// Output how many centroids left unchosen (not selected by any district)
	cout << non_zero << endl ;

	outputConnections();
	return 0;
}