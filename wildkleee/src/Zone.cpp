//
//  Zone.cpp
//  mappingSetup
//
//  Created by Jonas Fehr on 14/06/2019.
//

#include "Zone.hpp"

Zone::Zone() {
    ofAddListener(parameters.parameterChangedE(), this,  &Zone::onParameterChange);
    ofAddListener(isActiveE, this,  &Zone::changeActive);

    if(isInputUnlocked) lockInput();
    if(isOutputUnlocked) lockOutput();
}

Zone::Zone(string name, ofRectangle *pInputWindow, ofRectangle *pOutputWindow){
    ofAddListener(parameters.parameterChangedE(), this,  &Zone::onParameterChange);
    ofAddListener(isActiveE, this,  &Zone::changeActive);

    if(isInputUnlocked) lockInput();
    if(isOutputUnlocked) lockOutput();
    
    setup(name, pInputWindow, pOutputWindow);
}


Zone::~Zone() {
    if(isInputUnlocked) lockInput();
    if(isOutputUnlocked) lockOutput();
    ofRemoveListener(parameters.parameterChangedE(), this,  &Zone::onParameterChange);
    ofRemoveListener(isActiveE, this,  &Zone::changeActive);

}

void Zone::changeActive(bool &b){
    if(b){
        unlockInput();
        unlockOutput();
    }else{
        lockInput();
        lockOutput();
    }
}

void Zone::setup(string name, ofRectangle *pInputWindow, ofRectangle *pOutputWindow){
    this->name = name;
    this->pInputWindow = pInputWindow;
    this->pOutputWindow = pOutputWindow;
    
    inputHandles[0].setup(glm::vec2(0., 0.), pInputWindow);
    inputHandles[1].setup(glm::vec2(1., 1.), pInputWindow);
    inputCenter.setup(glm::vec2(0.5, 0.5), pInputWindow);
    
    outputHandles[0].setup(glm::vec2(0.05, 0.05), pOutputWindow);
    outputHandles[1].setup(glm::vec2(0.95, 0.05), pOutputWindow);
    outputHandles[2].setup(glm::vec2(0.95, 0.95), pOutputWindow);
    outputHandles[3].setup(glm::vec2(0.05, 0.95), pOutputWindow);
    outputCenter.setup(glm::vec2(0.5, 0.5), pOutputWindow);

    iconCenter.load("icons/arrows.png");
    iconCenter.resize(24, 24);
    iconCenter.update();
    
    
    unlockInput();
    unlockOutput();
    
    updateMatrix();
    updateHandles();

    SelectableObjectBase::setup(name);
};

void Zone::onParameterChange(ofAbstractParameter& e){
    if( showMapping ){
        if(editInput != isInputUnlocked){
            if(editInput) unlockInput();
            else lockInput();
        }
        
        if(editOutput != isOutputUnlocked){
            if(editOutput) unlockOutput();
            else lockOutput();
        }
    } else {
        if(isInputUnlocked)    lockInput();
        if(isOutputUnlocked)   lockOutput();
    }
}

void Zone::inputCenterMove(glm::vec2 &p){
    glm::vec2 center;
    int i = 0;
    for(auto & inputHandle : inputHandles){
        center = center + inputHandle.getPos();
        i++;
    }
    center = center / i;
    glm::vec2 move = p-center;

    bool isOnEdgeX = false;
    bool isOnEdgeY = false;
    for(auto & inputHandle : inputHandles){
        glm::vec2 newPos = inputHandle.getPos() + move;
        if(newPos.x < 0 || newPos.x > 1.) isOnEdgeX = true;
        if(newPos.y < 0 || newPos.y > 1.) isOnEdgeY = true;
    }
    for(auto & inputHandle : inputHandles){
        glm::vec2 newPos = inputHandle.getPos() + move;
        if(newPos.x > 0 && newPos.x < 1. && !isOnEdgeX) inputHandle.getPos().x = newPos.x;
        if(newPos.y > 0 && newPos.y < 1. && !isOnEdgeY) inputHandle.getPos().y = newPos.y;

    }
    if(isOnEdgeX || isOnEdgeY) inputCenter.getPos() = center;
    
    updateMatrix();
}

void Zone::outputCenterMove(glm::vec2 &p){
    glm::vec2 center;
    int i = 0;
    for(auto & outputHandle : outputHandles){
        center = center + outputHandle.getPos();
        i++;
    }
    center = center / i;
    glm::vec2 move = p-center;
    for(auto & outputHandle : outputHandles){
        outputHandle.getPos() = outputHandle.getPos() + move;
    }
    updateMatrix();

}


// ----- UI -----

void Zone::unlockInput(){
    for(auto & iH : inputHandles){
        iH.activate();
    }
    inputCenter.activate();
    isInputUnlocked = true;
    ofAddListener(inputHandles[0].changedE, this,  &Zone::updateOnChange);
    ofAddListener(inputHandles[1].changedE, this,  &Zone::updateOnChange);
    ofAddListener(inputCenter.changedE, this,  &Zone::inputCenterMove);
}
void Zone::lockInput(){
    for(auto & iH : inputHandles){
        iH.deactivate();
    }
    inputCenter.deactivate();
    isInputUnlocked = false;

    ofRemoveListener(inputHandles[0].changedE, this,  &Zone::updateOnChange);
    ofRemoveListener(inputHandles[1].changedE, this,  &Zone::updateOnChange);
    ofRemoveListener(inputCenter.changedE, this,  &Zone::inputCenterMove);

}

void Zone::unlockOutput(){
    for(auto & oH : outputHandles){
        oH.activate();
    }
    outputCenter.activate();
    isOutputUnlocked = true;

    ofAddListener(outputHandles[0].changedE, this,  &Zone::updateOnChange);
    ofAddListener(outputHandles[1].changedE, this,  &Zone::updateOnChange);
    ofAddListener(outputHandles[2].changedE, this,  &Zone::updateOnChange);
    ofAddListener(outputHandles[3].changedE, this,  &Zone::updateOnChange);
    ofAddListener(outputCenter.changedE, this,  &Zone::outputCenterMove);
}

void Zone::lockOutput(){
    for(auto & oH : outputHandles){
        oH.deactivate();
    }
    outputCenter.deactivate();
    isOutputUnlocked = false;

    ofRemoveListener(outputHandles[0].changedE, this,  &Zone::updateOnChange);
    ofRemoveListener(outputHandles[1].changedE, this,  &Zone::updateOnChange);
    ofRemoveListener(outputHandles[2].changedE, this,  &Zone::updateOnChange);
    ofRemoveListener(outputHandles[3].changedE, this,  &Zone::updateOnChange);
    ofRemoveListener(outputCenter.changedE, this,  &Zone::outputCenterMove);

}

void Zone::draw(){
    SelectableObjectBase::draw();

    if(!showMapping) return;
    drawInput();
    drawOutput();


}

void Zone::drawInput(){
    if(!showMapping) return;
    ofPushStyle();
    {
        ofSetColor(100);
        ofNoFill();
        int w = inputHandles[1].getWindowPos().x - inputHandles[0].getWindowPos().x;
        int h = inputHandles[1].getWindowPos().y - inputHandles[0].getWindowPos().y;
        ofDrawRectangle(inputHandles[0].getWindowPos(),w,h);
        ofSetColor(ofColor::orange);
        ofDrawBitmapString(name, inputHandles[0].getWindowPos()+glm::vec2(5,-5));
        
        ofSetColor(255);
        iconCenter.draw(inputCenter.getWindowPos()-glm::vec2(iconCenter.getWidth()/2));
//        inputCenter.draw();
    }
    ofPopStyle();
    for(auto & iH : inputHandles)  iH.draw();
}
void Zone::drawOutput(){
    if(!showMapping) return;
    ofPushStyle();
    {
        ofSetColor(100);
        ofDrawLine(outputHandles[0].getWindowPos(),outputHandles[1].getWindowPos());
        ofDrawLine(outputHandles[1].getWindowPos(),outputHandles[2].getWindowPos());
        ofDrawLine(outputHandles[2].getWindowPos(),outputHandles[3].getWindowPos());
        ofDrawLine(outputHandles[3].getWindowPos(),outputHandles[0].getWindowPos());
        ofSetColor(ofColor::orange);
        ofDrawBitmapString(name, outputHandles[0].getWindowPos()+glm::vec2(5,-5));

        ofSetColor(255);
        iconCenter.draw(outputCenter.getWindowPos()-glm::vec2(iconCenter.getWidth()/2));
//        outputCenter.draw();
    }
    ofPopStyle();
    for(auto & oH : outputHandles) oH.draw();
}


float Zone::getNormLength(){
    float w = glm::distance(outputHandles[0].getPos(), outputHandles[3].getPos());
    float l = glm::distance(outputHandles[0].getPos(), outputHandles[1].getPos());
    return l/w;
}

// ----- MAPPING -----
void Zone::updateOnChange(glm::vec2 &p){
    updateHandles();
    updateMatrix();
}
void Zone::updateMatrix(){
    matrix = getMatrix(outputHandles[0].getPos(), outputHandles[1].getPos(), outputHandles[2].getPos(), outputHandles[3].getPos());
}

void Zone::updateHandles(){
    // REPLACE CENTER HANDLES
    glm::vec2 center = glm::vec2(0.);
    int i = 0;
    for(auto & outputHandle : outputHandles){
        center = center + outputHandle.getPos();
        i++;
    }
    center = center / i;
    outputCenter.getPos() = center;
    
    center = glm::vec2(0.);
    i = 0;
    for(auto & inputHandle : inputHandles){
        center = center + inputHandle.getPos();
        i++;
    }
    center = center / i;
    inputCenter.getPos() = center;
    
    //    inputRect.set(inputHandles[0].getPos(), inputHandles[1].getPos().x-inputHandles[0].getPos().x, inputHandles[1].getPos().y-inputHandles[0].getPos().y);
}
//
//
////--------------------------------------------------------------
//void Zone::update(const vector<ofPolyline> &origPolys, vector<ofPolyline> &mappedPolys) {
//    updateMatrix();
//
//    mappedPolys = origPolys;
//    for(int i=0; i<mappedPolys.size(); i++) {
//        for(int p = 0; p < mappedPolys[i].getVertices().size(); p++){
//            mappedPolys[i].getVertices()[p] = matrix.preMult((ofVec3f)mappedPolys[i].getVertices()[p]);
//        }
//    }
//
//}
//
//void Zone::remap(ofPolyline &toBeMappedPoly){
//    updateMatrix();
//
//    for(int p = 0; p < toBeMappedPoly.getVertices().size(); p++){
//        toBeMappedPoly.getVertices()[p] = matrix.preMult((ofVec3f)toBeMappedPoly.getVertices()[p]);
//    }
//}
//
//ofPolyline Zone::map(ofPolyline &originalPoly){
//    updateMatrix();
//
//    ofPolyline mappedPoly = originalPoly;
//    for(int p = 0; p < originalPoly.getVertices().size(); p++){
//        mappedPoly.getVertices()[p] = matrix.preMult((ofVec3f)mappedPoly.getVertices()[p]);
//    }
//    return mappedPoly;
//}
//
//ofxIlda::Poly Zone::map(ofxIlda::Poly &originalPoly){
//    updateMatrix();
//
//    ofxIlda::Poly mappedPoly = originalPoly;
//    for(int p = 0; p < originalPoly.getVertices().size(); p++){
//        mappedPoly.getVertices()[p] = matrix.preMult((ofVec3f)mappedPoly.getVertices()[p]);
//    }
//    return mappedPoly;
//}
//
//vector <ofPolyline> Zone::map(vector<ofPolyline> &originalPolylines) {
//    updateMatrix();
//
//    vector <ofPolyline> mappedPolys = originalPolylines;
//    for(int i=0; i<mappedPolys.size(); i++) {
//        for(int p = 0; p < mappedPolys[i].getVertices().size(); p++){
//            mappedPolys[i].getVertices()[p] = matrix.preMult((ofVec3f)mappedPolys[i].getVertices()[p]);
//        }
//    }
//
//    return mappedPolys;
//
//}
//vector<ofxIlda::Poly> Zone::map(vector<ofxIlda::Poly> &originalPolylines) {
//    updateMatrix();
//
//    vector <ofxIlda::Poly> mappedPolys = originalPolylines;
//    for(int i=0; i<mappedPolys.size(); i++) {
//        for(int p = 0; p < mappedPolys[i].getVertices().size(); p++){
//            mappedPolys[i].getVertices()[p] = matrix.preMult((ofVec3f)mappedPolys[i].getVertices()[p]);
//        }
//    }
//
//    return mappedPolys;
//
//}
//
////----------------------------------------------------- save / load.
////void Zone::save(const string& path) {
////    ofSavePrettyJson(path, getJson());
////}
//void Zone::setupFromJson(ofJson & json){
//
//    identifier = json["identifier"].get<std::string>();
//
//    int i = 0;
//    for( auto & src : json["src"]){
//        srcPoints[i].x = src["x"];
//        srcPoints[i].y = src["y"];
//        i++;
//    }
//    i = 0;
//    for( auto & dst : json["dst"]){
//        dstPoints[i].x = dst["x"];
//        dstPoints[i].y = dst["y"];
//        i++;
//    }
//
//    auto & jp = json["parameters"];
//    ofDeserialize(jp, parameters);
//}
//
//void Zone::addToJson(ofJson & json){
//    auto & j = json[identifier];
//    j["identifier"] = identifier;
//
//    for(int i=0; i<4; i++) {
//        j["src"]["point_"+ofToString(i)]["x"] = srcPoints[i].x;
//        j["src"]["point_"+ofToString(i)]["y"] = srcPoints[i].y;
//    }
//    for(int i=0; i<4; i++) {
//        j["dst"]["point_"+ofToString(i)]["x"] = dstPoints[i].x;
//        j["dst"]["point_"+ofToString(i)]["y"] = dstPoints[i].y;
//    }
//
//    auto & jp = j["parameters"];
//    ofSerialize(jp, parameters);
//}


glm::vec2 Zone::getScreenPosOutput(glm::vec2 &point){
    
    glm::vec3 p = calcPos(matrix, remapFromInput(point));
    return pOutputWindow->getPosition()+p*glm::vec2(pOutputWindow->getWidth(), pOutputWindow->getHeight());
}

glm::vec2 Zone::getScreenPosInput(glm::vec2 &point){
    return pInputWindow->getPosition()+point*glm::vec2(pInputWindow->getWidth(), pInputWindow->getHeight());
}


bool Zone::isInsideInput(glm::vec2 &point){
    bool x = point.x > inputHandles[0].getPos().x && point.x < inputHandles[1].getPos().x;
    bool y = point.y > inputHandles[0].getPos().y && point.y < inputHandles[1].getPos().y;
    return x&&y;
}

glm::vec2 Zone::mapToInput(glm::vec2 &point){
    glm::vec2 wSize = inputHandles[1].getPos() - inputHandles[0].getPos();
    return (point - inputHandles[0].getPos()) * glm::vec2(1./wSize.x, 1./wSize.y);
}

glm::vec2 Zone::remapFromInput(glm::vec2 &point){
    if(isInsideInput(point)) return mapToInput(point);
    else return glm::vec2(-0.1);
}

glm::vec3 Zone::map(glm::vec3 point){
    glm::vec3 p = calcPos(matrix, point);
    return p;
}

glm::vec2 Zone::map(glm::vec2 point){
    glm::vec3 p = calcPos(matrix, point);
    return glm::vec2(p.x, p.y);
}

vector<vector<ofxIlda::Point>> Zone::getMapped(vector<vector<ofxIlda::Point>> pointGroups){
    for(auto & pointGroup: pointGroups){
        for(auto & point: pointGroup){
            point = map(remapFromInput(point));
        }
    }
    return pointGroups;
}

glm::vec3 Zone::calcPos( const glm::mat4 &_mat, const glm::vec2 &v ) const {
    float d = 1.0f / (_mat[0][3] * v.x + _mat[1][3] * v.y + _mat[3][3]) ;
    return glm::vec3( (_mat[0][0]*v.x + _mat[1][0]*v.y + _mat[3][0])*d,
                     (_mat[0][1]*v.x + _mat[1][1]*v.y + _mat[3][1])*d,
                     (_mat[0][2]*v.x + _mat[1][2]*v.y + _mat[3][2])*d);
}

glm::vec3 Zone::calcPos( const glm::mat4 &_mat, const glm::vec3 &v ) const {
    float d = 1.0f / (_mat[0][3] * v.x + _mat[1][3] * v.y + _mat[2][3] * v.z + _mat[3][3]) ;
    return glm::vec3( (_mat[0][0]*v.x + _mat[1][0]*v.y + _mat[2][0]*v.z + _mat[3][0])*d,
                     (_mat[0][1]*v.x + _mat[1][1]*v.y + _mat[2][1]*v.z + _mat[3][1])*d,
                     (_mat[0][2]*v.x + _mat[1][2]*v.y + _mat[2][2]*v.z + _mat[3][2])*d);
}

// --- functions borrowed from ofxHomography ---
// https://github.com/paulobarcelos/ofxHomography
/*
 * Homography Functions adapted from:
 * http://www.openframeworks.cc/forum/viewtopic.php?p=22611
 * Author: arturo castro
 */
void Zone::gaussian_elimination(float *input, int n){
    // ported to c from pseudocode in
    // http://en.wikipedia.org/wiki/Gaussian_elimination
    
    float * A = input;
    int i = 0;
    int j = 0;
    int m = n-1;
    while (i < m && j < n){
        // Find pivot in column j, starting in row i:
        int maxi = i;
        for(int k = i+1; k<m; k++){
            if(fabs(A[k*n+j]) > fabs(A[maxi*n+j])){
                maxi = k;
            }
        }
        if (A[maxi*n+j] != 0){
            //swap rows i and maxi, but do not change the value of i
            if(i!=maxi)
                for(int k=0;k<n;k++){
                    float aux = A[i*n+k];
                    A[i*n+k]=A[maxi*n+k];
                    A[maxi*n+k]=aux;
                }
            //Now A[i,j] will contain the old value of A[maxi,j].
            //divide each entry in row i by A[i,j]
            float A_ij=A[i*n+j];
            for(int k=0;k<n;k++){
                A[i*n+k]/=A_ij;
            }
            //Now A[i,j] will have the value 1.
            for(int u = i+1; u< m; u++){
                //subtract A[u,j] * row i from row u
                float A_uj = A[u*n+j];
                for(int k=0;k<n;k++){
                    A[u*n+k]-=A_uj*A[i*n+k];
                }
                //Now A[u,j] will be 0, since A[u,j] - A[i,j] * A[u,j] = A[u,j] - 1 * A[u,j] = 0.
            }
            
            i++;
        }
        j++;
    }
    
    //back substitution
    for(int i=m-2;i>=0;i--){
        for(int j=i+1;j<n-1;j++){
            A[i*n+m]-=A[i*n+j]*A[j*n+m];
            //A[i*n+j]=0;
        }
    }
}

glm::mat4 Zone::getMatrix(glm::vec2 outputTL, glm::vec2 outputTR, glm::vec2 outputBR, glm::vec2 outputBL ){
    glm::vec2 dstPoints[4] =  {outputTL, outputTR, outputBR, outputBL};
    if(flipX){
        glm::vec2 dst = dstPoints[0];
        dstPoints[0] =  dstPoints[1];
        dstPoints[1] =  dst;
        dst = dstPoints[2];
        dstPoints[2] =  dstPoints[3];
        dstPoints[3] =  dst;
    }
    if(flipY){
        glm::vec2 dst = dstPoints[0];
        dstPoints[0] =  dstPoints[3];
        dstPoints[3] =  dst;
        dst = dstPoints[1];
        dstPoints[1] =  dstPoints[2];
        dstPoints[2] =  dst;
    }
    return getMatrix(&dstPoints[0]);
}

void Zone::serialize(ofJson & js){
    int i = 0;
    for(auto & inputHandle : inputHandles){
        js["Zone"]["Input"][ofToString(i)] = ofxJsonUtils::convert(inputHandle.getPos());
        i++;
    }
    
    i = 0;
    for(auto & outputHandle : outputHandles){
        js["Zone"]["Output"][ofToString(i)] = ofxJsonUtils::convert(outputHandle.getPos());
        i++;
    }
}

void Zone::deserialize(ofJson & js){
    int i = 0;
    for(auto & inputHandle : inputHandles){
        ofxJsonUtils::parse(js["Zone"]["Input"][ofToString(i)], inputHandle.getPos());
        i++;
    }
    i = 0;
    for(auto & outputHandle : outputHandles){
        ofxJsonUtils::parse(js["Zone"]["Output"][ofToString(i)], outputHandle.getPos());
        i++;
    }
    updateMatrix();
    updateHandles();
}


// slightly rewritten version of the function findHomography from ofxHomography
glm::mat4 Zone::getMatrix(const glm::vec2* dst){
    float homography[16];
    // create the equation system to be solved
    // src and dst must implement [] operator for point access
    //
    // from: Multiple View Geometry in Computer Vision 2ed
    //       Hartley R. and Zisserman A.
    //
    // x' = xH
    // where H is the homography: a 3 by 3 matrix
    // that transformed to inhomogeneous coordinates for each point
    // gives the following equations for each point:
    //
    // x' * (h31*x + h32*y + h33) = h11*x + h12*y + h13
    // y' * (h31*x + h32*y + h33) = h21*x + h22*y + h23
    //
    // as the homography is scale independent we can let h33 be 1 (indeed any of the terms)
    // so for 4 points we have 8 equations for 8 terms to solve: h11 - h32
    // after ordering the terms it gives the following matrix
    // that can be solved with gaussian elimination:
    
    float P[8][9]={
        {0, 0, -1,   0,   0,  0, 0, 0, -dst[0].x }, // h11
        {  0,   0,  0, 0, 0, -1, 0, 0, -dst[0].y }, // h12
        
        {-1, 0, -1,   0,   0,  0, dst[1].x, 0, -dst[1].x }, // h13
        {  0,   0,  0, -1, 0, -1, dst[1].y, 0, -dst[1].y }, // h21
        
        {-1, -1, -1,   0,   0,  0, dst[2].x, dst[2].x, -dst[2].x }, // h22
        {  0,   0,  0, -1, -1, -1, dst[2].y, dst[2].y, -dst[2].y }, // h23
        
        {0, -1, -1,   0,   0,  0, 0, dst[3].x, -dst[3].x }, // h31
        {  0,   0,  0, 0, -1, -1, 0, dst[3].y, -dst[3].y }, // h32
    };
    
    gaussian_elimination(&P[0][0],9);
    
    // gaussian elimination gives the results of the equation system
    // in the last column of the original matrix.
    // opengl needs the transposed 4x4 matrix:
    float aux_H[]={ P[0][8],P[3][8],0,P[6][8],    // h11  h21 0 h31
        P[1][8],P[4][8],0,P[7][8],    // h12  h22 0 h32
        0      ,      0,0,0,        // 0    0   0 0
        P[2][8],P[5][8],0,1};        // h13  h23 0 h33
    
    for(int i=0;i<16;i++) homography[i] = aux_H[i];
    
    
    return glm::mat4(
              homography[0], homography[1], homography[2], homography[3],
              homography[4], homography[5], homography[6], homography[7],
              homography[8], homography[9], homography[10], homography[11],
              homography[12], homography[13], homography[14], homography[15] );
}




//ofPoint Zone::toScreenCoordinates(ofPoint point, glm::mat4 homography){
//    ofVec4f original;
//    ofVec4f screen;
//
//    original.x = point.x;
//    original.y = point.y;
//    original.z = point.z;
//    original.w = 1.0;
//
//    glm::mat4 transposed = ofMatrix4x4::getTransposedOf(homography);
//
//    screen = transposed * original;
//
//    screen.x = screen.x / screen.w;
//    screen.y = screen.y / screen.w;
//    screen.z = screen.z / screen.w;
//
//    return ofPoint(screen.x,screen.y, screen.z);
//}
