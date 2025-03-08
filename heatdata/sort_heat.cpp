#include "./heat.h"
#include<algorithm>
using std::sort;
namespace heat{
namespace mapsort{
bool cmp(const mypa&p1,const mypa&p2){
	return p1.second > p2.second;
}


int main(const mymap&data_heat,int k){
	vector<mypa> data(data_heat.begin(),data_heat.end());
	// sort before
	cout<<"sort before:"<<endl;
	print(data);
	// sort after
	cout<<endl<<"sort after"<<endl;
	sort(data.begin(),data.end(),cmp);
	print(data);

	// K
	cout<<"popularity data:";
	for(int i=0;i<k;++i){
		cout<<"data="<<data[i].first<<",freq="<<data[i].second<<endl;
	}
	return 0;
}

}// namespace mapsort
}// namespace heat


