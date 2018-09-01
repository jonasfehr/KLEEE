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

class RandomWithoutRepeate{
public:
    
    void add(){
        int newNumber = array.size();
        array.push_back(newNumber);
        std::random_shuffle ( array.begin(), array.end() );
    }
    
    int getNext(){
        index++;
        if(index>=array.size()){
            std::random_shuffle ( array.begin(), array.end() );
            index = 0;
        }
        return array[ index ];
    }
private:
    int index = 0;
    vector<int> array;
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
    
    
    void setBoundaryPixels(ofFloatPixels boundaryPixels){
        this->boundaryPixels = boundaryPixels;
        hasBoundaryPixels = true;
        
        boundaryImage.setFromPixels(boundaryPixels.getChannel(0));
        boundaryImage.update();
        checkAttracktorsWithinBoarders();
        
    }
    
    void checkAttracktorsWithinBoarders(){
        if(!hasBoundaryPixels) return;
        for(auto & a : attractors){
            int w = boundaryPixels.getWidth();
            int h = boundaryPixels.getHeight();
            while(boundaryPixels.getColor(a.getPos().x*w, a.getPos().y*h).r > 0.8){
                a = Attractor();
            }
        }
    }
    
    
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
        
        for(int i = 0; i < 10; i++){
            Attractor attractor;
            attractors.push_back(attractor);
            random.add();
        }
        target=attractors[random.getNext()].getPos();
    }
    
    void setup(){
        
        
        
    }
    
    void addZone(ofPolyline &zone){
        this->zone = zone;
    }
    
    void seek(glm::vec3 target) {
        glm::vec3 desired = target - pos;
        desired = glm::normalize(desired);
        desired = desired * speed.get()*scaler.get();
        glm::vec3 steer = desired - vel;
        if(glm::length(steer)>maxForce.get()*scaler.get()) steer = glm::normalize(steer)*maxForce.get()*scaler.get();
        vel = vel + steer;
    }
    
    void avoid(glm::vec3 target, float minDistance) {
        
        float distance = glm::distance(pos,target);
        //        distance = glm::clamp(distance,0.01f,0.4f);
        if(distance<minDistance){
            glm::vec3 predator = target - pos;
            
            predator = glm::normalize(predator);
            //            predator = predator / distance;
            
            predator = predator * maxRepulsionSpeed.get()*scaler.get();
            glm::vec3 steer = (predator - vel)*(-1.0);
            if(glm::length(steer)>maxRepulsion.get()*scaler.get()) steer = glm::normalize(steer)*maxRepulsion.get()*scaler.get();
            vel = vel + steer;
        }
        
    }
    
    bool bBorders = false;
    void update(){
        // Noise based Movement
        
        switch(movementPattern){
            case STEERING:
            {
                seek(target);
                
                // AVOID WALLS
//                float boarder = 0.025;
                glm::vec3 desired = glm::vec3(0);
//                if(pos.x > 1-boarder){ bBorders = true; desired = glm::vec3(-speed.get(),vel.y,0);}
//                if(pos.x < boarder)  { bBorders = true; desired = glm::vec3(speed.get(),vel.y,0);}
//                if(pos.y > 1-boarder){ bBorders = true; desired = glm::vec3(vel.x,-speed.get(),0);}
//                if(pos.y < boarder)  { bBorders = true; desired = glm::vec3(vel.x,speed.get(),0);}
                
                
                int w = boundaryPixels.getWidth();
                int h = boundaryPixels.getHeight();
                ofFloatColor bCol = boundaryPixels.getColor(pos.x*w, pos.y*h);
                //                cout << pos.x*w << " / " << pos.y*h << endl;
                if(bCol.r<0.2) bBorders = false;

                if(bCol.r > 0.81 || bBorders){
                    bBorders = true;
                    desired = glm::vec3(-(bCol.g-0.5), -(bCol.b-0.5),0)*speed.get();
                    seek(glm::vec3(0.5,0.5,0));
                }
                
                if(bBorders){
                    glm::vec3 steer = desired - vel;
                    if(glm::length(steer)>speed.get()) steer = glm::normalize(steer)*speed.get();
                    vel = vel + steer;
                }
                
                for(int i = 0; i< attractors.size(); i++){
                    if(i!=activeAttractor){
                        avoid(attractors[i].getPos(), minDistancePred);
                        //                        glm::vec2 repulsion = - attractors[i].getAttraction(pos);
                        //                        repulsion = repulsion * maxRepulsion.get()*scaler.get();
                        //                        if(glm::length(repulsion)>maxRepulsion.get()*scaler.get()) repulsion = glm::normalize(repulsion)*maxRepulsion.get()*scaler.get();
                        //                        vel = vel + repulsion;
                        
                    }
                }
                
                
                if(glm::distance(pos,target)<0.01){
                    activeAttractor=random.getNext();
                    
                    // activate all attractors exept the one which is target
                    for(auto & attractor : attractors){
                        attractor.activate();
                    }
                    attractors[activeAttractor].deactivate();
                    
                    target=attractors[activeAttractor].getPos();
                    
                }
                // activate all attractors exept the one which is target
                for(auto & attractor : attractors){
                    
                    attractor.setPos( attractor.getPos()+attractor.vel*attractorMove.get());
                    ofFloatColor bCol = boundaryPixels.getColor(attractor.getPos().x*w, attractor.getPos().y*h);
                    if(bCol.r > 0.80) attractor.vel*= -1;

                }
                
            }
                break;
            case ATTRACTIONS:
            {
                vel = vel + attractors[activeAttractor].getAttraction(pos);
                //
                //                for(auto & attractor : attractors){
                //                    if(attractor.isActive()){
                //                        vel = vel + attractor.getAttraction(pos);
                //                        if(glm::distance(pos,attractor.getPos())<0.1){
                //                            attractor.deactivate();
                //                        }
                //                    }
                //                }
                if(glm::distance(pos,target)<0.1){
                    activeAttractor=random.getNext();
                    
                    target=attractors[activeAttractor].getPos();
                    for(auto & attractor : attractors){
                        attractor.activate();
                    }
                }
                //                target = zone.getCentroid2D();
                //                vel = vel + glm::distance(pos,target)*normVecToTarget(target);
                //                vel = glm::normalize(vel);
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
        
        
        if(glm::length(vel)>speed)   vel = glm::normalize(vel)*(float)speed;
        
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
        if(bBorders){
            ofPushStyle();
            ofFloatColor c = boundaryPixels.getColor(pos.x*boundaryPixels.getWidth(), pos.y*boundaryPixels.getHeight());
            ofSetColor(255*c.r,0,0);
            boundaryImage.draw(x,y,w,h);
            ofPopStyle();
        }
        
        ofPushStyle();
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
            
            ofSetColor(lineColor.get());
            poly.draw();
            
            for(auto & attractor: attractors){
                ofSetColor(ofColor::orange);
                ofDrawCircle(attractor.getPos(), 5./w);
            }
            
            ofSetColor(ofColor::blue);
            ofDrawCircle(target, 5./w);
            
        }
        ofPopMatrix();
        ofSetColor(ofColor::orange);
        modulator.draw(pos.x*w+x, pos.y*h+y, w, h);
        
        
        ofPopStyle();
        
        
    }
};

#endif /* Walker_h */
