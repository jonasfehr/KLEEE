#pragma once

#include "ofMain.h"

#include "ofxRayComposer.h"
#include "ofxIldaFrame.h"
#include "ofxGui.h"
#include "ofxIpCamStreamer.h"
#include "ofxOscParameterSync.h"

#include "Walker.hpp"
#include "Segmentator.hpp"


class ofApp : public ofBaseApp{
public:
    
    enum Status{
        UNDEFINED = 0,

        SELECT_ROI = 1,
        SUPERPIXELS = 2,
        RUN = 3,
        CALIB_POINTS = 4
        
    };// state;
    
    string currentStateStr;

    
		void setup();
        void setStatus(int & state);
		void update();
		void draw();
        void exit();

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
    
    
    ofxPanel guiLaser;
    ofxPanel guiIPCam;
    ofxPanel guiSegmentation;
    ofParameter<ofFloatColor> crossColor{ "crossColor", ofFloatColor::black };
    ofParameterGroup crossParam = {"crossParam", crossColor };
    ofxPanel guiCrossParam;
    ofParameterGroup unitedParameters;
    
    ofxPanel gui;
    ofParameter<bool> isActivated{"isActivated", false};
    //    ofParameter<bool> showTestPattern{"showTestPattern", false};
    ofParameter<int> LASER_PPSx1000{"LASER_PPSx1000", 30, 0.1, 30};
    ofParameterGroup parameters{"Laser", isActivated, LASER_PPSx1000};

    void listenerFunction(ofAbstractParameter& e);


    Walker walker;
    
//    BlobFinder blobFinder;
    
//    ofxLaserProjector laserProjector;
    
    ofxRayComposer dac;
    ofxIlda::Frame ildaFrame;

    ofxIpCamStreamer ipCam;
    Mat camMat;
    Mat roiMat;
    void savePoints(string filename, string pointName, vector<glm::vec2> & points);
    void loadPoints(string filename, string pointName, vector<glm::vec2> & points);
    ofJson vec3ToJson(glm::vec3 vec);
    glm::vec3 jsonToVec3(ofJson json);
    ofJson vec2ToJson(glm::vec2 vec);
    glm::vec2 jsonToVec2(ofJson json);
    
    vector<glm::vec2> srcPoints;
    vector<glm::vec2> calibPoints;

    // variables for dragging points
    bool movingPoint;
    glm::vec2* curPoint;

    ofPolyline testRect;
    void createTestRect(){
        testRect.addVertex(0.001, 0.001);
        testRect.addVertex(0.999, 0.001);
        testRect.addVertex(0.999, 0.999);
        testRect.addVertex(0.001, 0.999);
        testRect.close();
    }
    
    Segmentator segmentator;
    
    ofImage imageLoad;
    
    bool cmdDown;
    
    string type2str(int type) {
        string r;
        
        uchar depth = type & CV_MAT_DEPTH_MASK;
        uchar chans = 1 + (type >> CV_CN_SHIFT);
        
        switch ( depth ) {
            case CV_8U:  r = "8U"; break;
            case CV_8S:  r = "8S"; break;
            case CV_16U: r = "16U"; break;
            case CV_16S: r = "16S"; break;
            case CV_32S: r = "32S"; break;
            case CV_32F: r = "32F"; break;
            case CV_64F: r = "64F"; break;
            default:     r = "User"; break;
        }
        
        r += "C";
        r += (chans+'0');
        
        return r;
    }
    
    ofxOscParameterSync sync;
    ofParameter<int> state;
    int currentState;
    


};
