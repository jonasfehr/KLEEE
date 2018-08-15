#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    
    ofSetFrameRate(60);
    
//    syphonIn.setup("Normal", "MadMapper");
    
    walker.setup();
    
 
    
    blobFinder.setup();
    blobFinder.update();
    walker.addZone(blobFinder.getZone(0));
    
//    int x = ofGetWidth()-ofGetHeight();
//    int y = 0;
//    int w = ofGetHeight();
//    int h = ofGetHeight();
//    laserProjector.setup("mainLaser", 0, x, y, w, h);

    dac.setup(true, 0);
    dac.setPPS(LASER_PPSx1000*1000);
    
    
    ildaFrame.setup();
    
//    // Gui
//    this->gui.setup();
//    this->guiVisible = true;
    
    gui.setup();
    gui.add(parameters);
    gui.add(walker.parameters);
    gui.add(blobFinder.parameters);
    gui.add(ildaFrame.parameters);
    
    ofAddListener(parameters.parameterChangedE(), this, &ofApp::listenerFunction);

}

void ofApp::listenerFunction(ofAbstractParameter& e){
    dac.setPPS(LASER_PPSx1000*1000);
}

//--------------------------------------------------------------
void ofApp::update(){
    
    
    
    
    walker.update();
    ildaFrame.clear();
    ildaFrame.addPoly(walker.getPoly(), walker.lineColor);
    ildaFrame.update();
    // send points to the DAC
    if(isActivated.get()) dac.setPoints(ildaFrame);

}

//--------------------------------------------------------------
void ofApp::draw(){
    ofBackground(0);
    
    int x = ofGetWidth()-ofGetHeight();
    int y = 0;
    int w = ofGetHeight();
    int h = ofGetHeight();

    blobFinder.draw(x,y,w,h);
    
    walker.draw(x,y,w,h);
    
    ildaFrame.draw(x,y,w,h);

//    laserProjector.draw(x,y,w,h);

    // Gui
//    this->mouseOverGui = false;
//    if (this->guiVisible)
//    {
//        this->mouseOverGui = this->imGui();
//    }
//    if (this->mouseOverGui)
//    {
////        this->camera.disableMouseInput();
//    }
//    else
//    {
////        this->camera.enableMouseInput();
//    }
    
    
    gui.draw();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if(key=='r'){
        walker.attractors.clear();
        for(int i = 0; i < 5; i++){
            Attractor attractor;
            walker.attractors.push_back(attractor);
        }
        walker.target=walker.attractors[ofRandom(walker.attractors.size())].getPos();
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

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
//bool ofApp::imGui()
//{
//    auto mainSettings = ofxImGui::Settings();
//
//    this->gui.begin();
//    {
//        if (ofxImGui::BeginWindow("VisualControl", mainSettings, false))
//        {
//            ImGui::Text("%.1f FPS (%.3f ms/frame)", ofGetFrameRate(), 1000.0f / ImGui::GetIO().Framerate);
//
////            if (ofxImGui::BeginTree(this->walker.parameters, mainSettings))
////            {
//                ofxImGui::AddGroup(this->walker.parameters, mainSettings);
//            ofxImGui::AddGroup(this->blobFinder.parameters, mainSettings);
//            ofxImGui::AddGroup(this->laserProjector.parameters, mainSettings);
//
////                ofxImGui::EndTree(mainSettings);
////            }
//
//
//        }
//        ofxImGui::EndWindow(mainSettings);
//    }
//    this->gui.end();
//
//    return mainSettings.mouseOverGui;
//}
