#include "./heat.h"

namespace heat{

namespace priority{

struct mycmp{
	// >= : small root 
	// <= : big root
	bool operator()(const mypa&p1, const mypa&p2){
		return p1.second >= p2.second;
	}
};

void getHeatData(const mymap&data_heat,int k,mypq&heatq){
	for(const auto&data:data_heat){
		heatq.push(data);
		if(heatq.size()>k) heatq.pop();
	}

}

void copyHeat(vector<mypa>&heat,mypq&heatdata){
	while(!heatdata.empty()){
		heat.push_back(heatdata.top());
		heatdata.pop();
	}
}
int main(const mymap&data_heat,int k){
	mypq heatq;
	getHeatData(data_heat,k,heatq);
	cout<<"pop data:"<<endl;
	vector<mypa> heat;
	copyHeat(heat,heatq);
	print(heat);
	return 0;
}

}// namesapce priority
}// namespace heat
