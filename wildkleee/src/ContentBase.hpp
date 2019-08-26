//
//  ContentBase.hpp
//  ByensPuls
//
//  Created by Jonas Fehr on 13/07/2019.
//

#ifndef ContentBase_hpp
#define ContentBase_hpp

#include <stdio.h>
#include "ofMain.h"
#include "ofxIldaPoint.h"



class ContentBase{
public:
    vector<ofxIlda::Point> points;
    vector<vector<ofxIlda::Point>> pointGroups;

    virtual void setup(){};
    virtual void update(){};
    virtual bool isDead(){ return false; };

    void clear(){
        points.clear();
        pointGroups.clear();
    }
    
    void addPoint(glm::vec2 pos, float brightness){
        points.push_back(ofxIlda::Point( pos, ofFloatColor(brightness,1.)));
    };
    
    void addPoint(ofxIlda::Point point){
        points.push_back(point);
    };
    
    void addPointGroup(vector<ofxIlda::Point> pointGroup){
        pointGroups.push_back(pointGroup);
    };
    
    void closePointGroup(){
        pointGroups.push_back(points);
        points.clear();
    };
    
    int size(){
        return pointGroups.size();
    }
    
    vector<ofxIlda::Point> getPoints(){
        return points;
    }
    
    vector<ofxIlda::Point> getPointGroup(int index){
        if(index < size()){
            return pointGroups[index];
        } else{
            vector<ofxIlda::Point> emptyPoints;
            return emptyPoints;
        }
    };
    
    vector<vector<ofxIlda::Point>> getPointGroups(){
        return pointGroups;
    }
    
    bool isDirty = true;
    
//    int getPriority(){ return priority; }

//    enum ContentPriority{
//        CP_LOW,
//        CP_MID,
//        CP_HIGH
//    } priority;
//
//
//    enum ContentType{
//        CT_WALKER,
//        CT_PULSE,
//        CT_ZONE
//    } type;

};
#endif /* ContentBase_hpp */
