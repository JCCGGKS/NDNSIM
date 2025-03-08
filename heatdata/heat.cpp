#include "./heat.h"
#include<limits>
int main(){
	vector<int> data;
	cout<<"please enter some data(end with non-num):"<<endl;
	int n;
	while(cin>>n) {
		data.push_back(n);
	}
	cin.clear();// clear error
	cin.ignore(numeric_limits<streamsize>::max(),'\n');
	cout<<"how many heat_data do you want:"<<endl;
	int k;
	while(cin>>k){
		if(k) break;
		cout<<"input is invalid,please reinput(k>0):"<<endl;
	}
	cout<<"what method do you want(MAP_SORT_:0; or PRIORITY:1):"<<endl;
	int flag;
	while(cin>>flag){
		if(flag==0 || flag==1) break;
		cout<<"input is invalid,please reinput(0 or 1):"<<endl;
	}
	// sum freq
	mymap data_heat;
	getHeat(data,data_heat);
	if(flag){
		// PRIORITY
		heat::priority::main(data_heat,k);
	}else{
		// MAP_SORT_
		heat::mapsort::main(data_heat,k);
	}
	return 0;
}
