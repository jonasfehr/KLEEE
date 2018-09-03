//
//  Attractor.h
//  wandernderPunkt
//
//  Created by Jonas Fehr on 03/09/2018.
//

#ifndef Attractor_h
#define Attractor_h

#include "ofMain.h"

class Attractor{
public:
    Attractor(){
        pos = glm::vec3(ofRandom(80)/100.0+0.1, ofRandom(80)/100.0+0.1, 0);
        mass = 0.000001;
        G = 1;
        bActive = true;
        vel = glm::normalize(pos-glm::vec3(0.5,0.5,0));
    }
    
    glm::vec3 getAttraction(glm::vec3 pos){
        glm::vec3 force = this->pos - pos;
        float distance = force.length();
        
        
        distance = glm::clamp(distance,0.01f,0.4f);
        
        
        force = glm::normalize(force);
        float strength = (G * mass) / (distance * distance);
        force = force * strength;
        return force;
    }
    
    bool isActive(){ return bActive;}
    void deactivate(){ bActive = false;}
    void activate(){ bActive = true;}
    void setPos(glm::vec3 pos ){ this->pos = pos;}
    glm::vec3 getPos(){ return pos;}
    
    
    glm::vec3 vel;
    
private:
    bool bActive;
    glm::vec3 pos;
    float mass;
    float G;
    
};

#endif /* Attractor_h */
