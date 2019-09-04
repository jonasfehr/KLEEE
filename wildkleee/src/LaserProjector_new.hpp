//
//  LaserProjector_new.hpp
//  ByensPuls
//
//  Created by Jonas Fehr on 10/07/2019.
//

#ifndef LaserProjector_new_hpp
#define LaserProjector_new_hpp

#include <stdio.h>

#include "ofxGui.h"
#include "ofxRayComposer.h"
#include "ofxIldaFrame.h"
#include "SelectableObjectBase.hpp"

class LaserProjector_new : public SelectableObjectBase{
public:
    
    
    LaserProjector_new(string key, string deviceId, ofxRayComposer::Handler & rcHandler){
        SelectableObjectBase::setup(key);
        
        gui.setup(key);
        gui.add(parameters);
        
        ofAddListener(parameters.parameterChangedE(), this, &LaserProjector_new::onParameterChange);
        
        this->rcHandler = &rcHandler;
        
        
        connectDevice(deviceId);
        if(dacIsConnected) dac->setPPS(LASER_PPSx1000*1000);
        
        ildaFrame.setup();
        
        
        ofAddListener(this->rcHandler->newDeviceFoundE, this, &LaserProjector_new::connectDevice);
    }
    
    ~LaserProjector_new(){
        ofRemoveListener(parameters.parameterChangedE(), this, &LaserProjector_new::onParameterChange);
        ofRemoveListener(this->rcHandler->newDeviceFoundE, this, &LaserProjector_new::connectDevice);
        
    }
    
    void connectDevice(string & deviceName){
        if(dacIsConnected && deviceName == rcDeviceID){
            return;
        } else {
            if(dacIsConnected && dac != NULL){
                dac->clear();
//                dac->stop();
            }
            rcDeviceID = deviceName;
            dacIsConnected = this->rcHandler->referenceInterface(dac, rcDeviceID);
        }
    }
    
    void onParameterChange(ofAbstractParameter& e){
        if(dacIsConnected && dac != NULL) dac->setPPS(LASER_PPSx1000*1000);
    }
    
    void draw(ofRectangle rect){
        
        
        SelectableObjectBase::draw();
        
        ofSetColor(ofColor::white);
        int numPoints = 0;
        for(auto & pointGroup : pointGroups){
            numPoints += pointGroup.size();
        }
        ofDrawBitmapString(ofToString(ildaFrame.getPointsDac().size()), clickableSurface.getLeft()+90, clickableSurface.getBottom()-5);
        
        ofPushStyle();
        {
            if(dacIsConnected){
                ofSetColor(ofColor::green);
            } else {
                ofSetColor(ofColor::red);
            }
            float space = 2.5;
            int dia = clickableSurface.getHeight()-2*space;
            ofDrawEllipse(clickableSurface.getRight()-dia/2-space, clickableSurface.getTop()+clickableSurface.getHeight()/2, dia, dia);
        }
        ofPopStyle();
        
//        if(isActive())
            ildaFrame.draw(rect);
    }
    
    void update(){
        //        // IF NOT CONNECTED TRY TO FIND AND CONNECT;
        //        if(!dacIsConnected){
        //            rcHandler->init();
        //            dacIsConnected = this->rcHandler->referenceInterface(dac, "test");
        //        }
        ildaFrame.clear();
        ildaFrame.addPointGroups(pointGroups);
        
        ildaFrame.update();
        
        // send points to the DAC
        if(isActivated && dacIsConnected) dac->setPoints(ildaFrame);
        
        if(dac != NULL) dac->update();
    }
    
    
    void clearPoints(){
        points.clear();
        pointGroups.clear();
    }
    
    void addPoint(ofxIlda::Point point){
        points.push_back(point);
    }
    
    void addPointsToGroup(){
        if(points.size()>0) pointGroups.push_back(points);
        points.clear();
    }
    
    void addPointGroup(vector<ofxIlda::Point> pointGroup){
        pointGroups.push_back(pointGroup);
    }
    
    void serialize(ofJson & js){
        ofSerialize(js, parameters);
        js["deviceID"] = rcDeviceID;
        
    }
    void deserialize(ofJson & js){
        ofDeserialize(js, parameters);
        if(js["deviceID"].is_string()){
            string deviceName = js["deviceID"].get<std::string>();
            connectDevice(deviceName);
        }
    }
    
    
    ofxRayComposer::Handler * rcHandler;
    std::shared_ptr<ofxRayComposer::Interface> dac;
    bool dacIsConnected;
    string rcDeviceID;
    
    ofxIlda::Frame ildaFrame;
    
    
    
    // --- GUI
    ofxPanel gui;
    ofParameter<bool> isActivated{"isActivated", false};
    //    ofParameter<bool> showTestPattern{"showTestPattern", false};
    ofParameter<int> LASER_PPSx1000{"LASER_PPSx1000", 30, 0.1, 30};
    ofParameter<bool> showCross{"showSquare", false};
    ofParameter<bool> flipX{"flipX", false};
    ofParameter<bool> flipY{"flipY", false};
    
    ofParameterGroup parameters{"Laser", isActivated, LASER_PPSx1000, showCross, flipX, flipY, ildaFrame.params.output.galvoCorrection, ildaFrame.params.output.startBlanks, ildaFrame.params.output.startCount, ildaFrame.params.output.endCount, ildaFrame.params.output.endBlanks, ildaFrame.params.output.blackMoveMinDist, ildaFrame.params.output.masterColor, ildaFrame.params.draw.finalPoints, ildaFrame.params.draw.moveInBlack};
    
    
    
    vector<vector<ofxIlda::Point>> pointGroups;
    vector<ofxIlda::Point> points;
    
    ofPolyline cross;
    
    
    
    
};
#endif /* LaserProjector_new_hpp */


