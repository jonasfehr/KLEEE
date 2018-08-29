//
//  LaserHandler.h
//  cameraLaserCalibration
//
//  Created by Jonas Fehr on 09/08/2018.
//

#ifndef LaserHandler_h
#define LaserHandler_h

#include "ofxRayComposer.h"
#include "ofxIldaFrame.h"
#include "ofxGui.h"

class LaserHandler{
public:
    ofxRayComposer dac;
    ofxIlda::Frame ildaFrame;
    
    ofParameterGroup parameters;
    ofParameter<bool> bTestframe;
    ofPolyline testRect;
    vector<ofPolyline> laserPolys;
    
    
    void setup(){
        dac.setup(true, 0, true);
        dac.setPPS(18000);
        ildaFrame.setup();
        
        
        parameters.add(bTestframe.set("showTestRect", false));
        parameters.add(ildaFrame.parameters);
        
        createTestRect();
        
    }
    
    void set(vector<ofPolyline> laserPolys){
            this->laserPolys = laserPolys;
    }
    
    void update(){
        ildaFrame.clear();
        
        if(bTestframe){
            ildaFrame.addPoly(testRect,ofFloatColor(1.0,1.0,1.0,1.0));
        }else{
            ildaFrame.addPolys(laserPolys,ofFloatColor(1.0,1.0,1.0,1.0));
        }
        
        ildaFrame.update();
        dac.setPoints(ildaFrame);
    }
    
    void draw(int x, int y, int w, int h){
            ildaFrame.draw(x,y,w,h);
            ofNoFill();
            ofSetColor(150);
            ofSetLineWidth(2);
            ofDrawRectangle(x,y,w,h);
    }
    
    void close(){
    }
    
    void createTestRect(){
        testRect.addVertex(0.001, 0.001);
        testRect.addVertex(0.999, 0.001);
        testRect.addVertex(0.999, 0.999);
        testRect.addVertex(0.001, 0.999);
        testRect.close();
    }
    
};

#endif /* LaserHandler_h */
