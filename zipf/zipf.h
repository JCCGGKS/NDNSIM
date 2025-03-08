#ifndef ZIPF_H_
#define ZIPF_H_


#include<cstdint>
#include<cmath>
#include<random>
#include<vector>
namespace zipf{
    double getRandomValue1(){
        // false random
        return (double)rand()/RAND_MAX;
    }
    double getRandomValue(){
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0,std::numeric_limits<int32_t>::max());
        int32_t r = dis(gen);
        return (double)r/std::numeric_limits<int>::max();
    }
    void maVector(std::vector<uint32_t>&cons,std::map<int,int>&ma){
        for(auto num:cons){
            if(ma.count(num)) ++ma[num];
            else ma.insert(std::make_pair(num,1));
        }
    }
    void print(std::map<int,int>&ma){
        for(const auto &pa:ma ){
            std::cout<<pa.first<<" "<<pa.second<<std::endl;
        }
        std::cout<<"content size = "<<ma.size()<<std::endl;
    }
    class Zipf{
    public:
        Zipf():NumberofContents(10),m_q(0.7),m_s(0.7),P_Scum(NumberofContents+1,0.0){
            init();

        }
        Zipf(uint32_t n,double q,double s):NumberofContents(n),m_q(q),m_s(s),P_Scum(NumberofContents+1,0.0){
            init();
        }
        uint32_t getNext(){
            double r = getRandomValue();
            for(uint32_t i=1;i<=NumberofContents;i++){
                if(r<P_Scum[i]){
                    return i;
                }
            }
            return 0;
        }
        void getContent(uint32_t N,std::vector<uint32_t>&cons){
            // get N content
            for (uint32_t i = 0; i < N; i++){
                cons.push_back(getNext());
            }
        }
        ~Zipf(){}
    private:
        void init(){
            for(uint32_t i=1;i<=NumberofContents;i++){
                P_Scum[i]=P_Scum[i-1] + 1.0/(std::pow(i+m_q,m_s));
            }

            for(uint32_t i=1;i<=NumberofContents;i++){
                P_Scum[i] /= P_Scum[NumberofContents];
            }
        }
    private:
         uint32_t NumberofContents;
         double m_q;
         double m_s;
         // P_Scum
         std::vector<double> P_Scum;
         // 1.0/(n+m_q)^m_s
    };
}
#endif