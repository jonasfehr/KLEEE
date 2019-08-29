//
//  Zone.hpp
//  mappingSetup
//
//  Created by Jonas Fehr on 14/06/2019.
//

#ifndef Zone_hpp
#define Zone_hpp

#include <stdio.h>

#include "DragHandle.hpp"
#include "ofxJsonUtils.h"
#include "ofxIldaPoint.h"
#include "ofxGui.h"
#include "SelectableObjectBase.hpp"
#include "SelectableObjectBase.hpp"
#include "ContentBase.hpp"

class Zone :public SelectableObjectBase{
public:
    // ---- GUI ----
   
    
    ofParameter<bool> showMapping{"showMapping", true};
    ofParameter<bool> editInput{"editInput", true};
    ofParameter<bool> editOutput{"editOutput", true};
    ofParameterGroup parameters{"Mapping", showMapping, editInput, editOutput};
    
    Zone();
    Zone(string name, ofRectangle *pInputWindow, ofRectangle *pOutputWindow);
    ~Zone();

    
    bool isInputUnlocked;
    bool isOutputUnlocked;

    void setup(string name, ofRectangle *pInputWindow, ofRectangle *pOutputWindow);
    
    void onParameterChange(ofAbstractParameter& e);
    void unlockInput();
    void lockInput();
    void unlockOutput();
    void lockOutput();
    
    void draw();
    void drawInput();
    void drawOutput();
    void drawButton();

    void updateMatrix();
    void updateOnChange(glm::vec2 &p);
    void updateHandles();
    
    void changeActive(bool &b);
    
    float getNormLength();

    void update(const vector<ofPolyline> &origPolys, vector<ofPolyline> &mappedPolys);
//    ofPolyline map(ofPolyline &originalPoly);
//    ofxIlda::Poly map(ofxIlda::Poly &originalPoly);
//    vector <ofPolyline> map(vector <ofPolyline> &originalPolylines);
//    vector <ofxIlda::Poly> map(vector <ofxIlda::Poly> &originalPolylines);
//    
//    void remap(ofPolyline &toBeMappedPoly);
    
    bool isInsideInput(glm::vec2 &point);
    glm::vec2 map(glm::vec2 point);
    glm::vec3 map(glm::vec3 point);
    vector<vector<ofxIlda::Point>> getMapped(vector<vector<ofxIlda::Point>> pointGroups);
    glm::vec2 mapToInput(glm::vec2 &point);
    glm::vec2 remapFromInput(glm::vec2 &point);
    glm::vec2 getScreenPosOutput(glm::vec2 &point);
    glm::vec2 getScreenPosInput(glm::vec2 &point);
    glm::vec3 calcPos( const glm::mat4 &_mat, const glm::vec2 &v ) const;
    glm::vec3 calcPos( const glm::mat4 &_mat, const glm::vec3 &v ) const;
    
    
    
    void serialize(ofJson & js);
    void deserialize(ofJson & js);

    glm::mat4 matrix;
    
    void inputCenterMove(glm::vec2 &p);
    void outputCenterMove(glm::vec2 &p);
    
    glm::vec2 getInputCenter(){return inputCenter.getPos();}
    
    ofRectangle & getOutputRectangle(){ return *pOutputWindow; }
    ofRectangle & getInputRectangle(){ return *pInputWindow; }
    
    void setFlipX(bool flip){
        this->flipX = flip;
        updateMatrix();
    }
    void setFlipY(bool flip){
        this->flipY = flip;
        updateMatrix();
    }
    void setFlip(ofParameter<bool> & flipX, ofParameter<bool> & flipY){
        this->flipX.makeReferenceTo(flipX);
        this->flipY.makeReferenceTo(flipY);
    }
    
    void createMappingAid();

    
    ContentBase mappingAid;
private:
    ofParameter<bool> flipX{"flipX", false};
    ofParameter<bool> flipY{"flipY", false};

    
    DragHandle inputHandles[2];
    DragHandle outputHandles[4];
    DragHandle inputCenter, outputCenter;
//    ofRectangle inputRect;
    ofRectangle *pInputWindow;
    ofRectangle *pOutputWindow;
    
    void gaussian_elimination(float *input, int n);
    glm::mat4 getMatrix(glm::vec2 outputTL, glm::vec2 outputTR, glm::vec2 outputBR, glm::vec2 outputBL );
    glm::mat4 getMatrix(const glm::vec2* dst);
    ofPoint toScreenCoordinates(ofPoint point, ofMatrix4x4 homography);
    
    ofMatrix4x4 test;
    ofImage iconCenter;
    
    string name;
};

#endif /* Zone_hpp */
