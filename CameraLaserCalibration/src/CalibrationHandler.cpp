//
//  CalibrationHandler.cpp
//  CameraLaserCalibration
//
//  Created by Jonas Fehr on 19/07/2018.
//

#include "CalibrationHandler.hpp"
#include "LaserHandler.h"

using namespace ofxCv;
using namespace cv;

#pragma mark Setup

void CalibrationHandler::setup(){
   
    laserHandler.setup();
    

    cam.allocate(1280,720,OF_IMAGE_COLOR);
    
    camMat = toCv(cam);
    camMat = ipCam.get();
    cam.update();
    

    // init camera
    //    cam.setup(1980, 1024);
    
    imitate(undistorted, cam);
    
    // duplicate image properties into ofPixels
    
    imitate(previous, cam);
    imitate(diff, cam);
    
    
    // draw UI on the left
    screenRect.set(0,0,2560,1440);
    // use second screen for projection
    projectorRect.set(0,0,4096,4096);
    projectorFbo.allocate(projectorRect.width, projectorRect.height);
    
    // setup calibrator
    camProjCalib.setup(projectorRect.width, projectorRect.height);
//    setState(CAMERA);
    setState(PROJECTOR_STATIC);
//    setState(PROJECTOR_DYNAMIC);
//    setState(RUN);

    log() << "Calibration started at step : " << getCurrentStateString() << endl;
    
    
    // create UI with default settings
    setupDefaultParams();
    
    
    lastTime = 0;
    bProjectorRefreshLock = true;
    bLog = true;
    
}

void CalibrationHandler::setupDefaultParams(){
    
    appParams.setName("Application");
    appParams.add( diffMinBetweenFrames.set("Difference min between frames", 4.0, 0, 10) );
    appParams.add( timeMinBetweenCaptures.set("Time min between captures", 2.0, 0, 10) );
    
    boardsParams.setName("Boards Params");
    boardsParams.add( numBoardsFinalCamera.set("Num boards Camera", 20, 10, 50) );
    boardsParams.add( numBoardsFinalProjector.set("Num boards Projector", 12, 6, 50) );
    boardsParams.add( numBoardsBeforeCleaning.set("Num boards before cleaning", 8, 5, 20) );
    boardsParams.add( numBoardsBeforeDynamicProjection.set("Num boards before dynamic proj", 5, 3, 10) );
    boardsParams.add( maxReprojErrorCamera.set("Max reproj error Camera", 0.2, 0.1, 2.5) );
    boardsParams.add( maxReprojErrorProjector.set("Max reproj error Projector", 0.2, 0.1, 2.5) );
    
    imageProcessingParams.setName("Processing Params");
    imageProcessingParams.add( alternativeProcessing.set("alternative Processing", false) );
    imageProcessingParams.add( circleDetectionThreshold.set("Circle image threshold", 220, 150, 255) );
    imageProcessingParams.add( denoise.set("denoise", 1, 6, 12) );

    parameters.add( currStateString.set("Current State", getCurrentStateString()) );
    
    parameters.add(appParams);
    parameters.add(boardsParams);
    parameters.add(imageProcessingParams);
}

#pragma mark - App states

void CalibrationHandler::setState(CalibState state){
    
    CameraCalibration & calibrationCamera = camProjCalib.getCalibrationCamera();
    ProjectorCalibration & calibrationProjector = camProjCalib.getCalibrationProjector();
    
    switch (state) {
        case CAMERA:
            camProjCalib.resetBoards();
            break;
        case PROJECTOR_STATIC:
            calibrationCamera.load("calibrationCamera.yml");
            camProjCalib.resetBoards();
            calibrationCamera.setupCandidateObjectPoints();
            calibrationProjector.setStaticCandidateImagePoints();
            createLaserPattern();
            break;
        case PROJECTOR_DYNAMIC:
//            camProjCalib.load("calibrationCamera.yml", "calibrationProjector.yml", "CameraProjectorExtrinsics.yml");
//            camProjCalib.resetBoards();
//            calibrationCamera.setupCandidateObjectPoints();
//            calibrationProjector.setStaticCandidateImagePoints();


            break;
        case RUN:
            camProjCalib.load("calibrationCamera.yml", "calibrationProjector.yml", "CameraProjectorExtrinsics.yml");
            rotObjToProj = Mat::zeros(3, 1, CV_64F);
            transObjToProj = Mat::zeros(3, 1, CV_64F);
            rotObjToCam = Mat::zeros(3, 1, CV_64F);
            transObjToCam = Mat::zeros(3, 1, CV_64F);
            
    
            
            
            break;
        default:
            break;
    }
    currState = state;
    
    currStateString = getCurrentStateString();
    log() << "Set state : " << getCurrentStateString() << endl;
}

string CalibrationHandler::getCurrentStateString(){
    
    string name;
    switch (currState) {
        case CAMERA:            name = "CAMERA"; break;
        case PROJECTOR_STATIC:  name = "PROJECTOR_STATIC"; break;
        case PROJECTOR_DYNAMIC: name = "PROJECTOR_DYNAMIC"; break;
        default: break;
    }
    return name;
}

#pragma mark - Update



void CalibrationHandler::update(){
    
    camMat = ipCam.get();
    cam.update();
//    if(cam.isFrameNew())
    {
//        Mat camMat = toCv(cam);
        
        switch (currState) {
                
            case CAMERA:
                if( !updateCamDiff(camMat) ) return;
                
                if( calibrateCamera(camMat) ){
                    lastTime = ofGetElapsedTimef();
                }
                break;
                
            case PROJECTOR_STATIC:
                laserHandler.update();
                if( !updateCamDiff(camMat) ) return;
                
                if( calibrateProjector(camMat) ){
                    lastTime = ofGetElapsedTimef();
                }
                break;
                
            case PROJECTOR_DYNAMIC:
                laserHandler.update();
                if(bProjectorRefreshLock){
                    if( camProjCalib.setDynamicProjectorImagePoints(camMat) ){
                        createLaserPattern();
                        if (!updateCamDiff(camMat)) {
                            return;
                        }
                        bProjectorRefreshLock = false;
                    }
                }
                else {
                    if( calibrateProjector(camMat) ) {
                        lastTime = ofGetElapsedTimef();
                    }
                    bProjectorRefreshLock = true;
                }
                break;
            case RUN:
//                camProjCalib.setDynamicProjectorImagePoints(camMat);
                
                break;
                
            default: break;
        }
    }
}



bool CalibrationHandler::updateCamDiff(cv::Mat camMat) {
    
    Mat prevMat = toCv(previous);
    Mat diffMat = toCv(diff);
    absdiff(prevMat, camMat, diffMat);
    camMat.copyTo(prevMat);
    diffMean = mean(Mat(mean(diffMat)))[0];

    
    float timeDiff = ofGetElapsedTimef() - lastTime;
    
    return timeMinBetweenCaptures < timeDiff && diffMinBetweenFrames > diffMean;
}

void CalibrationHandler::processImageForCircleDetection(cv::Mat img){
    
    if(alternativeProcessing){
        // In this case, we will "blend" camMat and prevMat before adding the image to process, or better,
        // we will process both first for grid circles (i.e., threhold, erode and dilate), and then OR them:
        Mat im1, im2;
        
        if(img.type() != CV_8UC1) {
            cvtColor(img, im1, CV_RGB2GRAY);
        }
        threshold(im1, im1, 210, 255, THRESH_BINARY_INV);
        erode(im1, im1, Mat());
        dilate(im1, im1, Mat());
        dilate(im1, im1, Mat());
        erode(im1, im1, Mat());
        
        if(toCv(previous).type() != CV_8UC1) {
            cvtColor(toCv(previous), im2, CV_RGB2GRAY);
        }
        threshold(im2, im2, 210, 255, THRESH_BINARY_INV);
        erode(im2, im2, Mat());
        dilate(im2, im2, Mat());
        dilate(im2, im2, Mat());
        erode(im2, im2, Mat());
        
        //OR them:
        bitwise_and(im1,im2,processedImg);
        
    }else{
        if(img.type() != CV_8UC1) {
            cvtColor(img, processedImg, CV_RGB2GRAY);
        } else {
            processedImg = img;
        }
        cv::threshold(processedImg, processedImg, circleDetectionThreshold, 255, cv::THRESH_BINARY_INV);
        //morphological opening (removes small objects from the foreground)
        erode(processedImg, processedImg, getStructuringElement(MORPH_ELLIPSE, cv::Size(denoise, denoise)) );
        dilate( processedImg, processedImg, getStructuringElement(MORPH_ELLIPSE, cv::Size(denoise, denoise)) );
        
        //morphological closing (removes small holes from the foreground)
        dilate( processedImg, processedImg, getStructuringElement(MORPH_ELLIPSE, cv::Size(denoise, denoise)) );
        erode(processedImg, processedImg, getStructuringElement(MORPH_ELLIPSE, cv::Size(denoise, denoise)) );
    }
}

bool CalibrationHandler::calibrateCamera(cv::Mat img){
    
    CameraCalibration & calibrationCamera = camProjCalib.getCalibrationCamera();
    
    bool bFound = calibrationCamera.add(img);
    if(bFound){
        
        log() << "Found board!" << endl;
        
        calibrationCamera.calibrate();
        
        if(calibrationCamera.size() >= numBoardsBeforeCleaning) {
            
            log() << "Cleaning" << endl;
            
            if (calibrationCamera.clean(maxReprojErrorCamera) == false) {
                log() << "clean returns false" << endl;
            }
            else if(calibrationCamera.getReprojectionError(calibrationCamera.size()-1) > maxReprojErrorCamera) {
                log() << "Board found, but reproj. error is too high, skipping" << endl;
                return false;
            }
        }
        
        if (calibrationCamera.size()>=numBoardsFinalCamera) {
            
            // save camera intrinsics
            calibrationCamera.save("calibrationCamera.yml");
            
            log() << "Camera calibration finished & saved to calibrationCamera.yml" << endl;
            
            setState(PROJECTOR_STATIC);
        }
    } else log() << "Could not find board" << endl;
    
    return bFound;
}

bool CalibrationHandler::calibrateProjector(cv::Mat img){
    
    CameraCalibration & calibrationCamera = camProjCalib.getCalibrationCamera();
    ProjectorCalibration & calibrationProjector = camProjCalib.getCalibrationProjector();
    
    processImageForCircleDetection(img);
    
    if(camProjCalib.addProjected(img, processedImg)){
        
        log() << "Calibrating projector" << endl;
        
        calibrationProjector.calibrate();
        
        if(calibrationProjector.size() >= numBoardsBeforeCleaning) {
            
            log() << "Cleaning" << endl;
            
            int numBoardRemoved = camProjCalib.cleanStereo(maxReprojErrorProjector);
            
            log() << numBoardRemoved << " boards removed";
            
            if(currState == PROJECTOR_DYNAMIC && calibrationProjector.size() < numBoardsBeforeDynamicProjection) {
                log() << "Too many boards removed, restarting to PROJECTOR_STATIC" << endl;
                setState(PROJECTOR_STATIC);
                return false;
            }
        }
        
        log() << "Performing stereo-calibration" << endl;
        
        camProjCalib.stereoCalibrate();
        
        log() << "Done" << endl;
        
        if(currState == PROJECTOR_STATIC) {
            
            if( calibrationProjector.size() < numBoardsBeforeDynamicProjection) {
                log() << numBoardsBeforeDynamicProjection - calibrationProjector.size() << " boards to go before dynamic projection" << endl;
            } else {
                setState(PROJECTOR_DYNAMIC);
            }
            
        } else {
            
            if( calibrationProjector.size() < numBoardsFinalProjector) {
                log() << numBoardsFinalProjector - calibrationProjector.size() << " boards to go to completion" << endl;
            } else {
                calibrationProjector.save("calibrationProjector.yml");
                log() << "Projector calibration finished & saved to calibrationProjector.yml" << endl;
                
                camProjCalib.saveExtrinsics("CameraProjectorExtrinsics.yml");
                log() << "Stereo Calibration finished & saved to CameraProjectorExtrinsics.yml" << endl;
                
                log() << "Congrats, you made it ;)" << endl;
                
                setState(RUN);
            }
        }
        return true;
    }
    return false;
}


#pragma mark - Draw

void CalibrationHandler::draw(){
    //    cam.update();
    ofSetColor(255);
    //    cam.draw(0, 0);
    ofxCv::drawMat(camMat,0,0);
    
    drawFoundImagePoints(camProjCalib.getCalibrationCamera());
    
    
    drawIntrinsics("Camera", camProjCalib.getCalibrationCamera(), 20);
    drawIntrinsics("Projector", camProjCalib.getCalibrationProjector(), 40);
    
    ofDrawBitmapStringHighlight("Movement: "+ofToString(diffMean), 10, 60, ofxCv::cyanPrint);
    
    drawReprojErrors("Camera", camProjCalib.getCalibrationCamera(), 80);
    
    
    switch (currState) {
        case CAMERA:
            drawReprojLog(camProjCalib.getCalibrationCamera(), 100);
            if(camProjCalib.getCalibrationCamera().size() > 0) {
                camProjCalib.getCalibrationCamera().undistort(camMat, toCv(undistorted));
                undistorted.update();
                undistorted.draw(0, 0, 720, 1280/4, 720/4);
                ofSetColor(255, 80);
                undistorted.draw(0, 0);
                ofSetColor(255);
                
            }
            break;
        case PROJECTOR_STATIC:
        case PROJECTOR_DYNAMIC:
            drawReprojErrors("Projector", camProjCalib.getCalibrationProjector(), 100);
            drawReprojLog(camProjCalib.getCalibrationProjector(), 120);
            
            camProjCalib.getCalibrationCamera().undistort(camMat, toCv(undistorted));
            undistorted.update();
            undistorted.draw(0, 0, 720, 1280/4, 720/4);
            ofSetColor(255, 80);
            undistorted.draw(0, 0);
            ofSetColor(255);
            if(ofxCv::getAllocated(processedImg)){
                ofxCv::drawMat(processedImg,1280/4, 720, 1280/4, 720/4);
            }
            
            
            laserHandler.draw(1280*3/4,720,320,320);
            
            break;
            
        case RUN:
        {
            // set some input points
            float w = 0.001/2;
            float h = w;
            vector<Point3f> inPts;
            inPts.push_back(Point3f(-w, -h, 0));
            inPts.push_back(Point3f(w, -h, 0));
            inPts.push_back(Point3f(w, h, 0));
            inPts.push_back(Point3f(-w, h, 0));
            
            // get videoproj's projection of inputPts
            vector<cv::Point2f> outPts = camProjCalib.getProjected(inPts, rotObjToCam, transObjToCam);
            
            // project the inputPts over the object
            ofPushMatrix();
            ofTranslate(1280/2, 720/2);
            ofSetColor(ofColor::red);
            for (int i=0; i<outPts.size(); i++){
                int next = (i+1) % outPts.size();
                ofDrawLine(outPts[i].x, outPts[i].y, outPts[next].x, outPts[next].y);
            }
            ofPopMatrix();
            
            laserHandler.draw(1280/2,720,320,320);
            
        }
            break;
            
        default:
            break;
    }
    
    
    
    
    ofDrawBitmapString(getLog(10), 10, 720+720/4+20);
    //    projectorFbo.draw(640,480,409,409);
    //    ofNoFill();
    //    ofSetColor(150);
    //    ofDrawRectangle(640,480,409,409);
    //    ofPushMatrix();
    //    ofTranslate(640,480);
    //    ofScale(0.1);
    //    drawFoundImagePoints(camProjCalib.getCalibrationProjector());
    //    ofPopMatrix();

}

void CalibrationHandler::drawFoundImagePoints(const ofxCv::Calibration & calib){
    const auto & imagePoints = calib.imagePoints;
    
    if(imagePoints.size()<=0) return;
    bool firstTime = true;
    ofPushStyle(); ofSetColor(ofColor::cyan); ofFill();
    for(const auto & p : imagePoints.back()) {
        if(!firstTime) ofSetColor(ofColor::orange); firstTime = false;
        
        ofDrawCircle(ofxCv::toOf(p), 3);
    }
    ofPopStyle();
}

void CalibrationHandler::drawIntrinsics(string name, const ofxCv::Calibration & calib, int y){
    
    stringstream intrinsics;
    intrinsics << "fov: " << toOf(calib.getDistortedIntrinsics().getFov()) << " distCoeffs: " << calib.getDistCoeffs();
    string oneLine = intrinsics.str();
    ofStringReplace(oneLine, "\n", "");
    ofDrawBitmapStringHighlight(name+" "+oneLine, 10, y, yellowPrint, ofColor(0));
}


void CalibrationHandler::drawReprojErrors(string name, const ofxCv::Calibration & calib, int y){
    string buff;
    buff = name + " Reproj. Error: " + ofToString(calib.getReprojectionError(), 2);
    buff += " from " + ofToString(calib.size());
    ofDrawBitmapStringHighlight(buff, 10, y, ofxCv::magentaPrint);
}

void CalibrationHandler::drawReprojLog(const ofxCv::Calibration & calib, int y) {
    string buff;
    for(int i = 0; i < calib.size(); i++) {
        buff = ofToString(i) + ": " + ofToString(calib.getReprojectionError(i));
        ofDrawBitmapStringHighlight(buff, 10, y + 16 * i, ofxCv::magentaPrint);
    }
}

void CalibrationHandler::createProjectorPattern(){
    
    projectorFbo.begin();
    {
        ofBackground(0);
        
        //    ofRectangle vp = ofGetCurrentViewport();
        //    ofViewport(projectorRect);
        //    glMatrixMode(GL_PROJECTION);
        //    glLoadIdentity();
        //    glOrtho(0, projectorRect.width, projectorRect.height, 0, -1, 1);
        //    glMatrixMode(GL_MODELVIEW);
        //    glLoadIdentity();
        {
            
            ofSetColor(ofColor::white);
            ofFill();
            for(const auto & p : camProjCalib.getCalibrationProjector().getCandidateImagePoints()) {
                ofDrawCircle(p.x, p.y, 10);
            }
        }
        //    ofViewport(vp);
    }
    projectorFbo.end();
}

void CalibrationHandler::createLaserPattern(){
    vector<ofPolyline> laserPolys;
    for(const auto & p : camProjCalib.getCalibrationProjector().getCandidateImagePoints()) {
        ofPolyline laserPoly;
        glm::vec3 pos = glm::vec3(p.x/4096.f, p.y/4096.f,0);
        if(pos.x < 0 || pos.x > 1) continue;
        if(pos.y < 0 || pos.y > 1) continue;
        laserPoly.addVertex(pos);
        laserPolys.push_back(laserPoly);
    }
    laserHandler.set(laserPolys);
}



#pragma mark - Log

string CalibrationHandler::getLog(int numLines){
    
    vector<string> elems;
    stringstream ss(log_.str());
    string buff;
    while (getline(ss, buff)) elems.push_back(buff);
    numLines = fmin(elems.size(), numLines);
    buff = "";
    if(elems.size()>0) {
        for (int i=elems.size()-1; i>elems.size()-numLines; i--) {
            buff = elems[i] + "\n" + buff;
        }
    }
    return buff;
}
