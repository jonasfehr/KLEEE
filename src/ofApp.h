#pragma once

#include "ofMain.h"
//#include "ofxImGui.h"

#include "Walker.h"
#include "BlobFinder.h"
//#include "ofxLaserProjector.h"
#include "ofxRayComposer.h"
#include "ofxIldaFrame.h"
#include "ofxGui.h"

class ofApp : public ofBaseApp{
public:

    
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
    
    
    // Gui
//    bool imGui();
//    ofxImGui::Gui gui;
//    bool guiVisible;
//    bool mouseOverGui;
    
    ofxPanel gui;
    ofParameter<bool> isActivated{"isActivated", false};
    //    ofParameter<bool> showTestPattern{"showTestPattern", false};
    ofParameter<int> LASER_PPSx1000{"LASER_PPSx1000", 30, 0.1, 30};
    ofParameterGroup parameters{"Laser", isActivated, LASER_PPSx1000};

    void listenerFunction(ofAbstractParameter& e);


    Walker walker;
    
    BlobFinder blobFinder;
    
//    ofxLaserProjector laserProjector;
    
    ofxRayComposer dac;
    ofxIlda::Frame ildaFrame;

};
