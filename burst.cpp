#include <vector>
#include <fstream>
#include <iostream>
#include <string>
#include <algorithm>
#include <memory>
#include <cstdlib>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/trim.hpp>
using namespace std;

struct Node {
    int const state; // burst level: 0, 1, ..., K-1
    shared_ptr<Node> const parent;
    Node(int s, shared_ptr<Node> const& p) : state(s), parent(p) {}
};

vector<int> detect_bursts_by_kleinberg_algorithm(vector<double> const& timeseries,
                                                 double const s=2,     // s>1.0
                                                 double const gamma=1) // gamma>0.0
{
    assert(timeseries.size()>1);
    assert(s>1.0);
    assert(gamma>0.0);
    
    // The number of events.
    size_t const N = timeseries.size();

    // Calculate time intervals between successive events.
    vector<double> intervals(N-1);
    for (size_t i=0; i<intervals.size(); ++i) {
        // 'timeseries' should be sorted in ascending order.
        assert(timeseries[i]<=timeseries[i+1]);
        intervals[i] = timeseries[i+1]-timeseries[i];
    }

    // The minimum interval.
    double const delta = *min_element(intervals.begin(), intervals.end());

    // The time length of the whole timeseries.
    double const T = timeseries.back()-timeseries.front();
    assert(T>0.0);

    // The upper limit of burst levels.
    int const K = int(ceil(1+log(T/delta)/log(s)));

    vector<double> alpha(K); // Event rate at each burst level.
    vector<double> ln_alpha(K);

    alpha[0] = N/T; // Average event rate.
    ln_alpha[0] = log(alpha[0]);

    for (int i=1; i<K; ++i) {
        alpha[i] = s*alpha[i-1];
        ln_alpha[i] = log(alpha[i]);
    }

    double const gammalogn = gamma*log(double(N));

    // Cost function for reproducing a given interval.
    // 'i': the burst level
    // 'x': interval
    auto logf = [&](int i, double x) -> double {
        return ln_alpha[i]-alpha[i]*x;
    };

    // Cost function for changing a burst level.
    // 'i': the previous burst level
    // 'j': the next burst level
    auto tau = [&](int i, int j) -> double {
        if (i>=j) return 0.0;
        return (j-i)*gammalogn;
    };

    // Initialize.
    vector<shared_ptr<Node> > q(K); // state chains.
    for_each(q.begin(), q.end(), [](shared_ptr<Node>& p) {
            p.reset(new Node(0, shared_ptr<Node>()));
        });

    vector<double> C(K, numeric_limits<double>::infinity()); // costs.
    C[0] = 0;

    // Start optimization.
    for_each(intervals.begin(), intervals.end(), [&](double interval) {
            vector<shared_ptr<Node> > q_new(K);
            vector<double> C_new(K);
            for (int i=0; i<K; ++i) {
                vector<double> c(K);
                for (int j=0; j<K; ++j) {
                    c[j] = C[j]+tau(j, i);
                }
                size_t const j_min = min_element(c.begin(), c.end())-c.begin();
                // Store the cost for setting the burst level i.
                C_new[i] = -logf(i, interval)+c[j_min];
                q_new[i].reset(new Node(i, q[j_min]));
            }

            q_new.swap(q);
            C_new.swap(C);
        });

    vector<int> bursts(N);
    size_t const seq_min = min_element(C.begin(), C.end())-C.begin();
    size_t count = 0;
    for (shared_ptr<Node> p=q[seq_min]; p.get()!=0; p=p->parent) {
        assert(count<N);
        bursts[N-++count] = p->state;
    }

    // ##################################################################
    // To avoid stack overflow caused by recursive calling of a large
    // number of destructors when the linked list 'q' is deleted,
    // once expand 'q' into a temporary simple array.
    vector<shared_ptr<Node> > tmp;
    for (shared_ptr<Node> p=q.front(); p.get()!=0; p=p->parent) {
        tmp.push_back(p);
    }
    q.clear();
    // ##################################################################

    return bursts;
}

int main(int argc, char** argv)
{
    if (argc!=4) {
        cerr << "usage: a.exe s gamma time-stamp-file" << endl;
        exit(1);
    }

    double const s = boost::lexical_cast<double>(argv[1]);     // s>1
    double const gamma = boost::lexical_cast<double>(argv[2]); // gamma>0.0
    ifstream ifs(argv[3]);

    vector<double> timeseries;
    
    for (string line; getline(ifs, line);) {
        boost::trim(line);
        timeseries.push_back(boost::lexical_cast<double>(line));
    }
  
    vector<int> bursts = detect_bursts_by_kleinberg_algorithm(timeseries, s, gamma);

    assert(timeseries.size()==bursts.size());

    for_each(bursts.begin(), bursts.end(), [](int b) {
            cout << b << endl;
        });


    return 0;
}
