#ifndef HEAT_H_
#define HEAT_H_
#include<iostream> // std::cin std::cout
#include<vector> // std::vector
#include<unordered_map> // std::unordered_map
#include<queue> // std::priority_queue
using namespace std;

namespace heat{
typedef pair<int,int> mypa;
typedef unordered_map<int,int> mymap;

namespace mapsort{
bool cmp(const mypa&,const mypa&);
int main(const mymap&data_heat,int k);
}// namespace mapsort

namespace priority{
struct mycmp;
typedef priority_queue<mypa,vector<mypa>,mycmp> mypq;
void getHeatData(const mymap&,int);
void copyHeat(vector<mypa>&,mypq&);
int main(const mymap&data_heat,int k);
}// namespace priority

template<typename T>
void getHeat(const vector<T>&freqs,unordered_map<T,int>&data_heat){
	for(const auto&freq:freqs){
		if(data_heat.count(freq)) ++data_heat[freq];
		else data_heat.emplace(freq,1);
	}
}

template<typename T>
void print(const T&data_heat){
	for(const auto&heat:data_heat){
		cout<<"data="<<heat.first<<",freq="<<heat.second<<endl;
	}
}

}//namespace heat
using namespace heat;
#endif
