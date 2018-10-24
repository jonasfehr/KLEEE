//
//  SuperPixel.cpp
//  wandernderPunkt
//
//  Created by Jonas Fehr on 03/09/2018.
//

#include "SuperPixel.hpp"
using namespace cv;

float SuperPixel::getMinDist(glm::vec2 centroid){
    float minDistance = 1024;
    for(auto & c : centroids){
        float dist = glm::distance(c, centroid);
        if(dist < minDistance) minDistance = dist;
    }
    return minDistance;
}


ofFloatPixels SuperPixel::getBoundaryPixels(bool doInvert){
    Mat invMask;
    mask.copyTo(invMask);
//    cvtColor(invMask, invMask, COLOR_BGR2GRAY);
    if(doInvert) bitwise_not ( invMask, invMask );
//    invert(invMask, invMask);
    /// Get the contours
    vector<vector<cv::Point> > contours;
    findContours( invMask, contours, RETR_TREE, CHAIN_APPROX_SIMPLE);
    
    /// Calculate the distances to the contour
    Mat raw_dist( invMask.size(), CV_32F );
    for( int i = 0; i < invMask.rows; i++ )
    {
        for( int j = 0; j < invMask.cols; j++ )
        {
            raw_dist.at<float>(i,j) = (float)pointPolygonTest( contours[0], Point2f((float)j, (float)i), true );
        }
    }
    
    // create image
    double minVal, maxVal;
    minMaxLoc( raw_dist, &minVal, &maxVal );
    minVal = abs(minVal);
    maxVal = abs(maxVal);
    
    ofFloatPixels boundarys;
    boundarys.allocate(1024, 1024, 3);
    /// Depicting the  distances graphically
    Mat drawing = ofxCv::toCv(boundarys);//Mat::zeros( src.size(), CV_8UC3 );
    //            imitate(drawing);
    for( int i = 0; i < invMask.rows; i++ )
    {
        for( int j = 0; j < invMask.cols; j++ )
        {
            if( raw_dist.at<float>(i,j) > 0 )
            {
                float dist = 1.-(raw_dist.at<float>(i,j) / maxVal);
                dist = dist*dist*dist*dist;
                drawing.at<Vec3f>(i,j)[0] = dist;
                drawing.at<Vec3f>(i,j)[1] = dist;
                drawing.at<Vec3f>(i,j)[2] = dist;
            }
            else
            {
                drawing.at<Vec3f>(i,j)[0] = 1;
                drawing.at<Vec3f>(i,j)[1] = 1;
                drawing.at<Vec3f>(i,j)[2] = 1;
            }
        }
    }
    
    Mat grad_x, grad_y;
    Mat g_img;
    int ddepth = CV_32F;
    int scale = 1;
    float delta = 0.5;
    
    cvtColor(drawing,g_img,CV_BGR2GRAY);
    
    Scharr(g_img,grad_x,ddepth,1,0,scale, delta);
    Scharr(g_img,grad_y,ddepth,0,1,scale, delta);
    
    for( int i = 0; i < drawing.rows; i++ )
    {
        for( int j = 0; j < drawing.cols; j++ )
        {
            drawing.at<Vec3f>(i,j)[1] = grad_x.at<float>(i,j);
            drawing.at<Vec3f>(i,j)[2] = grad_y.at<float>(i,j);
        }
    }
//                ofImage ofImg;
//                ofImg.setFromPixels(boundarys);
//                ofImg.save("distMap.jpg", OF_IMAGE_QUALITY_BEST);
    
    return boundarys;
    }


void SuperPixel::setupContours(){
    contour.findContours(mask);
}
