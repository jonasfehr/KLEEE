#include "ofApp.h"

#define CAM_PREVIEW_W 1280
#define CAM_PREVIEW_H 720
#define ROI_PREVIEW_W 600
#define ROI_PREVIEW_H 600
//--------------------------------------------------------------
void ofApp::setup(){
    dac.setup(true, 0, true);
    dac.setPPS(LASER_PPSx1000*1000);
    ildaFrame.setup();
    
    // Guis
    gui.setup();
    gui.add(walker.parameters);
    gui.setPosition(220,10);
    sync.setup((ofParameterGroup&)gui.getParameter(),8888,"localhost",8889);


    guiIPCam.setup();
    guiIPCam.add(ipCam.parameters);
    guiIPCam.setPosition(220,10);
    
    guiLaser.setup();
    guiLaser.add(parameters);
    guiLaser.add(ildaFrame.parameters);
    
    guiSegmentation.setup();
    guiSegmentation.add(segmentator.parametersFilters);
    guiSegmentation.add(segmentator.parametersSLIC);

    ofAddListener(parameters.parameterChangedE(), this, &ofApp::listenerFunction);
    
    createTestRect();
    
    roiMat.create(1024,1024,CV_8UC3);
    camMat = Mat::zeros(1920,1080,CV_8UC3);
    
    setStatus(SELECT_ROI);
    firstTime = false;
}

void ofApp::listenerFunction(ofAbstractParameter& e){
    dac.setPPS(LASER_PPSx1000*1000);
}

void ofApp::setStatus(int newState){
    switch(newState){
            
        case SELECT_ROI:
        {
            state = SELECT_ROI;
            currentState = "Select ROI";
            ofSetFrameRate(30);
            
            guiLaser.loadFromFile("settingsLaser_SelectROI.json");
            isActivated = false;
            
            // Setup
            srcPoints.clear();
            srcPoints.push_back(glm::vec2(100,10));
            srcPoints.push_back(glm::vec2(730,10));
            srcPoints.push_back(glm::vec2(700,700));
            srcPoints.push_back(glm::vec2(80,710));
            
            loadRoi("settingRoi.json");
        } break;
            
        case SEGMENTATION:
        {
            state = SEGMENTATION;
            currentState = "Segmentation";
            ofSetFrameRate(30);
            
            guiSegmentation.loadFromFile("settingsSegmentation.json");
            
            segmentator.doUpdate = true;
            
            
            ofImage ofImg;
            toOf(camMat, ofImg);
            ofImg.save("captures/inputImages/"+ofGetTimestampString("%y%m%d_%H-%M-%S")+"_inputImage.jpg", OF_IMAGE_QUALITY_BEST);
            
            toOf(roiMat, ofImg);
            ofImg.save("captures/roi/"+ofGetTimestampString("%y%m%d_%H-%M-%S")+"_roi.jpg", OF_IMAGE_QUALITY_BEST);
        } break;
            
        case RUN:
        {
            firstTime = false;
            state = RUN;
            currentState = "wandernder Punkt";
            ofSetFrameRate(60);
            
            guiLaser.loadFromFile("settingsLaser_Run.json");
            gui.loadFromFile("settingsWalker.json");
            
            SuperPixel spSky;
            for(auto & sp: segmentator.superPixels)if(sp.isTopRow) walker.setBoundaryPixels(sp.getBoundaryPixels());
        } break;
            
        default:
            break;
    }
}

//--------------------------------------------------------------
void ofApp::update(){
    switch(state){

        case SELECT_ROI:
        {
            camMat = ipCam.get();
            
            vector<cv::Point2f> srcPts;
            for(auto & p :srcPoints) {
                srcPts.push_back(cv::Point2f(p.x*camMat.cols/CAM_PREVIEW_W,p.y*camMat.rows/CAM_PREVIEW_H));
            }
            if(camMat.rows > 0) ofxCv::unwarpPerspective(camMat, roiMat, srcPts);
            
            ildaFrame.clear();
            ildaFrame.addPoly(testRect,ofFloatColor::white);
            ildaFrame.update();
            if(isActivated.get()) dac.setPoints(ildaFrame);
            
            if(!movingPoint) saveRoi("settingRoi.json");
            
        } break;
            
        case SEGMENTATION:
        {
            segmentator.slic(roiMat);
        } break;
            
        case RUN:
        {
            walker.update();
            ildaFrame.clear();
            ildaFrame.addPoly(walker.getPoly(), walker.lineColor);
            ildaFrame.update();
            // send points to the DAC
            if(isActivated.get()) dac.setPoints(ildaFrame);
            
            sync.update();

        } break;
            
        default:
            break;
    }
    
    
    
    
   
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofBackground(0);
    
    stringstream windowInfo;

    switch(state){

        case SELECT_ROI:
        {
            ofxCv::drawMat(camMat,0,0,CAM_PREVIEW_W,CAM_PREVIEW_H);
            ofxCv::drawMat(roiMat,CAM_PREVIEW_W,0,ROI_PREVIEW_W,ROI_PREVIEW_H);
            
            ofPushStyle();
            ofNoFill();
            ofSetColor(ofColor::orange);
            for(auto & p :srcPoints) {
                ofSetLineWidth(3);
                ofDrawCircle(p, 10);
                ofSetLineWidth(1);
                ofDrawCircle(p, 1);
            }
            ofPopStyle();
            
            guiLaser.draw();
            guiIPCam.draw();
            if(firstTime) setStatus(SEGMENTATION);
        } break;
            
        case SEGMENTATION:
        {
            segmentator.draw();
            guiSegmentation.draw();
            if(firstTime) setStatus(RUN);
        } break;
            
        case RUN:
        {

            int x = ofGetWidth()-ofGetHeight();
            int y = 0;
            int w = ofGetHeight();
            int h = ofGetHeight();
            walker.draw(x,y,w,h);
            ildaFrame.draw(x,y,w,h);
            
            ofPushMatrix();
            ofTranslate(x,y);
            ofScale(roiMat.rows/w, roiMat.cols/h);
            for(auto & sp : segmentator.superPixels){
                if(sp.isTopRow) sp.contour.draw();
            }
            ofPopMatrix();
            guiLaser.draw();
            gui.draw();
        } break;
            
        default:
            break;
    }
    
    windowInfo << currentState + " | FPS: "+ofToString(ofGetFrameRate());
    ofSetWindowTitle(windowInfo.str());
}

//--------------------------------------------------------------
void ofApp::exit(){
    ipCam.close();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if(key==OF_KEY_COMMAND){
        cmdDown = true;
    }
    if(key=='r'){
        walker.attractors.clear();
        for(int i = 0; i < 5; i++){
            Attractor attractor;
            walker.attractors.push_back(attractor);
            
        }
        walker.checkAttracktorsWithinBoarders();
        walker.target=walker.attractors[ofRandom(walker.attractors.size())].getPos();
    }
    if(key=='t'){
        walker.target=walker.attractors[ofRandom(walker.attractors.size())].getPos();
    }

    
    if(key=='x'){
        switch(state){
                
            case SELECT_ROI:
            {
                setStatus(SEGMENTATION);
            } break;
                
            case SEGMENTATION:
            {
                setStatus(RUN);
            } break;
                
            case RUN:
            {
                setStatus(SELECT_ROI);
            } break;
                
            default:
                break;
        }
    }
    
    if(key=='s'&& cmdDown ){
        switch(state){
                
            case SELECT_ROI:
            {
                guiLaser.saveToFile("settingsLaser_SelectROI.json");
            } break;
                
            case SEGMENTATION:
            {
                guiSegmentation.saveToFile("settingsSegmentation.json");
            } break;
                
            case RUN:
            {
                guiLaser.saveToFile("settingsLaser_Run.json");
                gui.saveToFile("settingsWalker.json");
            } break;
                
            default:
                break;
        }
    }
    
    if(key=='l'&& cmdDown ){
        switch(state){
                
            case SELECT_ROI:
            {
                guiLaser.loadFromFile("settingsLaser_SelectROI.json");
            } break;
                
            case SEGMENTATION:
            {
                guiSegmentation.loadFromFile("settingsSegmentation.json");
            } break;
                
            case RUN:
            {
                guiLaser.loadFromFile("settingsLaser_Run.json");
                gui.loadFromFile("settingsWalker.json");
            } break;
                
            default:
                break;
        }
    }
    

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    if(key==OF_KEY_COMMAND){
        cmdDown = false;
    }
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}


//--------------------------------------------------------------

void ofApp::mouseDragged(int x, int y, int button){
    if(state==SELECT_ROI){
        if(movingPoint) {
            curPoint->x = x;
            curPoint->y = y;
        }
    }
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    if(state==SELECT_ROI){
        for(auto & p : srcPoints){
            if(glm::distance(p,glm::vec2(x,y))<20){
                curPoint = &p;
                movingPoint = true;
            }
        }
    }
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
    movingPoint = false;
}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
//--------------------------------------------------------------
void ofApp::saveRoi(string filename){
    ofJson json;// = ofLoadJson(filename);
    for(int i = 0; i<srcPoints.size(); i++){
        json["srcPoints"][ofToString(i)] = vec2ToJson(srcPoints[i]);
    }
    
    ofSavePrettyJson(filename, json);
}

void ofApp::loadRoi(string filename){
ofFile jsonFile(filename);
ofJson json = ofLoadJson(jsonFile);
    if(!json["srcPoints"].is_null()) srcPoints.clear();
    for(auto & jPoint : json["srcPoints"]){
        srcPoints.push_back(jsonToVec2(jPoint));
    }
}



ofJson ofApp::vec3ToJson(glm::vec3 vec){
    ofJson json;
    json["x"] = vec.x;
    json["y"] = vec.y;
    json["z"] = vec.z;
    return json;
}

glm::vec3 ofApp::jsonToVec3(ofJson json){
    glm::vec3 vec;
    vec.x = json["x"].get<float>();
    vec.y = json["y"].get<float>();
    vec.z = json["z"].get<float>();
    return vec;
}

ofJson ofApp::vec2ToJson(glm::vec2 vec){
    ofJson json;
    json["x"] = vec.x;
    json["y"] = vec.y;
    return json;
}

glm::vec2 ofApp::jsonToVec2(ofJson json){
    glm::vec2 vec;
    vec.x = json["x"].get<float>();
    vec.y = json["y"].get<float>();
    return vec;
}
