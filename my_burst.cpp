#include <iostream>
#include <algorithm>
#include <fstream>
#include <vector>
#include <fstream>
#include <string>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/trim.hpp>

using namespace std;

struct Node {
    int const state;
    Node *parent;
    Node(int s, Node *p): state(s), parent(p) {}
};

double logf(vector<double> const& alpha, vector<double> const& ln_alpha, int i, double x){
    return ln_alpha[i] - alpha[i] * x;
}

double tou(double gammalogn, int i, int j){
    return i >= j ? 0.0 :(j - i) * gammalogn;
}

vector<int> kleinberg_algorithm(vector<double> timeseries, double const s=2, double const gamma=1.0){
    // The number of events.
    size_t N = timeseries.size();

    // Calculate time intervals between successive events.
    vector<double> intervals(N-1);
    for(size_t i=0; i<intervals.size(); i++){
        intervals[i] = timeseries[i+1] - timeseries[i];
    }

    // The minimum interval.
    double const delta = *min_element(intervals.begin(), intervals.end());

    // The time length of the whole timeseries.
    double const T = timeseries.back() - timeseries.front();

    // The upper limit of burst levels.
    int const K = int(ceil(1 + log(T/delta) / log(s)));

    // Set alpha and ln_alpha
    vector<double> alpha(K);
    vector<double> ln_alpha(K);
    alpha[0] = N / T;
    ln_alpha[0] = log(alpha[0]);
    for(int i=1; i<K; i++){
        alpha[i] = s * alpha[i-1];
        ln_alpha[i] = log(alpha[i]);
    }

    double const gammalogn = gamma * log(double(N));

    vector<Node*> q(K);
    for(auto &it : q){
        it = NULL;
    }
    vector<double> C(K, numeric_limits<double>::infinity());
    C[0] = 0;

    // Start optimization.
    for(auto it : intervals){
        double interval = it;
        vector<Node*> q_new(K);
        vector<double> C_new(K);
        for(int i=0; i<K; i++){
            vector<double> c(K);
            for(int j=0; j<K; j++){
                c[j] = C[j] + tou(gammalogn, j, i);
            }
            size_t const j_min = min_element(c.begin(), c.end()) - c.begin();
            C_new[i] = -logf(alpha, ln_alpha, i, interval) + c[j_min];
            q_new[i] = new Node(i, q[j_min]);
        }

        q_new.swap(q);
        C_new.swap(C);
    }

    size_t const seq_min = min_element(C.begin(), C.end()) - C.begin();
    vector<int> bursts(N);
    size_t count = 0;
    for(Node* p=q[seq_min]; p!=NULL; p=p->parent){
        bursts[N - ++count] = p->state;
    }

    return bursts;

}


int main(int argc, char** argv){
    if(argc != 4){
        cerr << "Usage: a.out s gamma time-stamp-file" << endl;
        exit(1);
    }

    double const s = boost::lexical_cast<double>(argv[1]);
    double const gamma = boost::lexical_cast<double>(argv[2]);
    ifstream ifs(argv[3]);

    vector<double> timeseries;

    for(string line; getline(ifs, line);){
        boost::trim(line);
        timeseries.push_back(boost::lexical_cast<double>(line));
    }

    vector<int> bursts = kleinberg_algorithm(timeseries, s, gamma);

    for(auto it : bursts){
        cout << it << endl;
    }

}
