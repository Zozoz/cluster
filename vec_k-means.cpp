#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <limits>

using namespace std;

struct Point{
    vector<double> vec;
    int length;
    int centroid_id;
    Point(vector<double> v, int len, int id):vec(v), length(len), centroid_id(id) {}
};

class KMeans{

    public:
        KMeans();
        KMeans(int k);
        ~KMeans();
        void cluster();
        double cal_distance(vector<double> vec1, vector<double> vec2);
        void load_training_file(string input_file);
        void display();

    protected:
        int num_c; // the number of cluster
        vector<Point> centroid;
        vector<Point> input;

};

KMeans::KMeans(){
}

KMeans::~KMeans(){
}

KMeans::KMeans(int k){
    num_c = k;
}

void KMeans::load_training_file(string input_file) {
    double data[][2] = {{0, 0}, {1, 0}, {0, 1}, {2, 2}, {4, 4}, {5, 5}, {6, 6}};
    for(int i=0; i<7; i++){
        vector<double> tmp;
        for(int j=0; j<2; j++){
            tmp.push_back(data[i][j]);
        }
        input.push_back(Point(tmp, -1, 0));
    }
    for (int i = 0; i < num_c; i++) {
        input[i].length = 0;
        input[i].centroid_id = i;
        centroid.push_back(input[i]);
    }
}

double KMeans::cal_distance(vector<double> vec1, vector<double> vec2) {

    double sum = 0;
    for (size_t i = 0; i < vec1.size(); i++) {
        sum += (vec1[i] - vec2[i]) * (vec1[i] - vec2[i]);
    }
    return sum;
}

void KMeans::cluster(){
    while(true){
        bool flag = false;
        vector<Point> new_c;
        for (auto it : centroid) {
            it.length = 0;
            new_c.push_back(it);
        }
        for (size_t i = 0; i < input.size(); i++) {
            double minc = numeric_limits<double>::max();
            size_t index = -1;
            for (size_t j = 0; j < centroid.size(); j++) {
                double dist = cal_distance(input[i].vec, centroid[j].vec);
                if (minc > dist) {
                    minc = dist;
                    index = j;
                }
            }
            if (input[i].centroid_id != index){
                flag = true;
            }
            input[i].centroid_id = index;
            int leng = new_c[index].length;
            for (size_t k = 0; k < new_c[index].vec.size(); k++) {
                new_c[index].vec[k] = (new_c[index].vec[k] * leng + input[i].vec[k]) / (leng + 1.0);
            }
            new_c[index].length++;
        }
        new_c.swap(centroid);
        if (!flag) break;
        for (size_t i = 0; i < centroid.size(); i++) {
            centroid[i].length = 0;
        }
    }
}

void KMeans::display() {
    for (auto it : input) {
        cout << it.centroid_id << " ";
    }
    cout << endl;
    for (auto it : centroid) {
        for (auto v : it.vec) {
            cout << v << " ";
        }
        cout << endl;
    }
}


int main() {
    KMeans kmeans(2);
    kmeans.load_training_file("pp");
    kmeans.cluster();
    kmeans.display();
}





