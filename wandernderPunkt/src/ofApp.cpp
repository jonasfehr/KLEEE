#include "ofApp.h"

#define CAM_PREVIEW_W 1280
#define CAM_PREVIEW_H 720
#define ROI_PREVIEW_W 600
#define ROI_PREVIEW_H 600
//--------------------------------------------------------------
void ofApp::setup(){
    ofSetFrameRate(60);
    dac.setup(true, 0, true);
    dac.setPPS(LASER_PPSx1000*1000);
    ildaFrame.setup();
    
    // Guis
    state.set("state", UNDEFINED, SELECT_ROI, CALIB_POINTS);
    gui.setup();
    gui.add(state);
    gui.add(walker.parameters);
    gui.setPosition(220,10);

    guiIPCam.setup();
    guiIPCam.add(ipCam.parameters);
    guiIPCam.setPosition(220,10);
    
    guiLaser.setup();
    guiLaser.add(parameters);
    guiLaser.add(ildaFrame.parameters);
    
    guiSegmentation.setup();
    guiSegmentation.add(segmentator.parametersFilters);
    guiSegmentation.add(segmentator.parametersSLIC);
    
    guiCrossParam.setup();
    guiCrossParam.add(crossParam);
    guiCrossParam.setPosition(220,10);


    ofAddListener(parameters.parameterChangedE(), this, &ofApp::listenerFunction);
    
    unitedParameters.add(state);
    unitedParameters.add(walker.parameters);
    unitedParameters.add(crossParam);

//    unitedParameters.add(parameters);
//    unitedParameters.add(ildaFrame.parameters);
    
    sync.setup(unitedParameters,8888,"localhost",8889);


    createTestRect();
    
    roiMat.create(1024,1024,CV_8UC3);
    camMat = Mat::zeros(1920,1080,CV_8UC3);
    
    state.addListener(this, &ofApp::setStatus);
    int newState = RUN;
    setStatus(newState);

}

void ofApp::listenerFunction(ofAbstractParameter& e){
    dac.setPPS(LASER_PPSx1000*1000);
}

void ofApp::setStatus(int & newState){

    switch(newState){
            
        case SELECT_ROI:
        {
            guiLaser.loadFromFile("settingsLaser_SelectROI.json");
            isActivated = false;
            
            srcPoints.clear();
            srcPoints.push_back(glm::vec2(100,10));
            srcPoints.push_back(glm::vec2(730,10));
            srcPoints.push_back(glm::vec2(700,700));
            srcPoints.push_back(glm::vec2(80,710));
            loadPoints("settingRoi.json", "srcPoints", srcPoints);
            
            currentState = SELECT_ROI;
            currentStateStr = "Select ROI";
            ofSetFrameRate(30);
        } break;
            
        case SUPERPIXELS:
        {
            if(currentState == SELECT_ROI){
                // save images if previous SEL_ROI
                ofImage ofImg;
                toOf(camMat, ofImg);
                ofImg.save("captures/inputImages/"+ofGetTimestampString("%y%m%d_%H-%M-%S")+"_inputImage.jpg", OF_IMAGE_QUALITY_BEST);
                
                toOf(roiMat, ofImg);
                ofImg.save("captures/roi/"+ofGetTimestampString("%y%m%d_%H-%M-%S")+"_roi.jpg", OF_IMAGE_QUALITY_BEST);
            }

            // load images
            ofDirectory dir;
            dir.open("captures/roi");
            dir.listDir();
            dir.sortByDate();
            string filename = dir.getPath(dir.size()-1);

            ofImage loadImg;
            loadImg.load(filename);
            roiMat = toCv(loadImg).clone();

            guiSegmentation.loadFromFile("settingsSuperpixels.json");
            
            segmentator.doUpdate = true;
            
            currentState = SUPERPIXELS;
            currentStateStr = "Superpixels";
            ofSetFrameRate(30);
        } break;
            
        case RUN:
        {
            if(currentState == SUPERPIXELS){
                segmentator.groupSegmentSuperPixels();

                
                for (int i = 0; i<10; i++){
                    for(auto & sp: segmentator.superPixels){
                        if(sp.segment==i){
                            ofFloatImage sektionImg;
                            sektionImg.setFromPixels(sp.getBoundaryPixels(i==0?true:false));
                            sektionImg.update();
                            if(i == 0) sektionImg.save("masks/boundrys.jpg",OF_IMAGE_QUALITY_BEST);
                            else{
                               sektionImg.save("masks/sektion_"+ofToString(i)+".jpg",OF_IMAGE_QUALITY_BEST);
                                sp.setupContours();
                                walker.addZone(sp.contour.getPolyline(0));
                            }
                        }
                    }
                }
            }
            
            walker.setBoundaryPixels("masks/boundrys.jpg");

            if(currentState != CALIB_POINTS){
                camMat = ipCam.get();
                
                ofImage ofImg;
                toOf(camMat, ofImg);
                ofImg.save("captures/camOnRun/"+ofGetTimestampString("%y%m%d_%H-%M-%S")+"_inputImage.jpg", OF_IMAGE_QUALITY_BEST);
            }
            guiLaser.loadFromFile("settingsLaser_Run.json");
            if(currentState != RUN) gui.loadFromFile("settingsWalker.json");
            
            currentState = RUN;
            currentStateStr = "wandernder Punkt";
            ofSetFrameRate(60);
        } break;
        
        case CALIB_POINTS:
        {
            // load images
            ofDirectory dir;
            dir.open("captures/roi");
            dir.listDir();
            dir.sortByDate();
            string filename = dir.getPath(dir.size()-1);
            
            ofImage loadImg;
            loadImg.load(filename);
            roiMat = toCv(loadImg).clone();

            guiLaser.loadFromFile("settingsLaser_CalibPoints.json");
            isActivated = true;
            
            // Setup
            calibPoints.clear();
            calibPoints.push_back(glm::vec2(200,200));

            loadPoints("settingCalibPoints.json", "calibPoints", calibPoints);
            
            currentState = CALIB_POINTS;
            currentStateStr = "calibration points";
            ofSetFrameRate(30);
        } break;
            
        default:
            break;
    }
}

//--------------------------------------------------------------
void ofApp::update(){
    switch(currentState){

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
            
            if(!movingPoint) savePoints("settingRoi.json", "srcPoints", srcPoints);
        } break;
            
        case SUPERPIXELS:
        {
            segmentator.slic(roiMat);
            segmentator.manualSelect();
        } break;
            
        case RUN:
        {
            walker.update();
            ildaFrame.clear();
            ildaFrame.addPoly(walker.getPoly(), walker.lineColor);
            ildaFrame.update();
            // send points to the DAC
            if(isActivated.get()) dac.setPoints(ildaFrame);
        } break;
        
        case CALIB_POINTS:
        {
            vector<ofPolyline> cross;
            
            ofPolyline line;
            float squareSize = 0.1;
            for(auto & calibPoint : calibPoints){
                glm::vec2 cP_pts = glm::vec2(calibPoint.x/ROI_PREVIEW_W,calibPoint.y/ROI_PREVIEW_H);
                
                line.clear();
                line.addVertex(glm::vec3(cP_pts.x-squareSize, cP_pts.y, 0));
                line.addVertex(glm::vec3(cP_pts.x+squareSize, cP_pts.y, 0));
                cross.push_back(line);
                
                line.clear();
                line.addVertex(glm::vec3(cP_pts.x, cP_pts.y + squareSize, 0));
                line.addVertex(glm::vec3(cP_pts.x, cP_pts.y - squareSize, 0));
                cross.push_back(line);
            }

            
            ildaFrame.clear();
            ildaFrame.addPolys(cross,crossColor);
            ildaFrame.update();
            if(isActivated.get()) dac.setPoints(ildaFrame);
            
            if(!movingPoint) savePoints("settingCalibPoints.json", "calibPoints", calibPoints);
        } break;
            
        default:
            break;
    }
    
    sync.update();

    
    // SEND WATCHDOG
    if(ofGetFrameNum()%60==0){
        ofxOscMessage m;
        m.setAddress("/watchdog");
        m.addIntArg(1);
        sync.sender.sendMessage(m, false);
    }
    
    
   
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofBackground(0);
    
    stringstream windowInfo;

    switch(currentState){

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
        } break;
            
        case SUPERPIXELS:
        {
            segmentator.draw();
            guiSegmentation.draw();
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
                if(sp.segment==1) sp.contour.draw();
            }
            ofPopMatrix();
            guiLaser.draw();
            gui.draw();
        } break;
        
        case CALIB_POINTS:
        {
            ofxCv::drawMat(roiMat,0,0,ROI_PREVIEW_W,ROI_PREVIEW_H);
            
            ofPushStyle();
            ofNoFill();
            ofSetColor(ofColor::orange);
            for(auto & p :calibPoints) {
                ofSetLineWidth(3);
                ofDrawCircle(p, 10);
                ofSetLineWidth(1);
                ofDrawCircle(p, 1);
            }
            ofPopStyle();
            
            guiLaser.draw();
            guiCrossParam.draw();
        } break;
            
        default:
            break;
    }
    
    windowInfo << currentStateStr + " | FPS: "+ofToString(ofGetFrameRate());
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
        switch(currentState){
                
            case SELECT_ROI:
            {
                state = (SUPERPIXELS);
            } break;
                
            case SUPERPIXELS:
            {
                state = (RUN);
            } break;
                
            case RUN:
            {
                state = (SELECT_ROI);
            } break;
                
            case CALIB_POINTS:
            {
                state = (RUN);
            } break;
                
            default:
                break;
        }
    }
    
    if(key=='c'){
        state = (CALIB_POINTS);
    }
    
    if(key=='s'&& cmdDown ){
        switch(currentState){
                
            case SELECT_ROI:
            {
                guiLaser.saveToFile("settingsLaser_SelectROI.json");
            } break;
                
            case SUPERPIXELS:
            {
                guiSegmentation.saveToFile("settingsSegmentation.json");
            } break;
                
            case RUN:
            {
                guiLaser.saveToFile("settingsLaser_Run.json");
                gui.saveToFile("settingsWalker.json");
            } break;
            
            case CALIB_POINTS:
            {
                guiLaser.saveToFile("settingsLaser_CalibPoints.json");
            } break;
                
            default:
                break;
        }
    }
    
    if(key=='l'&& cmdDown ){
        switch(currentState){
                
            case SELECT_ROI:
            {
                guiLaser.loadFromFile("settingsLaser_SelectROI.json");
            } break;
                
            case SUPERPIXELS:
            {
                guiSegmentation.loadFromFile("settingsSegmentation.json");
            } break;
                
            case RUN:
            {
                guiLaser.loadFromFile("settingsLaser_Run.json");
                gui.loadFromFile("settingsWalker.json");
            } break;
            
            case CALIB_POINTS:
            {
                guiLaser.loadFromFile("settingsLaser_CalibPoints.json");
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
    if(state==SELECT_ROI || state==CALIB_POINTS){
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
    else if(state==CALIB_POINTS){
        for(auto & p : calibPoints){
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
void ofApp::savePoints(string filename, string pointName, vector<glm::vec2> & points){
    ofJson json;// = ofLoadJson(filename);
    for(int i = 0; i<points.size(); i++){
        json[pointName][ofToString(i)] = vec2ToJson(points[i]);
    }
    
    ofSavePrettyJson(filename, json);
}

void ofApp::loadPoints(string filename, string pointName, vector<glm::vec2> & points){
ofFile jsonFile(filename);
ofJson json = ofLoadJson(jsonFile);
    if(!json[pointName].is_null()) points.clear();
    for(auto & jPoint : json[pointName]){
        points.push_back(jsonToVec2(jPoint));
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
