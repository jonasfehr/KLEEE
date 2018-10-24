//
//  SuperPixel.hpp
//  wandernderPunkt
//
//  Created by Jonas Fehr on 03/09/2018.
//

#ifndef SuperPixel_hpp
#define SuperPixel_hpp

#include "ofxCv.h"
#include "ofMain.h"


class SuperPixel{
public:
    int label;
    cv::Vec3b meanColor;
    cv::Mat mask;
    cv::MatND hist;
    vector<glm::vec2> centroids;
    ofxCv::ContourFinder contour;
    void setupContours();
    
    int segment=-1;

    
    float getMinDist(glm::vec2 centroid);
    ofFloatPixels getBoundaryPixels(bool doInvert);
    
};

#endif /* SuperPixel_hpp */
