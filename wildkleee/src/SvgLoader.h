//
//  SvgLoader.h
//  firstTests
//
//  Created by Jonas Fehr on 14/12/2018.
//

#ifndef SvgLoader_h
#define SvgLoader_h

#include <stdio.h>
using namespace std;


#include "ofxSvg.h"


class SvgLoader{
public:
    
    SvgLoader(){};
    SvgLoader(string filename, bool isClosed = false){
        setup(filename, isClosed);
    };

    
    string name;
    
    void setup(string filename, bool isClosed = false){
        svg.load(filename);
        
        auto nameSplit = ofSplitString(filename, ".");
        name = nameSplit[0];
        
        for (ofPath p: svg.getPaths()){
            // svg defaults to non zero winding which doesn't look so good as contours
            p.setPolyWindingMode(OF_POLY_WINDING_ODD);
            const vector<ofPolyline>& lines = p.getOutline();
            for(const ofPolyline & line: lines){
                outlines.push_back(line);//.getResampledBySpacing(  1));
            }
        }
        
        
        // Normalize
        for(auto & line : outlines){
            line.setClosed(isClosed);
            for(auto & p : line){
                p.x /= svg.getHeight();//svg.getWidth();
                p.y /= svg.getHeight();
            }
        }
        
        // calculateBoundingBox
        for(auto & poly : outlines){
            boundingBox.growToInclude(poly.getBoundingBox());
        }
    }
    
    void draw(int x, int y, int w, int h){
        ofPushMatrix();
        {
            ofTranslate(x,y);
//            ofScale(w/width, h/height);
            ofScale(w, h);
            for(auto & l : outlines){
                l.draw();
            }
        }
        ofPopMatrix();
    }
    
    
    glm::vec2 getPointOnLine(int index, float step, float spacing = 0){
        glm::vec2 pos;
        if(spacing != 0){
            ofPolyline resampled = outlines[index].getResampledBySpacing(spacing);
            int i = glm::round(resampled.getIndexAtPercent(step));
            pos = resampled[i];
        }
        else {
            pos = outlines[index].getPointAtPercent(step);
        }
        return pos;
    }
    
    glm::vec2 getPointAtLength(int index, float length){
        return outlines[index].getPointAtLength(length);
    }
    
    glm::vec2 getNormalAtLength(int index, float length){
        return outlines[index].getNormalAtIndexInterpolated(outlines[index].getIndexAtLength(length));
    }
    
    float getLength(int index){
        return outlines[index].getPerimeter();
    }
   
    int getNumOfLines(){
        return outlines.size();
    }
    
    
    ofRectangle getBoundingBox(){
        return boundingBox;
    }
    
//private:
    ofxSVG svg;
    vector<ofPolyline> outlines;
    ofRectangle boundingBox;

};

#endif /* SvgLoader_h */
