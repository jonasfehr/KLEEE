//
//  Walker.hpp
//  wandernderPunkt
//
//  Created by Jonas Fehr on 03/09/2018.
//

#ifndef Walker_hpp
#define Walker_hpp

#include "Modulator.h"
#include "ofMain.h"
#include "ofxGui.h"

#include "Attractor.h"
#include "RandomWithoutRepeate.h"

enum MovementPatterns{
    STEERING,
    ATTRACTIONS,
    ZONEBOUNCE,
    ZONEZIGZACK,
    ZONESPIRAL,
    MOUSEFOLLOW,
    NOISE,
    RND,
    SINUS
};



class Walker{
public:
    ofParameter<int> movementPattern{"MovementPattern", STEERING, 0, SINUS};
    ofParameter<float> scaler{"Scaler", 1.0f, 0.0f, 1.0f};
    ofParameter<float> speed{"Speed", 0.001f, 0.0f, 0.01f};
    ofParameter<float> maxForce{"maxForce", 0.001f, 0.0f, 0.01f};
    ofParameter<float> maxRepulsion{"maxRepulsion", 0.001f, 0.0f, 0.01f};
    ofParameter<float> maxRepulsionSpeed{"maxRepulsionSpeed", 0.001f, 0.0f, 0.01f};
    ofParameter<float> minDistancePred{"minDistancePred", 0.1f, 0.0f, 1.0f};
    ofParameter<int> length{"Length", 200, 1, 1000};
    ofParameter<bool> doModulate{"doModulate", true};
    ofParameter<float> attractorMove{"attractorMove", 0.0001f, 0.0f, 0.001f};
    ofParameter<ofFloatColor> lineColor{ "lineColor", ofFloatColor::cyan };
    ofParameterGroup parameters{"Walker", scaler, speed, maxForce, maxRepulsion, maxRepulsionSpeed, minDistancePred, length, doModulate,attractorMove,lineColor};
    
    
    glm::vec3 pos;
    glm::vec3 vel;
    glm::vec3 target;
    
    RandomWithoutRepeate random;
    
    vector<Attractor> attractors;
    int activeAttractor;
    
    vector<glm::vec3> history;
    
    ofPolyline zone;
    ofPolyline poly;
    
    ofFloatPixels boundaryPixels;
    ofImage boundaryImage;
    bool hasBoundaryPixels = false;
    
    
    void setBoundaryPixels(ofFloatPixels boundaryPixels);
    
    void checkAttracktorsWithinBoarders();
    
    
    // for ZigZag
    bool firstTime;
    int targetIndex = 0;
    int offset;
    
    // for spiral
    float splineIndex = 0;
    float dist = 0;
    
    Modulator modulator;
    
    Walker();
    
    void addZone(ofPolyline &zone);
    
    void seek(glm::vec3 target);
    
    void avoid(glm::vec3 target, float minDistance);
    
    bool bBorders = false;
    void update();
    
    glm::vec3 normVecToTarget(glm::vec3 t);
    
    ofPolyline & getPoly();
    
    void draw(int x, int y, int w, int h);
};
#endif /* Walker_hpp */
