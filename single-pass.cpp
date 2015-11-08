#include <iostream>
#include <vector>
#include <map>
#include <fstream>

using namespace std;

struct Point{
    vector<double> vec;
    vector<double> tf;
    int centroid_id;
    int leng;
    Point(){}
    Point(vector<double> v, vector<double> t, int id, int len):vec(v), tf(t), centroid_id(id), leng(len){}
};

class SinglePass{

    private:
        vector<Point> input;
        vector<Point> centroid;

    public:
        double cal_dist(Point &a, Point &b);
        void read_data_file(string data_file);
        vector<string> string_split(string terms_str, string spliting_tag);
        vector<Point> cluster(double threshold);
        void display();
};

void SinglePass::read_data_file(string data_file){
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
                double feat_value = (double)atof(it.substr(feat_pos+1).c_str());
                if (feat_value != 0){
                    p.vec.push_back(feat_id);
                    p.tf.push_back(feat_value);
                }
            }
        }
        p.leng = 0;
        p.centroid_id = -1;
        input.push_back(p);
    }
    fin.close();
}

vector<string> SinglePass::string_split(string terms_str, string spliting_tag) {
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

double SinglePass::cal_dist(Point &a, Point &b){
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

vector<Point> SinglePass::cluster(double threshold){

    input[0].centroid_id = 0;
    input[0].leng = 1;
    centroid.push_back(input[0]);

    for(size_t i=1; i<input.size(); i++){
        double minc = numeric_limits<double>::max();
        int index = -1;
        for(size_t j=0; j<centroid.size(); j++){
            double dist = cal_dist(centroid[j], input[i]);
            if (minc > dist){
                minc = dist;
                index = j;
            }
        }
        // cout << minc << endl;
        if(minc > threshold){
            input[i].centroid_id = centroid.size();
            input[i].leng = 1;
            centroid.push_back(input[i]);
        }else{
            input[i].centroid_id = index;
            int leng = centroid[index].leng;
            for(size_t j=0; j<centroid[index].vec.size(); j++){
                bool flag = true;
                for(size_t k=0; k<input[i].vec.size(); k++){
                    if(centroid[index].vec[j] == input[i].vec[k]){
                        flag = false;
                        centroid[index].tf[j] = (centroid[index].tf[j] * leng + input[i].tf[k]) / (leng + 1.0);
                        break;
                    }
                }
                if(flag) centroid[index].tf[j] = centroid[index].tf[j] * leng / (leng + 1.0);
            }
            int len = centroid[index].vec.size();
            for(size_t j=0; j<input[i].vec.size(); j++){
                bool flag = true;
                for(size_t k=0; k<len; k++){
                    if(input[i].vec[j] == centroid[index].vec[k]){
                        flag = false;
                        break;
                    }
                }
                if(flag){
                    centroid[index].vec.push_back(input[i].vec[j]);
                    centroid[index].tf.push_back(input[i].tf[j] / (leng + 1.0));
                }
            }
            centroid[index].leng++;
        }
    }

    return centroid;
}

void SinglePass::display(){
    for (auto it : input) {
        for(size_t i=0; i<it.vec.size(); i++){
            cout << it.vec[i] << ":" << it.tf[i] << " | ";
        }
        cout << endl;
    }

    for(auto it : input) {
        cout << it.centroid_id << " ";
    }
    cout <<  endl;

    map<int, vector<int> > clu;
    for(size_t i=0; i<input.size(); i++) {
        clu[input[i].centroid_id].push_back(i);
    }
    for(auto it : clu){
        cout << it.first << " : ";
        for(auto tt : it.second){
            cout << tt << " ";
        }
        cout <<  endl;
    }
}

int main(){
    SinglePass singlepass;
    singlepass.read_data_file("single.txt");
    singlepass.cluster(100);
    singlepass.display();
}


