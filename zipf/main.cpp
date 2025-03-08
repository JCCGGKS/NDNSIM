#include <iostream>
#include <map> // auto sort
#include "./zipf.h"
// const int numberofRequests= 100;
// const int numberofContents = 10;
// const double q=0.7;
// const double s=0.7;
int main(){
    std::cout<<"please input these information:(-1 point default)"<<std::endl;
    uint32_t numberofContents;
    std::cout<<"number of contents:";std::cin>>numberofContents;
    if(numberofContents==-1) numberofContents=10;
    double q;
    std::cout<<"q:";std::cin>>q;
    if(q==-1) q=0.7;
    double s;
    std::cout<<"s:";std::cin>>s;
    if(s==-1) s=0.7;
    uint32_t numberofRequests;
    std::cout<<"number of requests:";std::cin>>numberofRequests;
    if(numberofRequests==-1) numberofRequests=100;
    std::cout<<"number of contents:"<<numberofContents<<", ";
    std::cout<<"q:"<<q<<", ";
    std::cout<<"s:"<<s<<", ";
    std::cout<<"number of requests:"<<numberofRequests<<std::endl;
    zipf::Zipf zipf(numberofContents,q,s);
    std::vector<uint32_t> seqs;
    zipf.getContent(numberofRequests,seqs);
    std::cout<<"seq of contents:"<<std::endl;
    for(uint32_t i=0;i<seqs.size();i++){
        std::cout<<seqs[i]<<" ";
        if((i+1)%10==0) std::cout<<std::endl;
    }
    std::map<int,int> ma;
    zipf::maVector(seqs,ma);
    std::cout<<"content frequency:"<<std::endl;
    zipf::print(ma);

    return 0;
}