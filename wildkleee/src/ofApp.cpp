#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    brownianRythm.setup(100, 1000, 4, 0.2);
    ofSetFrameRate(60);
    ss << " ";
    
    onResize();
    
    
    controlButtons.setup(controlButtonsRect, ST_PRESSED, true, 30-10 );
    controlButtons.add(*new SelectableObjectBase("plus", "icons/plus.png"));
    controlButtons.add(*new SelectableObjectBase("minus", "icons/minus.png"));
    ofAddListener(controlButtons.keyActivatedE, this, &ofApp::zoneControlChanged);
    
    zoneSelector.setup(zoneButtonsRect, ST_RADIO, true);
//    ofAddListener(zoneSelector.keyActivatedE, this, &ofApp::zoneControlChanged);
    
    rcHandler.setup();
    
    vector<string> deviceIDs = rcHandler.getListOfDeviceIDs();

    if(deviceIDs.size()==0) deviceIDs.push_back("notSet");
    projector = make_unique<LaserProjector_new>("Projector", deviceIDs[0], rcHandler);
    mainPages.setup(mainPagesRect, ST_RADIO, false, 150 );
    mainPages.add(*new SelectableObjectBase("Runtime"));
    mainPages.add(*projector);
    mainPages.add(*new SelectableObjectBase("Zones"));
    mainPages.select("Runtime");

    ofAddListener(mainPages.keyActivatedE, this, &ofApp::guiSelectChanged);

    gui.setup("GUI", "settings.json", guiRect.getLeft(), guiRect.getTop());

    haus.load("haus.png");
    haus.resize(outputWindow.getWidth(),outputWindow.getHeight());
    haus.update();
    
    parameters.setName("Runtime");
    parameters.add(run);
    parameters.add(projector->ildaFrame.params.output.masterColor);
    parameters.add(brownianRythm.parameters);
    parameters.add(entry.parameters);

    loadFromFile("settings.json");
    
    entry.setup(zones);

}

void ofApp::zoneControlChanged(string & key){
    if(key == "plus") {
        zones.push_back(make_unique<Zone>("Zone_"+ofToString(zoneSelector.size()), &inputWindow, &outputWindow));
        zoneSelector.add(*zones.back());
    }
    if(key == "minus") {
        int index = zoneSelector.getIndex();
        if( index == -1 ) return;
        zones.erase (zones.begin()+index);
        zoneSelector.deleteAtIndex(index);
    }
    
    if(key == "Projector"){
        
    }
    
//    int index =  zoneSelector.getIndexFromKey(key);
//    cout << zones[index]->getNormLength() << endl;
}

void ofApp::guiSelectChanged(string & key){
    gui.clear();
    if(key == "Projector") {
        gui.add(projector->parameters);
    }
    if(key == "Runtime") {
        gui.add(parameters);
    }
    
    //    int index =  zoneSelector.getIndexFromKey(key);
    //    cout << zones[index]->getNormLength() << endl;
}

void ofApp::onResize(){
    
    mainPagesRect.set(10, 10, ofGetWidth(), 30);
    int outSide = ofGetHeight()-30-mainPagesRect.getHeight();
    int controlWidth = ofGetWidth()-outSide;

    outputWindow.set(ofGetWidth()-outSide - 15, mainPagesRect.getHeight()+15, outSide, outSide);
    guiRect.set(15,mainPagesRect.getBottom()+10, controlWidth-10, ofGetHeight()/2-mainPagesRect.getHeight());

    controlButtonsRect.set(guiRect);
    controlButtonsRect.setWidth(30);
    zoneButtonsRect.set(controlButtonsRect.getRight(), guiRect.getTop(), 200-30, guiRect.getHeight());


}


//--------------------------------------------------------------
void ofApp::update(){
    if(brownianRythm.update()){
        int index = zoneSelector.getIndex();
        if(index == -1) return;
        //        ss << iwtss.getNextString() << " ";
        entry.next();
    }
    
    projector->clearPoints();
    if(run){
        for(auto & pointGroup : entry.getPointGroups()){
            projector->addPointGroup(pointGroup);
        }
    }
    projector->update();
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofBackground(50);

    // Draw in and output
    ofSetColor(0);
    ofDrawRectangle(outputWindow);
    
    ofSetColor(100,100,120);
    haus.draw(outputWindow.getTopLeft());

    // Control
    ofSetColor(255);
    mainPages.draw();
    projector->draw(outputWindow);

    string key = mainPages.getKey();
    if(key == "Runtime"){
        gui.draw();
    }else if(key == "Projector"){
        gui.draw();
    }else if(key == "Zones"){
        zoneSelector.draw();
        controlButtons.draw();
        for(auto & zone : zones){
            zone->drawOutput();
        }
    }




    ofSetColor(255);
//    projector->drawPoints(outputWindow);
    
    


    // draw String
    ofSetColor(255);
    string ssStr = ss.str();
    ofDrawBitmapString(ss.str(), 30,30);
    
    ofSetColor(255);
    entry.draw(outputWindow);

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if(key == 's'){
        saveToFile("settings.json");
    }
    if(key == 'l'){
        loadFromFile("settings.json");
    }

}

void ofApp::saveToFile(const std::string& filename){
    ofJson js = ofLoadJson(filename);
    for(auto & zone : zones){
        zone->serialize(js["Zones"][zone->getKey()]);
    }
    projector->serialize(js["Projector"]);
    ofSerialize(js["Parameters"], parameters);

    ofSavePrettyJson(filename, js);
    
}

void ofApp::loadFromFile(const std::string& filename){
    ofFile jsonFile(filename);
    ofJson js = ofLoadJson(jsonFile);
    zones.clear();
    zoneSelector.clear();
    for (auto& [key, zoneJs] : js["Zones"].items()) {
        zones.push_back(make_unique<Zone>(key, &inputWindow, &outputWindow));
        zoneSelector.add(*zones.back());
        zones.back()->deserialize(zoneJs);

    }
    projector->deserialize(js["Projector"]);
    ofDeserialize(js["Parameters"], parameters);



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
    onResize();
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
