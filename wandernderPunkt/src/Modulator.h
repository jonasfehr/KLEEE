//
//  Modulator.h
//  wandernderPunkt
//
//  Created by Jonas Fehr on 19/05/2018.
//

#ifndef Modulator_h
#define Modulator_h

class Modulator{
public:
    ofParameter<bool> doFollowDirection{"doFollowDirection", true};
    ofParameter<float> xAmplitude{"xAmplitude", 0.0, 0.0, .1};
    ofParameter<float> xSpeed{"xSpeed", 0.00, 0.0, 1.0};
    ofParameter<float> yAmplitude{"yAmplitude", 0.001, 0.0, .1};;
    ofParameter<float> ySpeed{"ySpeed", 0.001, 0.0, 1.0};;
    ofParameterGroup parameters{"Modulator", doFollowDirection, xAmplitude, xSpeed, yAmplitude, ySpeed};

    
    glm::vec3 pos;
    float angle;
    
    float xCounter, yCounter;
    
    
    Modulator(){
        pos = glm::vec3(0,0,0);
        xCounter = 0;
        yCounter = 0;
    }
    
    void setup(){
    }
    
    void update(){
        xCounter+=xSpeed;
        yCounter+=ySpeed;
        
        pos.x = glm::sin(xCounter)*xAmplitude;
        pos.y = glm::cos(yCounter)*yAmplitude;
    }
    
    glm::vec3 getModualtedPos(glm::vec3 p, glm::vec3 v){
        glm::vec3 directionVector = v;
        glm::normalize(directionVector);
        angle =  glm::orientedAngle(glm::normalize(v), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f,0.0f,1.0f));
        angle = -angle;
        if(doFollowDirection) return p + glm::rotate(pos, angle, glm::vec3(0.0f,0.0f,1.0f));
        else return p+pos;
    }
    
    void draw(int x, int y, int w, int h){
        ofPushMatrix();
        {
            ofTranslate(x,y);
            ofScale(w,h);
            if(doFollowDirection) ofRotate(ofRadToDeg(angle));
            ofNoFill();
            ofSetColor(ofColor::orange);
            ofDrawCircle(pos, 5./w);
            
            ofDrawLine(0,0,pos.x, 0);
            ofDrawLine(pos.x,0,pos.x, pos.y);
        }
        ofPopMatrix();
    }
};

#endif /* Modulator_h */
