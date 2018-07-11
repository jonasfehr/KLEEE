//
//  Walker.h
//  wandernderPunkt
//
//  Created by Jonas Fehr on 19/05/2018.
//

#ifndef Walker_h
#define Walker_h

#include "Modulator.h"

enum MovementPatterns{
    FORCES,
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
    ofParameter<int> movementPattern{"MovementPattern", FORCES, 0, SINUS};
    ofParameter<float> speed{"Speed", 0.001f, 0.0f, 0.01f};
    ofParameter<int> length{"Length", 200, 1, 1000};
    ofParameter<bool> doModulate{"doModulate", true};
    ofParameter<ofFloatColor> lineColor{ "lineColor", ofFloatColor::cyan };

    ofParameterGroup parameters{"Walker", movementPattern, speed, length, doModulate,lineColor};


    glm::vec3 pos;
    glm::vec3 vel;
    glm::vec3 target;
    
    
    int prevMP = movementPattern.get();

    vector<glm::vec3> history;
    
    ofPolyline zone;
    ofPolyline poly;

    
    // for ZigZag
    bool firstTime;
    int targetIndex = 0;
    int offset;
    
    // for spiral
    float splineIndex = 0;
    float dist = 0;

    Modulator modulator;
    
    Walker(){
        modulator.setup();
        parameters.add(modulator.parameters);
        
        // startPos
        pos = glm::vec3(0.5f, 0.5f, 0.0f);
        vel = glm::vec3(0.0f, 0.0f, 0.0f);
    }
    
    void setup(){



    }
    
    void addZone(ofPolyline &zone){
        this->zone = zone;
    }
    
    void update(){
        // Noise based Movement
        
        switch(movementPattern){
            case FORCES:
            {
                target = zone.getCentroid2D();
                vel = normVecToTarget(target);

                vel = glm::normalize(vel);
            }
                break;
                
            case ZONEBOUNCE:
            {
                if(!zone.inside(pos)){
                    unsigned int closestPointIndex;
                    target = zone.getClosestPoint(pos, &closestPointIndex);
                    
                    if(glm::distance(pos, target) < 2*speed){
                        glm::vec3 norm = zone.getNormalAtIndex(closestPointIndex);
                        vel = vel - 2 * glm::dot(vel, norm) * norm;
                    } else{
                        vel = normVecToTarget(target);
                        
                    }
                    
                }
                    vel = glm::normalize(vel);
            }
                break;
                
            case ZONEZIGZACK:
            {
                if(zone.inside(pos)){
                    if(firstTime){
                        unsigned int closestPoint;
                        zone.getClosestPoint(pos, &closestPoint);
                        offset = closestPoint;
                        firstTime = false;
                    }
                    
                    if(glm::distance(pos, target) < 0.01){
                        targetIndex+= 15;
                        int index;
                        if(targetIndex%2 == 0)  index = zone.size()-targetIndex/2;
                        else index = targetIndex/2;
                        index += offset;
                        index = index%zone.size();
                        target = zone[index];
//                        target = zone.getPointAtPercent(ofRandom(100)/100.0);
                    }

                }else{
                    target = zone.getClosestPoint(pos);//zone.getBoundingBox().getCenter();
                    firstTime = true;
            
                    
                }
                vel = normVecToTarget(target);

            }
                break;
                
            case ZONESPIRAL:
            {
                if(zone.inside(pos)){
                    unsigned int closestPointIndex;
                    glm::vec3 closestPoint = zone.getClosestPoint(pos, &closestPointIndex);
                    
                    if(firstTime){
                        offset = zone.getLengthAtIndex(closestPointIndex);
                        splineIndex = 0;
                        dist = 0.01;
                        firstTime = false;
                    }
                    
                    dist += speed/100;
                    splineIndex += speed;
                    float length = splineIndex;
                    length += offset;

                    length = glm::mod<float>(length, zone.getLengthAtIndex(zone.size()-1));
                    float index =  zone.getIndexAtLength(length);
                    
                    float distance = dist;
//                    float d = glm::distance(pos, closestPoint);
//                    if(abs(index-closestPointIndex) >0.01) distance = d;
                    
                    pos = zone.getPointAtIndexInterpolated(index) + (zone.getNormalAtIndex(index)*distance);
                    vel = glm::vec3(0);
                    
                }else{
                    target = zone.getClosestPoint(pos);//zone.getBoundingBox().getCenter();
                    firstTime = true;
                    vel = normVecToTarget(target);
                }
                
            }
                break;
                
                
            case MOUSEFOLLOW:
            {
                glm::vec3 mousePos = glm::vec3(0.0f);
                int offsetX = ofGetWidth()-ofGetHeight();
                if(ofGetMouseX()> offsetX){
                    mousePos.x = (ofGetMouseX()-offsetX)/(float)(ofGetHeight());
                    mousePos.y = ofGetMouseY()/(float)ofGetHeight();
                    vel = normVecToTarget(mousePos);
                }

            }
                break;
                
            case NOISE:
                vel = glm::vec3(1.0f,0.0f,0.0f);
                vel = glm::rotate(vel, float(ofNoise(glm::vec4(pos*10., ofGetElapsedTimef())) * TWO_PI), glm::vec3(0.0f,0.0f,1.0f));
                break;
                
            case RND:
                vel = glm::vec3(1.0f,0.0f,0.0f);
                vel = glm::rotate(vel, float(ofRandom(100)/100. * TWO_PI), glm::vec3(0.0f,0.0f,1.0f));
                break;
                
            case SINUS:
                vel = glm::vec3(1.0f,0.0f,0.0f);
                vel = glm::rotate(vel, float(glm::sin(ofGetElapsedTimef())), glm::vec3(0.0f,0.0f,1.0f));
                break;
        }
        
        
        cout << vel << endl;

        
        vel = vel*(float)speed;
        pos = pos + vel;

//        pos = pos + glm::vec3();
        
        //    pos = pos +
        
        glm::vec3 linePos = glm::vec3(0.0f,0.0f,0.0f);

        // ADD MODULATION
        modulator.update();
        if( doModulate ) linePos = modulator.getModualtedPos(pos, vel);
        else linePos = pos ;

        
        // jump walls
        if(linePos.x > 1) pos.x = linePos.x-1;
        if(linePos.x < 0) pos.x = 1-linePos.x;
        if(linePos.y > 1) pos.y = linePos.y-1;
        if(linePos.y < 0) pos.y = 1-linePos.y;
        
        // ADD TO HISTORY
        history.push_back( linePos );
        
        
        // reduce history according length
        while(history.size()>length) history.erase(history.begin());
        
        
        //  create poly
        poly.clear();
        poly.addVertices(history);

    }
    
    glm::vec3 normVecToTarget(glm::vec3 t){
        glm::vec3 forceDir = t - pos;
        return glm::normalize(forceDir);
    }
    
    ofPolyline & getPoly(){
        return poly;
    }
    
    void draw(int x, int y, int w, int h){
        
        ofSetLineWidth(2);
        
        
        ofPushMatrix();
        {
            ofTranslate(x,y);
            ofScale(w,h);
            ofSetColor(50);
            ofDrawRectangle(0,0,1,1);
            
            ofSetColor(ofColor::red);
            ofDrawCircle(pos, 5./w);
//            ofDrawCircle(pos+vel*10, 5./w);
            
            ofSetColor(ofColor::blue);
            ofDrawCircle(target, 5./w);

            ofSetColor(lineColor.get());
            poly.draw();
            

        }
        ofPopMatrix();
        
        ofSetColor(ofColor::orange);
        modulator.draw(pos.x*w+x, pos.y*h+y, w, h);

        
    }
};

#endif /* Walker_h */
