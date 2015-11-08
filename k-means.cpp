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
    vector<double> tf;
    int length;
    int centroid_id;
    Point(){}
    Point(vector<double> v, vector<double> t, int len, int id):vec(v), tf(t), length(len), centroid_id(id) {}
};

class KMeans{

    public:
        KMeans();
        KMeans(int k);
        ~KMeans();
        void cluster();
        double cal_distance(vector<double> vec1, vector<double> vec2);
        double cal_dist(Point &a, Point &b);
        void load_training_file(string input_file);
        void read_data_file(string input_file);
        vector<string> string_split(string terms_str, string spliting_tag);
        double cal_cost();
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
    for (int i = 0; i < 7; i++) {
        vector<double> tmp_vec;
        vector<double> tmp_tf;
        tmp_vec.push_back(0);
        tmp_vec.push_back(1);
        for (int j = 0; j < 2; j++) {
            tmp_tf.push_back(data[i][j]);
        }
        Point p(tmp_vec, tmp_tf, 0, -1);
        input.push_back(p);
    }
}

double KMeans::cal_distance(vector<double> vec1, vector<double> vec2) {

    double sum = 0;
    for (size_t i = 0; i < vec1.size(); i++) {
        sum += (vec1[i] - vec2[i]) * (vec1[i] - vec2[i]);
    }
    return sum;
}

void KMeans::read_data_file(string data_file){
    ifstream fin(data_file.c_str());
    if (!fin){
        cerr << "Error opening file: "<< data_file << endl;
        exit(0);
    }
    string line_str;
    while (getline(fin, line_str)){
        Point p;
        if (line_str != ""){
            vector<string> fv_vec = string_split(line_str, " ");
            for (auto it : fv_vec) {
                size_t feat_pos = it.find_first_of(":");
                int feat_id = atoi(it.substr(0, feat_pos).c_str());
                int feat_value = (int)atof(it.substr(feat_pos+1).c_str());
                if (feat_value != 0){
                    p.vec.push_back(feat_id);
                    p.tf.push_back(feat_value);
                }
            }
        }
        p.length = 0;
        p.centroid_id = -1;
        input.push_back(p);
    }
    fin.close();
}

vector<string> KMeans::string_split(string terms_str, string spliting_tag) {
    vector<string> feat_vec;
    size_t term_beg_pos = 0;
    size_t term_end_pos = 0;
    while ((term_end_pos=terms_str.find_first_of(spliting_tag, term_beg_pos)) != string::npos) {
        if (term_end_pos > term_beg_pos) {
            string term_str = terms_str.substr(term_beg_pos, term_end_pos - term_beg_pos);
            feat_vec.push_back(term_str);
        }
        term_beg_pos = term_end_pos + 1;
    }
    if (term_beg_pos < terms_str.size()) {
        string end_str = terms_str.substr(term_beg_pos);
        feat_vec.push_back(end_str);
    }
    return feat_vec;
}

double KMeans::cal_dist(Point &a, Point &b){
    double sum = 0;
    for(size_t i=0; i<a.vec.size(); i++){
        bool flag = true;
        for(size_t j=0; j<b.vec.size(); j++){
            if(a.vec[i] == b.vec[j]){
                sum += (a.tf[i] - b.tf[j]) * (a.tf[i] - b.tf[j]);
                flag = false;
                break;
            }
        }
        if(flag) sum += a.tf[i] * a.tf[i];
    }
    for(size_t i=0; i<b.vec.size(); i++){
        bool flag = true;
        for(size_t j=0; j<a.vec.size(); j++){
            if(b.vec[i] == a.vec[j]){
                flag = false;
                break;
            }
        }
        if(flag) sum += b.tf[i] * b.tf[i];
    }
    return sum;
}

void KMeans::cluster(){
    for (int i = 0; i < num_c; i++) {
        input[i].length = 0;
        input[i].centroid_id = i;
        centroid.push_back(input[i]);
    }
    while(true){
        bool flag_b = false;
        vector<Point> new_c;
        for (auto it : centroid) {
            it.length = 0;
            new_c.push_back(it);
        }
        for (size_t i = 0; i < input.size(); i++) {
            double minc = numeric_limits<double>::max();
            size_t index = -1;
            for (size_t j = 0; j < centroid.size(); j++) {
                //double dist = cal_distance(input[i].vec, centroid[j].vec);
                double dist = cal_dist(input[i], centroid[j]);
                if (minc > dist) {
                    minc = dist;
                    index = j;
                }
            }
            // cout << "minc=" << minc << "  ,index=" << index << endl;
            if (input[i].centroid_id != index){
                flag_b = true;
            }
            input[i].centroid_id = index;
            int leng = new_c[index].length;
            //for (size_t k = 0; k < new_c[index].vec.size(); k++) {
            //    new_c[index].vec[k] = (new_c[index].vec[k] * leng + input[i].vec[k]) / (leng + 1.0);
            //}
            for (size_t j = 0; j < new_c[index].vec.size(); j++) {
                bool flag = true;
                for (size_t k = 0; k < input[i].vec.size(); k++) {
                    if (new_c[index].vec[j] == input[i].vec[k]) {
                        flag = false;
                        new_c[index].tf[j] = (new_c[index].tf[j] * leng + input[i].tf[k]) / (leng + 1.0);
                        break;
                    }
                }
                if(flag) new_c[index].tf[j] = new_c[index].tf[j] * leng / (leng + 1.0);
            }
            int len = new_c[index].vec.size();
            for (size_t j = 0; j < input[i].vec.size(); j++) {
                bool flag = true;
                for (size_t k = 0; k < len; k++) {
                    if (input[i].vec[j] == new_c[index].vec[k]) {
                        flag = false;
                        break;
                    }
                }
                if (flag) {
                    new_c[index].vec.push_back(input[i].vec[j]);
                    new_c[index].tf.push_back(input[i].tf[j] / (leng + 1.0));
                }
            }
            new_c[index].length++;
        }
        new_c.swap(centroid);
        if (!flag_b) break;
        for (size_t i = 0; i < centroid.size(); i++) {
            centroid[i].length = 0;
        }
    }
}

double KMeans::cal_cost() {
    double sum = 0;
    for (auto it : input) {
        int ind = it.centroid_id;
        sum += cal_dist(it, centroid[ind]);
    }
    return sum;
}


void KMeans::display() {
    for (auto it : centroid) {
        for (size_t i = 0; i < it.vec.size(); i++) {
            cout << it.vec[i] << ":" << it.tf[i] << " | ";
        }
        cout << endl;
    }

    for (auto it : input) {
        cout << it.centroid_id << " ";
    }
    cout << endl;
}


int main() {
    for (int i = 2; i < 50; i++) {
        KMeans kmeans(i);
        kmeans.read_data_file("single.txt");
        //kmeans.load_training_file("single.txt");
        kmeans.cluster();
        //kmeans.display();
        cout << kmeans.cal_cost() <<endl;
    }

}


