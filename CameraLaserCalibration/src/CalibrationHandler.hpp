//
//  CalibrationHandler.hpp
//  CameraLaserCalibration
//
//  Created by Jonas Fehr on 19/07/2018.
//

#ifndef CalibrationHandler_hpp
#define CalibrationHandler_hpp

#include "ofxGui.h"
#include "ofxCv.h"
#include "ofxCvCameraProjectorCalibration.h"
#include "ofxIpCamStreamer.h"
#include "LaserHandler.h"

enum CalibState {
    CAMERA,
    PROJECTOR_STATIC,
    PROJECTOR_DYNAMIC,
    RUN
};

class CalibrationHandler{
public:
    void setup();
    void update();
    void draw();
    
    ofxCv::CameraProjectorCalibration camProjCalib;
    
    //    ofVideoGrabber cam;
    ofxIpCamStreamer ipCam;
    Mat camMat;
    ofImage cam;
    
    LaserHandler laserHandler;
    
    void setState(CalibState state);
    string getCurrentStateString();
    
    bool calibrateCamera(cv::Mat img);
    bool calibrateProjector(cv::Mat img);
    
private:
    
    CalibState currState;
    
    // circles detection
    
    void processImageForCircleDetection(cv::Mat camMat);
    cv::Mat processedImg;
    ofImage ofImg;
    
    // board holding movement
    ofImage undistorted;
    ofPixels previous;
    ofPixels diff;
    float diffMean;
    float lastTime;
    bool updateCamDiff(cv::Mat camMat);
    
    // lock to give enough time for the projection to be seen by the camera
    
    bool bProjectorRefreshLock;
    
    // screen & projector configuration
    
    ofRectangle projectorRect;
    ofRectangle screenRect;
    
    // draw
    void drawIntrinsics(string name, const ofxCv::Calibration & calib, int y);
    void drawReprojErrors(string name, const ofxCv::Calibration & calib, int y);
    void drawReprojLog(const ofxCv::Calibration & calib, int y);
    void drawFoundImagePoints(const ofxCv::Calibration & calib);
    void createProjectorPattern();
    void createLaserPattern();

    // log
    
    stringstream log_;
    stringstream & log() {
        return log_;
    }
    bool bLog;
    string getLog(int numLines=15);
    
    // params
    
    void setupDefaultParams();
public:
    ofParameterGroup parameters;

    ofParameterGroup appParams;
    ofParameter<float> diffMinBetweenFrames;
    ofParameter<float> timeMinBetweenCaptures;
    ofParameter<string> currStateString;
    
    ofParameterGroup boardsParams;
    ofParameter<int> numBoardsFinalCamera;
    ofParameter<int> numBoardsFinalProjector;
    ofParameter<int> numBoardsBeforeCleaning;
    ofParameter<int> numBoardsBeforeDynamicProjection;
    ofParameter<float> maxReprojErrorCamera;
    ofParameter<float> maxReprojErrorProjector;
    
    ofParameterGroup imageProcessingParams;
    ofParameter<bool> alternativeProcessing;
    ofParameter<int> circleDetectionThreshold;

    ofFbo projectorFbo;
    
    cv::Mat rotObjToCam, transObjToCam;
    cv::Mat rotObjToProj, transObjToProj;
    
};
/*
 From Alvaro:
 https://www.youtube.com/watch?v=pCq7u2TvlxU
 
 Basically the procedure is broken in three steps:
 
 #CAMERA
 1) start by first calibrating the camera, and saving the intrinsics. (ofApp::calibrateCamera)
 
 #PROJECTOR_STATIC
 2) Camera/projector calibration starts: the projector projects a grid of circles
 First in a fixed position, then as the calibration achieved some accuracy, the grid start following the printed pattern.
 This step is as follows:
 (a) the camera is used to compute the 3d position of the projected circles
 (by backprojection) in camera coordinate system,
 and then in "board" coordinate system (or "world coordinates").
 This is done in the method (Calibration::backProject) called only when the object is of type "camera".
 (b) This means we can start computing the instrinsics of the projector
 because you have 3d points (the projected circles) in world coordinates,
 and their respective "projection" in projector "image" plane.
 (c) Finally, we can start computing the extrinsics of the camera-projector
 (because basically we have 3d points in "world coordinates" (the board),
 which are the projected circles, and also their projection (2d points) in the camera image and projector "image").
 This means you can use the standard stereo calibration routine in openCV
 The reason why we compute FIRST the instrinsics of the projector
 is because the method stereoCalibrationCameraProjector will call the openCV stereo calibration function
 using "fixed intrinsics" to ensure better convergence of the algorithm
 (presumably better because if the camera is very well calibrated first,
 then the instrinsics of the projector should be good -
 probably better than if recomputed from all the 3d points and image points in camera and projector).
 
 #PROJECTOR_DYNAMIC
 3) Once we get a good enough reprojection error (for the projector), we can start moving the projected points around
 so as to better explore the space (and get better and more accurate calibration).
 In this phase, we can run openCV stereo calibration to obtain the camera-projector extrinsics and again,
 we don't need to recompute the instrinics of the projector.
 After a few cycles (and cleaning of bad "boards"), the process converges, data is saved and you can do AR.
 There is a lot going on the code to ensure we can do this in real time,
 and acquire data when the boards are not moving a lot, etc).
 
 How to use:
 
 Print the chessboard_8x5.pdf and glue it to a larger white board as in the video (https://www.youtube.com/watch?v=pCq7u2TvlxU).
 Note the orientation of the pattern: if you glue it the other way, the dynamically projected pattern will be outside the board).
 The actual size of the board is not important. If you want to get real distances (say, in cm or mm in the extrinsics),
 then you want to measure the size of the elemental squares and indicate it in the .yml file settingsPatternCamera.yml:
 %YAML:1.0 xCount: 4 yCount: 5 squareSize: 30 << -- here, size in cm or mm of the elemental square posX: 400 posY: 250 patternType: 2 color: 255
 Also, check some comments on the header of the testApp.h file.
 In particular, it is important that you #define the size of the computer screen and the projector screen:
 
 cam.initgrabber
 screenRect
 projectorRect
 
 The process goes in three phases:
 (a) calibrating the camera (you may change the number of "good" pre-calibration boards by checking it on the testApp.cpp file,
 as well as the meaning of "good" by playing with the value of maxErrorCamera):
 const int preCalibrateCameraTimes = 15; // this is for calibrating the camera BEFORE starting projector calibration.
 const int startCleaningCamera = 8; // start cleaning outliers after this many samples (10 is ok...).
 Should be < than preCalibrateCameraTimes const float maxErrorCamera=0.3;
 
 (b) The camera is then used to compute the 3d position of the projected pattern (asymmetric circles) by backprojection,
 as long as both patterns are visible at the same time (acquisition is automatic) and is indicated as a red flash on the projected pattern.
 In this phase, stereo calibration is performed to obtain the camera-projector extrinsics.
 The process of projector calibration goes exactly like that for the camera,
 meaning that you need to set the maxErrorProjector, as well as the minimum number of "good" boards).
 There is an additional parameter "startDynamicProjectorPattern": this is the minimum of good boards before starting projecting "dynamically".
 When this number of boards is reached, the projection will follow the printed pattern and you can start moving the board around
 so as to better explore the whole field of view space (and get better and more accurate calibration):
 const float maxErrorProjector=0.30;
 const int startCleaningProjector = 6;
 const int startDynamicProjectorPattern=5; <<-- after this number of projector/camera calibration,
 the projection will start following the printed pattern
 to facilitate larger exploration of the field of view.
 If this number is larger than minNumGoodBoards, then this will never happen automatically.
 const int minNumGoodBoards=15; <<-- after this number of simultaneoulsy acquired "good" boards,
 IF the projector total reprojection error is smaller than a certain threshold, we end calibration (and move to AR mode automatically)
 
 (c) Once minNumGoodBoards are acquired for both the camera and projector,
 extrinsics and intrinsics are saved, and the program goes into a simple AR demo mode, projecting some dots over the printed pattern.
 Note that this may affect the printed board detection, but it is just for trying (normally you would use another kind of fiducial,
 for instance a marker or the corners of a board, and project on the side or inside the board, not OVER the printed fiducials...)
 
 TODOS:
 - solve inconsistence between openGL and openCV "manual" projection.
 Something to do with the full screen mode in dual screen?
 - pre-processing of acquired images with color based segmentation
 (this data should be in the pattern calibration file)
 - color picker
 - 3d representation of camera/projector configuration in a separate viewport
 (with axis and mouse controlled rotation to check some things, like camera orientation, etc)
 - openCV 2.4 would avoid slowing down image acquisition when the board is not on the image.
 
 DEMOS:
 use the modified rectangle finder (with tracking) to do AR.
 
 */

#endif /* CalibrationHandler_hpp */
