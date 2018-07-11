//
//  PostProcessor.h
//  wandernderPunkt
//
//  Created by Jonas Fehr on 22/05/2018.
//

#ifndef PostProcessor_h
#define PostProcessor_h

#include "ofIldaFrame.h"

class PostProcessor: public ofxIldaFrame{
public:
    
    
    
    void updatePoints(vector<glm::vec3> originalPoints, ofFloatColor pointColorOriginal){
        points.clear();
        ofFloatColor &pcolor = pointColorOriginal;
        
        if(originalPoints.size() > 0) {
            
            glm::vec2 displace = glm::vec2(0.);
            
            
            glm::vec3 startPoint = transformPoint(originalPoints.front());
            glm::vec3 endPoint = transformPoint(originalPoints.back());
            
            // blanking at start
            for(int n=0; n<params.output.startBlanks; n++) {
                points.push_back( Point(startPoint, ofFloatColor(0, 0, 0, 0)));
            }
            
            // repeat at start
            for(int n=0; n<params.output.startCount; n++) {
                points.push_back( Point(startPoint, pcolor) );
            }
            
            
            // add points
            for(int j=0; j<originalPoints.size(); j++) {
                points.push_back( Point(transformPoint(originalPoints[j]), pcolor) );
            }
            
            // repeat at end
            for(int n=0; n<params.output.endCount; n++) {
                points.push_back( Point(endPoint, pcolor) );
            }
            
            // blanking at end
            for(int n=0; n<params.output.endBlanks; n++) {
                points.push_back( Point(endPoint, ofFloatColor(0, 0, 0, 0) ));
            }
            
        }
        //            } else {
        if(processedPolys.size()==0){ // for safety
            ofxIlda::Point point;
            point.set(glm::vec3(0.5,0.5,0.0), ofFloatColor(0));
            points.push_back(point);
        }
    }
};

#endif /* PostProcessor_h */


