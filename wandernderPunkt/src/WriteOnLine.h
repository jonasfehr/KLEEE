//
//  WriteOnLine.h
//
//  Created by Jonas Fehr on 06/10/2018.
//

#ifndef WriteOnLine_h
#define WriteOnLine_h

#include "ofMain.h"
#include "ofxSvg.h"


class WriteOnLine{
public:
    
    void setup(string filename, int index, float scale, glm::vec3 target, glm::vec3 origin){
        svg.load(filename);
        
        this->setTarget(target);
        this->setOrigin(origin);

        
        for (ofPath p: svg.getPaths()){
            // svg defaults to non zero winding which doesn't look so good as contours
            p.setPolyWindingMode(OF_POLY_WINDING_ODD);
            const vector<ofPolyline>& lines = p.getOutline();
            for(const ofPolyline & line: lines){
                outlines.push_back(line);//.getResampledBySpacing(  1));
            }
        }

        this->scale = scale;
        select(index);

    }
    
    void nextWord(){
        currentIndex = (currentIndex+1);//%outlines.size();
        select(currentIndex);
    }
    
    
    bool select(int index){
        if(index>outlines.size()-1) return false;
        this->currentIndex = index;
        
        word = outlines[currentIndex];
        
        float drawHeight = word.getBoundingBox().height*scale;
        float wordLength = glm::distance(word[word.size()-1], word[0]);
        float drawWidth = word.getBoundingBox().width*scale;
        
        distancePoints = glm::distance(target, origin);
        
        glm::vec3 startDrawingPoint = (glm::normalize(target-origin) * ((distancePoints-drawWidth)/2));
        
        glm::mat4 translationMatrix = glm::translate(startDrawingPoint);
        
        float angle = glm::orientedAngle(glm::vec3(1,0,0), glm::normalize(target-origin), glm::vec3(0,0,1) );
        angle -= glm::orientedAngle(glm::vec3(1,0,0), glm::normalize(word[word.size()-1]-word[0]), glm::vec3(0,0,1) );
        glm::mat4 rotationMatrix = glm::rotate( angle, glm::vec3(0,0,1) );
        
        float scaleX = distancePoints/wordLength;
        glm::mat4 scalingMatrix = glm::scale(glm::vec3(drawWidth/word.getBoundingBox().width, drawHeight/word.getBoundingBox().height , 1.0f));
        
        transform = translationMatrix * rotationMatrix * scalingMatrix;
        
        amtWord = drawWidth/distancePoints;
        amtStartWord = (1.-amtWord)/2.0;
        
        float distanceOT = glm::distance(origin, target); // avoid writ
        if(distanceOT<amtWord) currentIndex = (currentIndex -1)%outlines.size();
        
        return true;
    }
    
    glm::vec3 getPoint(float step){
        
        if(step > amtStartWord && step < amtStartWord+amtWord){ // draw word
            int num = (step-amtStartWord)/amtWord * word.size();
            
            glm::vec3 newPoint(word[num] - word[0]);
            glm::vec4 point = transform*glm::vec4(newPoint , 1);
            return glm::vec3(point)+ origin;
        } else{
            return glm::normalize(target-origin) * step * distancePoints +origin;
        }
    }
    
    glm::vec3 getOffset(float step){
        if(currentIndex > outlines.size()-1) return glm::vec3(0.);
        return getPoint(step)-(glm::normalize(target-origin) * step * distancePoints +origin);
    }
    
    glm::vec3 getOffset(glm::vec3 pos){
        float distanceOT = glm::distance(origin, target);
        if(distanceOT<amtWord) return glm::vec3(0.0);
        float step = glm::distance(origin, pos)/distanceOT;
        return getPoint(step)-(glm::normalize(target-origin) * step * distancePoints +origin);
    }
    
    
    
    

    void setTarget(glm::vec3 target){
//        this->origin = this->target;
        this->target = target;
    };
    void setOrigin(glm::vec3 origin){
        this->origin = origin;
        
    };
    
    float scale;
    void setScale(float scale){
        this->scale = scale;
        select(currentIndex);
    };
    
    void setCurrentIndex(int index){
        this->currentIndex = index;
    };
    
    int getCurrentIndex(){
        return this->currentIndex;
    };
    
    bool isFinished(){
        if(currentIndex>outlines.size()-1) return true;
        else return false;
    }
    
private:
    glm::mat4  transform;
    float amtWord;
    float amtStartWord;
    float distancePoints;
    int currentIndex;
    ofxSVG svg;
    vector<ofPolyline> outlines;
    ofPolyline word;
    glm::vec3 origin;
    glm::vec3 target;
};

#endif /* WriteOnLine_h */

