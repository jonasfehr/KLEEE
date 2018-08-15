#include "ofApp.h"
#include "ofAppGLFWWindow.h"

void ofApp::setup(){
    
    
//    ((ofAppGLFWWindow*)ofGetWindowPtr())->setMultiDisplayFullscreen(true);
//    ofSetFullscreen(true);
//    ofSetVerticalSync(true);
    ofEnableAlphaBlending();
    ofEnableSmoothing();
    ofSetCircleResolution(30);
    ofBackground(0,0,0);
	ofSetFrameRate(30);
    
    calibrationHandler.setup();
   
    

    guiCalibration.setup("Calibration");
    guiCalibration.add(calibrationHandler.parameters);
    guiCalibration.setPosition(640*1.5 + 10, 10);
    guiCalibration.loadFromFile("settings.xml");
    
    guiLaser.setup("Laser");
    guiLaser.add(calibrationHandler.laserHandler.parameters);
    guiLaser.setPosition(1280,10);
    guiLaser.loadFromFile("settings.xml");
    
}


void ofApp::update(){
    calibrationHandler.update();
}

void ofApp::draw(){
    ofSetColor(255);
    calibrationHandler.draw();
    
    
    guiCalibration.draw();
    guiLaser.draw();

    ofSetWindowTitle("FPS: "+ofToString(ofGetFrameRate()));
}

void ofApp::exit(){
    calibrationHandler.ipCam.close();
    calibrationHandler.laserHandler.close();

}

void ofApp::keyPressed(int key){
    if(key=='s') calibrationHandler.setState(PROJECTOR_STATIC);
    if(key=='d') calibrationHandler.setState(PROJECTOR_DYNAMIC);
    if(key=='c') calibrationHandler.setState(CAMERA);
    if(key=='x') {
        // save camera intrinsics
        calibrationHandler.camProjCalib.getCalibrationCamera().save("calibrationCamera.yml");
        calibrationHandler.setState(PROJECTOR_STATIC);
    }
    
    if(key=='t') {
//        bTestframe = !bTestframe;
    }


}


