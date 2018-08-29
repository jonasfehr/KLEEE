//
//  BlobFinder.h
//  wandernderPunkt
//
//  Created by Jonas Fehr on 21/05/2018.
//

#ifndef BlobFinder_h
#define BlobFinder_h

#include "ofxCv.h"

class BlobFinder{
public:
    
    ofxCv::ContourFinder contourFinder;
    
    ofParameterGroup parameters;
    ofParameter<float> minArea, maxArea, threshold;
    ofParameter<bool> holes;
    

    
    ofImage img;
    ofPolyline zone;

    
    
    void setup(){
        img.load("test.png");
        
        parameters.add(minArea.set("Min area", 10, 1, 100));
        parameters.add(maxArea.set("Max area", 200, 1, 500));
        parameters.add(threshold.set("Threshold", 128, 0, 255));
        parameters.add(holes.set("Holes", false));
    }
    
    void update(){
        contourFinder.setMinAreaRadius(minArea);
        contourFinder.setMaxAreaRadius(maxArea);
        contourFinder.setThreshold(threshold);
        contourFinder.findContours(ofxCv::toCv(img));
        contourFinder.setFindHoles(holes);
    }
    
    ofPolyline & getZone(int indx){
        return contourFinder.getPolyline(indx);
//        if(contourFinder.blobs.size()>0){
//            zone.clear();
//            for(auto & pts : contourFinder.blobs[indx].pts){
//                zone.addVertex(pts.x/img.getWidth(), pts.y/img.getHeight());
//            }
//            zone.close();
//            zone.flagHasChanged();
//        }
//        return zone;
    }
    
    void draw(int x, int y, int w, int h){
        ofPushMatrix();
        {
            ofTranslate(x,y);
            ofScale(w/img.getWidth(), h/img.getHeight());
            
            ofSetColor(255);
            img.draw(0,0);
            contourFinder.draw();
        }
        ofPopMatrix();
    }
    
};


//http://felix.abecassis.me/2011/09/opencv-morphological-skeleton/

//cv::threshold(img, img, 127, 255, cv::THRESH_BINARY);
//cv::Mat skel(img.size(), CV_8UC1, cv::Scalar(0));
//cv::Mat temp;
//cv::Mat eroded;
//
//cv::Mat element = cv::getStructuringElement(cv::MORPH_CROSS, cv::Size(3, 3));
//
//bool done;
//do
//{
//    cv::erode(img, eroded, element);
//    cv::dilate(eroded, temp, element); // temp = open(img)
//    cv::subtract(img, temp, temp);
//    cv::bitwise_or(skel, temp, skel);
//    eroded.copyTo(img);
//
//    done = (cv::countNonZero(img) == 0);
//} while (!done);
#endif /* BlobFinder_h */
