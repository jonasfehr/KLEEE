//
//  BlobFinder.h
//  wandernderPunkt
//
//  Created by Jonas Fehr on 21/05/2018.
//

#ifndef BlobFinder_h
#define BlobFinder_h

#include "ofxOpenCv.h"

class BlobFinder{
public:
    
    ofParameter<bool> drawColorImg{"drawColorImg", true};
    ofParameter<bool> drawGreyImg{"drawGreyImg", true};
    ofParameter<bool> drawBlobs{"drawBlobs", true};
    ofParameterGroup parameters{"BlobFinder", drawColorImg, drawGreyImg, drawBlobs};
    

    
    ofImage img;
    ofPolyline zone;
    
    ofxCvColorImage            colorImg;
    ofxCvGrayscaleImage     grayImage;
    ofxCvContourFinder     contourFinder;
    
    
    void setup(){
        img.load("test.png");
        colorImg.allocate(img.getWidth(), img.getHeight());
        grayImage.allocate(img.getWidth(), img.getHeight());
    }
    
    void update(){
        colorImg.setFromPixels(img.getPixels());
        grayImage = colorImg;
        
        contourFinder.findContours(grayImage, 20, (img.getWidth() * img.getHeight())/2, 10, true);    // find holes
        
    }
    
    ofPolyline & getZone(int indx){
        if(contourFinder.blobs.size()>0){
            zone.clear();
            for(auto & pts : contourFinder.blobs[indx].pts){
                zone.addVertex(pts.x/img.getWidth(), pts.y/img.getHeight());
            }
            zone.close();
            zone.flagHasChanged();
        }
        return zone;
    }
    
    void draw(int x, int y, int w, int h){
        ofPushMatrix();
        {
            ofTranslate(x,y);
            ofScale(w/img.getWidth(), h/img.getHeight());
            
            ofSetColor(255);
            if(drawColorImg.get()) img.draw(0,0);
            if(drawGreyImg.get())  grayImage.draw(0,0);
            
            if(drawBlobs.get()){
                for (int i = 0; i < contourFinder.nBlobs; i++){
                    contourFinder.blobs[i].draw(0,0);
                    ofDrawBitmapString(ofToString(i), contourFinder.blobs[i].boundingRect.getCenter().x, contourFinder.blobs[i].boundingRect.getCenter().y);
                    // draw over the centroid if the blob is a hole
                    ofSetColor(255);
                    if(contourFinder.blobs[i].hole){
                        ofDrawBitmapString("hole", contourFinder.blobs[i].boundingRect.getCenter().x, contourFinder.blobs[i].boundingRect.getCenter().y);
                    }
                }
            }
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
