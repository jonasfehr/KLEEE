#pragma once

#include "ofMain.h"

#include "CalibrationHandler.hpp"
#include "LaserHandler.h"
#include "ofxGui.h"


class ofApp : public ofBaseApp {
public:
	void setup();
	void update();
    void draw();
    void exit();
	void keyPressed(int key);
    
    CalibrationHandler calibrationHandler;
    ofxPanel guiCalibration;
    ofxPanel guiLaser;

};

