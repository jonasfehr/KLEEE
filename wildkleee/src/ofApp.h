#pragma once

#include "ofMain.h"
#include "I_want_to_say_something.h"
#include "BrownianRythm.h"
#include "Zone.hpp"
#include "ofxSelectableObjects.hpp"
#include "Entry.h"
#include "LaserProjector_new.hpp"
#include "ofxRayComposer.h"
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
    
    void onResize();
    void fontChanged(int & i);
    void zoneControlChanged(string & key);
    void guiSelectChanged(string & key);

    void saveToFile(const std::string& filename);
    void loadFromFile(const std::string& filename);

    
    I_want_to_say_something iwtss;
    BrownianRythm brownianRythm;
    
    vector<shared_ptr<Zone>> zones;

    ofRectangle mainPagesRect;

    ofRectangle inputWindow;
    ofRectangle outputWindow;
    ofRectangle guiRect;
    ofRectangle controlButtonsRect;
    ofRectangle zoneButtonsRect;

    ofxSelectableObjects controlButtons;
    ofxSelectableObjects zoneSelector;
    
    ofxSelectableObjects mainPages;


    stringstream ss;
    
    Entry entry;
    
    ofxRayComposer::Handler rcHandler;
    unique_ptr<LaserProjector_new> projector;
    
    
    ofxPanel gui;

    ofImage haus;
    
    ofParameter<bool> run{"run", false};
    ofParameter<int> font{"font", 1, 0, 4};

    ofParameterGroup parameters;

    ofParameter<bool> showMappingAid{"showMappingAid", false};
    ofParameterGroup parametersMapping{"Mapping", showMappingAid};
    
    string fontNames[5] = {"debug", "martina", "iwantto", "iwantto_2", "costaparadiso"};

};
