//
//  UI_DragablePoint.h
//  mappingSetup
//
//  Created by Jonas Fehr on 14/06/2019.
//  Original from https://github.com/sebleedelisle/ofxLaser -> DragHandle
//

#ifndef DragHandle_hpp
#define DragHandle_hpp

#include <stdio.h>

#include "ofMain.h"
using namespace glm;


#define COLOR_BORDER  ofColor::lightGray
#define COLOR_SELECTED ofColor::orange
#define COLOR_ACTIVE ofColor::skyBlue

#define COLOR_ID_STRING ofColor::skyBlue

class DragHandle{
    
public:
    
    DragHandle();
    ~DragHandle();

    void setup(vec2 position, ofRectangle *pWindow, float radius = 5);
    
    void activate();
    void deactivate();
    
    void draw();
    
    
    glm::vec2 normalisedFromWindow(glm::vec2 position);
    glm::vec2 getWindowPos();
    glm::vec2 & getPos();
    
    ofEvent<glm::vec2> changedE;

    
private:
    glm::vec2 pos;
    float radius = 5;
    glm::vec2 startDragPos;
    glm::vec2 clickOffset;
    ofRectangle *pViewWindow;
    
    
    // --- interaciton
    void initListeners();
    void removeListeners();
    
    bool mousePressed(ofMouseEventArgs &e);
    bool mouseDragged(ofMouseEventArgs &e);
    bool mouseReleased(ofMouseEventArgs &e);
    bool keyPressed(ofKeyEventArgs &e);
    
    bool hitTest(glm::vec2 hitpoint);

    
    enum States{
        DEACTIVATED,
        ACTIVE,
        DRAG,
        SELECTED,
        HOVER
    } state;

};

#endif /* DragHandle_hpp */
