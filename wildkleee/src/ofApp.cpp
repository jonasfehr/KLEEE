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
    parameters.add(font);
    parameters.add(projector->ildaFrame.params.output.masterColor);
    parameters.add(brownianRythm.parameters);
    parameters.add(laserFrame.parameters);
    parameters.add(projector->parameters);
    
    //    parameters.add(entry.parameters);
    font.addListener( this, &ofApp::fontChanged);
    
    sync.setup(parameters,8888,"localhost",8889);
    
    
    loadFromFile("settings.json");
    
    //    entry.setup(zones);
    laserFrame.setup(zones);
    
    int fontIndex = font.get();
    fontChanged(fontIndex);
    
}
void ofApp::fontChanged(int & i){
    string path = "fonts/" + fontNames[i] + "/";
    laserFrame.loadSvg(path);
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
}

void ofApp::guiSelectChanged(string & key){
    gui.clear();
    if(key == "Projector") {
        gui.add(projector->parameters);
    }
    if(key == "Runtime") {
        gui.add(parameters);
    }
    if(key == "Zones") {
        gui.add(parametersMapping);
        gui.add(projector->parameters);
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
    
    controlButtonsRect.set(guiRect.getLeft(), outputWindow.getTop()+outSide*2/3, 30, outSide*2/3);
    zoneButtonsRect.set(controlButtonsRect.getRight(), controlButtonsRect.getTop(), 200-30, guiRect.getHeight());
    
    
}


//--------------------------------------------------------------
void ofApp::update(){
    if(brownianRythm.update()){
        int index = zoneSelector.getIndex();
        if(index == -1) return;
        laserFrame.newFrame();
        
        // SEND New Word
        ofxOscMessage m;
        m.setAddress("/newWord");
        m.addIntArg(1);
        sync.sender.sendMessage(m, false);
    }
    
    projector->clearPoints();
    if(run){
        for(auto & pointGroup : laserFrame.getPointGroups()){
            projector->addPointGroup(pointGroup);
        }
        //        for(auto & pointGroup : entry_2.getPointGroups()){
        //            projector->addPointGroup(pointGroup);
        //        }
    }
    if(showMappingAid){
        projector->clearPoints();
        for(auto & pg : zones[zoneSelector.getIndex()]->mappingAid.getPointGroups()){
            projector->addPointGroup(pg);
        }
    }
    projector->update();
    
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
    ofBackground(50);
    
    // Draw in and output
    ofSetColor(0);
    ofDrawRectangle(outputWindow);
    
    ofSetColor(100,100,120);
    haus.draw(outputWindow.getTopLeft());
    
    // Control
    ofSetColor(255);
    mainPages.draw();
    
    string key = mainPages.getKey();
    if(key == "Runtime"){
        gui.draw();
    }else if(key == "Projector"){
        gui.draw();
    }else if(key == "Zones"){
        gui.draw();
        zoneSelector.draw();
        controlButtons.draw();
        for(auto & zone : zones){
            zone->drawOutput();
        }
    }
    
    projector->draw(outputWindow);
    
    
    
    
    
    
    // draw String
    ofSetColor(255);
    string ssStr = ss.str();
    ofDrawBitmapString(ss.str(), 30,30);
    
    ofSetColor(255);
    laserFrame.draw(outputWindow);
    
    stringstream windowInfo;
    //    windowInfo << " | DAC: " << dac.getDeviceID();
    //    windowInfo << " | Syphon (i): " << syphonIn.getName();
    //    windowInfo << " | numOfWalkers (up/down): " << walkers.size();
    windowInfo << " | FPS: " << fixed << setprecision(1) << ofGetFrameRate();
    //    windowInfo << " | " <<     ildaFrame.stats.pointCountInput << "/" <<     ildaFrame.stats.pointCountProcessed ;
    windowInfo << " | - ";
    windowInfo << laserFrame.getCombinationString();
    windowInfo << "- |";
    
    
    ofSetWindowTitle(windowInfo.str());
    
    
    
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
    ofJson js;// = ofLoadJson(filename);
    
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
    
    string guiKey = mainPages.getKey();
    guiSelectChanged(guiKey);
    
    
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

