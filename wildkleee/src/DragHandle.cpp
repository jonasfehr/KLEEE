//
//  DragHandle.cpp
//  mappingSetup
//
//  Created by Jonas Fehr on 14/06/2019.
//

#include "DragHandle.hpp"

DragHandle::DragHandle() {
    
};

DragHandle::~DragHandle() {
    if(state != DEACTIVATED) removeListeners();
};



void DragHandle::setup(vec2 position, ofRectangle *pWindow, float radius) {
    this->pViewWindow = pWindow;
    this->pos = position;//normalise(position);

    this->radius = radius;
};


void DragHandle::activate(){
    initListeners();
    state = ACTIVE;
}

void DragHandle::deactivate(){
    removeListeners();
    state = DEACTIVATED;

}

void DragHandle::draw() {
    
    
    switch(state){
        case DEACTIVATED:
        {
            ofPushStyle();
                ofSetColor(COLOR_ACTIVE);
                ofNoFill();
                ofDrawCircle(getWindowPos(),radius);
            ofPopStyle();
        }
            break;

        case ACTIVE:
        {
            ofPushStyle();
            ofSetColor(COLOR_ACTIVE);
            ofFill();
            ofDrawCircle(getWindowPos(),radius);
            ofPopStyle();
        }
            break;
        case HOVER:
        {
            ofPushStyle();
            ofSetColor(COLOR_ACTIVE);
            ofFill();
            ofDrawCircle(getWindowPos(),radius);
            ofSetColor(COLOR_SELECTED);
            ofNoFill();
            ofDrawCircle(getWindowPos(),radius);
            ofPopStyle();

        }
            break;
        case DRAG:
        case SELECTED:
        {
            ofPushStyle();
            ofSetColor(COLOR_SELECTED);
            ofFill();
            ofDrawCircle(getWindowPos(),radius);
            ofPopStyle();
        }
            break;
    }
};



bool DragHandle::hitTest(glm::vec2 hitpoint) {
    float dist = glm::distance(getWindowPos(), hitpoint);
    return (dist < radius);
}

glm::vec2 DragHandle::normalisedFromWindow(glm::vec2 position){
    return ( position - pViewWindow->getPosition() ) / glm::vec2(pViewWindow->getWidth(), pViewWindow->getHeight());
}

glm::vec2 DragHandle::getWindowPos(){
    return pos*glm::vec2(pViewWindow->getWidth(), pViewWindow->getHeight()) + pViewWindow->getTopLeft();
}

glm::vec2 & DragHandle::getPos(){
    return pos;
}


// --- interaciton
void DragHandle::initListeners(){
    ofAddListener(ofEvents().mousePressed, this, &DragHandle::mousePressed, OF_EVENT_ORDER_BEFORE_APP);
    ofAddListener(ofEvents().mouseReleased, this, &DragHandle::mouseReleased, OF_EVENT_ORDER_BEFORE_APP);
    ofAddListener(ofEvents().mouseDragged, this, &DragHandle::mouseDragged, OF_EVENT_ORDER_BEFORE_APP);
    ofAddListener(ofEvents().keyPressed, this, &DragHandle::keyPressed, OF_EVENT_ORDER_BEFORE_APP);

}

void DragHandle::removeListeners(){
    ofRemoveListener(ofEvents().mousePressed, this, &DragHandle::mousePressed, OF_EVENT_ORDER_BEFORE_APP);
    ofRemoveListener(ofEvents().mouseReleased, this, &DragHandle::mouseReleased, OF_EVENT_ORDER_BEFORE_APP);
    ofRemoveListener(ofEvents().mouseDragged, this, &DragHandle::mouseDragged, OF_EVENT_ORDER_BEFORE_APP);
    ofRemoveListener(ofEvents().keyPressed, this, &DragHandle::keyPressed, OF_EVENT_ORDER_BEFORE_APP);

    }

bool DragHandle::mousePressed(ofMouseEventArgs &e){
    
    glm::vec2 mousePoint = e;

    if(hitTest(mousePoint)){
        if(state != DRAG ){
            startDragPos = pos;
            clickOffset = normalisedFromWindow(mousePoint) - pos;
        }
        state = DRAG;

    }else{
        state = ACTIVE;
    }
}

bool DragHandle::mouseDragged(ofMouseEventArgs &e){
    glm::vec2 mousePoint = e;
    if(state == DRAG){
            glm::vec2 target = (normalisedFromWindow(mousePoint))  - clickOffset;
            pos = startDragPos + ((target-startDragPos) * (ofGetKeyPressed(OF_KEY_SHIFT)? 0.2 : 1));
        
        if(pos.x > 1.) pos.x = 1.;
        if(pos.x < 0.) pos.x = 0.;
        if(pos.y > 1.) pos.y = 1.;
        if(pos.y < 0.) pos.y = 0.;
        ofNotifyEvent(changedE, pos);
    }
}


bool DragHandle::mouseReleased(ofMouseEventArgs &e){
    glm::vec2 mousePoint = e;

//    if(hitTest(mousePoint)){
        if(state == DRAG) state = SELECTED;
//    }else{
//        state = ACTIVE;
//    }
    

}

bool DragHandle::keyPressed(ofKeyEventArgs &e){
    if(state == SELECTED){
        float jump = 0.01;
        if(e.key == OF_KEY_LEFT) pos.x -= jump * (ofGetKeyPressed(OF_KEY_SHIFT)? 0.1 : 1);
        if(e.key == OF_KEY_RIGHT) pos.x += jump * (ofGetKeyPressed(OF_KEY_SHIFT)? 0.1 : 1);
        if(e.key == OF_KEY_UP) pos.y -= jump * (ofGetKeyPressed(OF_KEY_SHIFT)? 0.1 : 1);
        if(e.key == OF_KEY_DOWN) pos.y += jump * (ofGetKeyPressed(OF_KEY_SHIFT)? 0.1 : 1);
        ofNotifyEvent(changedE, pos);
    }
    if(e.key == OF_KEY_SHIFT) startDragPos = pos;
}

