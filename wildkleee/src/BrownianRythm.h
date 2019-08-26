//
//  BrownianRythm.h
//  wildkleee
//
//  Created by Jonas Fehr on 13/08/2019.
//

#ifndef BrownianRythm_h
#define BrownianRythm_h

#include "SelectableObjectBase.hpp"

class Drunk{
public:
    int value;
    int step;
    int max;

    Drunk(){}
    Drunk(int max, int step){
        setup(max, step);
    }
    
    void setup(int max, int step){
        this->max = max;
        this->step = step;
        this->value = max/2;
    }
    
    void setStep(int step){
        
    }
    
    int get(){
        value += ofRandom(-step, step);
        if(value < 0) value = 0;
        if(value > max) value = max;
        return value;
    }
};

class Brownian{
public:
    
    float min;
    float max;
    float brownianFactor;
    
    Drunk drunk;
    
    Brownian(){};
    Brownian(float min, float max, float brownianFactor){
        setup(min, max, brownianFactor);
    }
    
    void setup(float min, float max, float brownianFactor){
        this->min = min;
        this->max = max;
        this->brownianFactor = brownianFactor;
        
        drunk.setup(65000, brownianFactor*65000);
    }
    
    void setBrownianFactor(float bf){
        this->brownianFactor = bf;
        drunk.setStep(brownianFactor * 65000);
    }
    
    float get(){
        return ((float)drunk.get()/65000.) * (max-min) + min;
    }
};

class BrownianRythm : public SelectableObjectBase{
public:
    BrownianRythm(){}
    BrownianRythm(int minED, int maxED, int EDval, float brownFactor){
        setup(minED, maxED, EDval, brownFactor);
        ofAddListener(parameters.parameterChangedE(), this, &BrownianRythm::onParameterChange);

    }
    
    ~BrownianRythm(){}
    
    void onParameterChange(ofAbstractParameter &p){
        brownian.setup(0, EDval, brownFactor);
    }

    
    void setup(int minED, int maxED, int EDval, float brownFactor){
        this->minED = minED;
        this->maxED = maxED;
        this->EDval = EDval;
        this->brownFactor = brownFactor;
        brownian.setup(0, EDval, brownFactor);
        timestamp = 0;
        ED = getED();
        SelectableObjectBase::setup("BrownianRythm");
    }
    
    float getED(){
        rowIndex = brownian.get();
        rowIndex = glm::clamp(rowIndex, 0, EDval.get());
        return transLog(rowIndex, minED, maxED, EDval-1);
    }
    
    bool update(){
        if(ofGetElapsedTimeMillis() > timestamp+ED){
            timestamp = ofGetElapsedTimeMillis();
            ED = getED();
            ofNotifyEvent(bang, ED, this);
            return true;
        }
        return false;
    }
    
    float transLog(float rowIndex, float startVal, float endVal, float steps){
        float a = pow(endVal/startVal, 1.0/steps);
        return pow(a, rowIndex)*startVal;
    }

    Brownian brownian;
    
    //IN
    
    ofParameter<int> minED{"minED", 100, 100, 10000};
    ofParameter<int> maxED{"maxED", 1000, 100, 10000};
    ofParameter<int> EDval{"EDval", 12, 1, 24};
    ofParameter<float> brownFactor{"brownFactor", 0.2, 0.01, 1.};
    
    ofParameterGroup parameters{"BrownianRythm", minED, maxED, EDval, brownFactor};
    //OUT
    //BANG
    int rowIndex;
    float ED;
    
    long timestamp;
    long nextBang;

    ofEvent<float> bang;
    
    
};

#endif /* BrownianRythm_h */
