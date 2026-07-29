#include <iostream>
#include <vector>
#include <string>
#include <algorithm> // For std::min

using namespace std;

// Structure for membership function parameters
struct Membershipfn {
    float higherlimit;
    float lowerlimit;
    float peakval;
    float leastval;
    float peakmfval;
};

struct Membershipval {
    float valarr[2];
    string membdescp[2];
    int mfindex[2];
};

class Fuzzy {
private:
    int numdescp;
    vector<string> descp;               // Descriptions
    vector<Membershipfn> membershipfn;     // Membership functions
    int maxcrispip;
    int mincrispip;
    int crispop;
    vector<float> supportpts;              // Support points
    float step;
    Membershipval membwrap;

public:
    Fuzzy(int numdescp, const vector<string> &descp, int maxcrispip, int mincrispip)
        : numdescp(numdescp),
          descp(descp),
          maxcrispip(maxcrispip),
          mincrispip(mincrispip),
          supportpts(numdescp) 
    {
        membershipfn.resize(numdescp); 
        createTriangularmf();
    }

    // Create triangular membership functions
    void createTriangularmf() {
        supportpts[0] = static_cast<float>(mincrispip);
        supportpts[numdescp - 1] = static_cast<float>(maxcrispip);

        step = static_cast<float>(maxcrispip - mincrispip) / (numdescp - 1);

        for (int i = 1; i < numdescp - 1; i++) {
            supportpts[i] = supportpts[0] + step * i;
        }
        for (int i = 0; i < numdescp; i++){
            membershipfn[i].peakval = 1;
            membershipfn[i].peakmfval = supportpts[i]; 
        }
        for (int i = 1; i < numdescp; i++){
            membershipfn[i].lowerlimit = supportpts[i - 1];
        }
        membershipfn[0].lowerlimit = supportpts[0];
        for (int i = 0; i < numdescp - 1; i++){
            membershipfn[i].higherlimit = supportpts[i + 1];
        }
        membershipfn[numdescp - 1].higherlimit = supportpts[numdescp - 1];
    }

    void displaySupportPoints(){
        cout << "Support Points: ";
        for (float value : supportpts) {
            cout << value << " ";
        }
        cout << endl;
    }
    
    void findMFval(int valnum){
        int index = valnum / static_cast<int>(step);
        
        // Guard boundaries to prevent out-of-bounds indexing
        if (index >= numdescp - 1) index = numdescp - 2;

        membwrap.valarr[0] = ((valnum - membershipfn[index + 1].lowerlimit) / (membershipfn[index + 1].peakmfval - membershipfn[index + 1].lowerlimit));
        membwrap.valarr[1] = ((membershipfn[index].higherlimit - valnum) / (membershipfn[index].higherlimit - membershipfn[index].peakmfval));
        membwrap.membdescp[0] = descp[index + 1];
        membwrap.membdescp[1] = descp[index];
        membwrap.mfindex[0] = index + 1;
        membwrap.mfindex[1] = index;
    }
    
    void displayMFval(){
        cout << "MF values: " << endl;
        for(int i = 0; i < 2; i++){
            cout << membwrap.membdescp[i] << " -> " << membwrap.valarr[i] << endl;
        }
    }
    
    Membershipval getmembwrap(){
        return membwrap;
    }
};

struct Opmfval {
    float opmf;
    string opmfdescp;
    int max_mfindex;
}; 

class Defuzzification{
    private:
      int numopdescp;
      int peaktimeval;
      vector<float>supportpts;
      string opmfdescp[3]={"Low","Medium","High"};
      vector<Membershipfn>opmembfn;
      float opcrispval;
      float step;
      
    public:
      Defuzzification(int numopdescp, int peaktimeval){
          this->numopdescp=numopdescp;
          this->peaktimeval=peaktimeval;
          supportpts.resize(numopdescp);
      }
      
      void mfcreation(){
          step=static_cast<float> (peaktimeval) / static_cast<float>(numopdescp-1);
          supportpts[0]=0;
          supportpts[1]=step;
          supportpts[2]=static_cast<float>(peaktimeval);
          opmembfn.resize(numopdescp);
          for(int i=0;i<numopdescp;i++){
              opmembfn[i].peakmfval=supportpts[i];
              opmembfn[i].peakval=1;
              opmembfn[i].leastval=0;
          }
          for(int i=0;i<numopdescp-1;i++){
              opmembfn[i].higherlimit=supportpts[i+1];
          }
          opmembfn[2].higherlimit=supportpts[2];
          for(int i=1;i<numopdescp;i++){
              opmembfn[i].peakmfval=supportpts[i-1];
          }opmembfn[0].peakmfval=supportpts[0];
      }
      float defuzzifiedop(float opmfval, string opdescp){
          int matched_index=0;
          for(int i=0;i<numopdescp;i++){
              if(opmfdescp[i]==opdescp){
                  matched_index=i;
              }
          }
          if(matched_index==2){
              opcrispval=opmembfn[matched_index].lowerlimit+opmfval*(opmembfn[matched_index].higherlimit-opmembfn[matched_index].lowerlimit);
          }else if(matched_index==0){
              opcrispval=opmembfn[matched_index].lowerlimit+(1-opmfval)*(opmembfn[matched_index].higherlimit-opmembfn[matched_index].lowerlimit);
          }else{
              opcrispval=step;
          }
          return opcrispval;
      }
};

// Mamdani Inference System
class CarFIS {
private:
    int numopdescp;
    string traffic_time_map[4][4];
    int xindex[2];
    int yindex[2];
    string opdescp[4];
    float opvals[4];
    Opmfval op;
    Defuzzification DefuzedOp;
    float crispval;
      
public:
    CarFIS(int numopdescp,int peaktimeval) : DefuzedOp(numopdescp,peaktimeval) { 
        for(int i = 0; i < 4; i++){
            for(int j = 0; j < 4; j++){
                if(i == j){
                    traffic_time_map[i][j] = "Medium"; 
                } else if(i < j){
                    traffic_time_map[i][j] = "High";
                } else {
                    traffic_time_map[i][j] = "Low";
                }
            }
        }
        this->numopdescp=numopdescp;
    }

    void getopmfval(Membershipval &membershipval1, Membershipval &membershipval2){
        xindex[0] = membershipval1.mfindex[0];
        xindex[1] = membershipval1.mfindex[1];
        yindex[0] = membershipval2.mfindex[0];
        yindex[1] = membershipval2.mfindex[1];
        
        opdescp[0] = traffic_time_map[xindex[0]][yindex[0]];
        opdescp[1] = traffic_time_map[xindex[0]][yindex[1]];
        opdescp[2] = traffic_time_map[xindex[1]][yindex[0]];
        opdescp[3] = traffic_time_map[xindex[1]][yindex[1]];
         
        // Fuzzy logic Min-Composition rule application using actual membership values
        opvals[0] = std::min(membershipval1.valarr[0], membershipval2.valarr[0]);
        opvals[1] = std::min(membershipval1.valarr[0], membershipval2.valarr[1]);
        opvals[2] = std::min(membershipval1.valarr[1], membershipval2.valarr[0]);
        opvals[3] = std::min(membershipval1.valarr[1], membershipval2.valarr[1]);
          
        // Max-composition calculation
        op.opmf = opvals[0];
        op.max_mfindex = 0;
        for(int i = 1; i < 4; i++){
            if(op.opmf < opvals[i]){
                op.opmf = opvals[i];
                op.max_mfindex = i;
            }
        }
        op.opmfdescp = opdescp[op.max_mfindex];
    }
      
    Opmfval getOpfuzzyval() { 
        return op;
    }
    
    float getCrispval(){
        DefuzedOp.mfcreation();
        crispval=DefuzedOp.defuzzifiedop(op.opmf,op.opmfdescp);
        return crispval;
    }
};


int main() {
    vector<string> descriptions = {"Low", "Medium", "High", "V.High"};
    Fuzzy trafficlane1(4, descriptions, 100, 0); 
    
    trafficlane1.displaySupportPoints();
    trafficlane1.findMFval(37);
    trafficlane1.displayMFval();
    Membershipval lane1mf=trafficlane1.getmembwrap();
    
    Fuzzy trafficlane2(4, descriptions, 100, 0); 
    
    trafficlane2.displaySupportPoints();
    trafficlane2.findMFval(70);
    trafficlane2.displayMFval();
    Membershipval lane2mf=trafficlane2.getmembwrap();
    
    CarFIS Timeop(3,289200);
    Timeop.getopmfval(lane1mf,lane2mf);
    
    Opmfval Fuzzyop=Timeop.getOpfuzzyval();
    cout<<"Output mf val:"<<endl;
    cout<<Fuzzyop.opmfdescp<<"->"<<Fuzzyop.opmf<<endl;
    
    float outputval=Timeop.getCrispval();
    cout<<"Crisp O/p"<<endl;
    cout<<outputval<<" seconds"<<endl;
    
    return 0;
}
