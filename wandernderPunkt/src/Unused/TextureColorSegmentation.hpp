//
//  TextureColorSegmentation.hpp
//  wandernderPunkt
//
//  Created by WandernderPunkt on 27.08.18.
//

#ifndef TextureColorSegmentation_hpp
#define TextureColorSegmentation_hpp

#include "ofxCv.h"
#include <stdio.h>
using namespace cv;
using namespace std;

class TextureColorSegmentation{
public:
    //functions from texture.cpp
    int textureDifference(Vec4b p1, Vec4b p2);
    int farTextureDifference(Vec4b p1, Vec4b p2);
    void printTexture(Mat texture);
    void printAtan();
    Mat generateGradient(Mat img);//generates gradient for each pixel
    Mat generateTexture(Mat gradient, int windowSize);//statistical texture pattern measure 20x20 patch
    void segmentTexture(Mat texture);//segment image according to texture only
    
    //functions from color.cpp
    Mat colSeg(Mat image, int winSize);//color segmentation only
    bool similar(Vec3b now, Vec3b actual);//current and seed color distance metric for only color segmentation
    bool farPtSimilar(Vec3b next, Vec3b curr);//local color distance metric for only color segmentation
    
    //
    Mat regionMerge(Mat image ,Mat texture, Mat col, Mat segm, vector<int>pixelsInArea, vector<Vec3b>avgColBGR, int winSize);
    
    //histogram
    void hist(Mat);
    
    void crop(Mat combined, Mat original);
    
    int textureDifference2(Vec4b p1, Vec4b p2);//current and seed texture distance metric for final merge
    int farTextureDifference2(Vec4b p1, Vec4b p2);//local texture distance metric for final merge
    bool similar2(Vec3b now, Vec3b actual);//current and seed color distance metric for final merge
    bool farPtSimilar2(Vec3b next, Vec3b curr);//local color distance metric for final merge
    int ifSimilar(Vec4b nextTexture,Vec4b seedTexture, Vec4b currTexture,
                                            Vec3b nextColor, Vec3b seedColor, Vec3b currColor,
                                            int nextSegment, int seedSegment, int currSegment,
                                            int nextTotalPixels, int seedTotalPixels, int currTotalPixels);//distance metric score for final merge(change binary score to continuous valued)
    
};
#endif /* TextureColorSegmentation_hpp */
