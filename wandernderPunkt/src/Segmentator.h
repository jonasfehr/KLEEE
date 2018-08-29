//
//  Segmentator.h
//  wandernderPunkt
//
//  Created by WandernderPunkt on 27.08.18.
//

#ifndef Segmentator_h
#define Segmentator_h
#include "ofxGui.h"
#include "ofxCv.h"


//#include "SLICSuperpixel.h"
//#include "SuperpixelSegmentation.h"

#include <opencv2/ximgproc.hpp>
#include "TextureColorSegmentation.hpp"

class Segmentator{
public:
    
    Segmentator(){
        ofAddListener(parametersSLIC.parameterChangedE(), this, &Segmentator::listenerFunction);
        ofAddListener(parametersFilters.parameterChangedE(), this, &Segmentator::listenerFunction);
        doUpdate = true;
    }
    
    void listenerFunction(ofAbstractParameter& e){
        doUpdate = true;
    }
    
    void slicGray(cv::Mat inputMat){
        
        if(doUpdate){
            Mat frame;
            inputMat.copyTo(frame);
            
            filters(frame, frame);

            
            cvtColor(frame, frame, COLOR_BGR2GRAY);
            
            double t = (double) getTickCount();
            
            Ptr<cv::ximgproc::SuperpixelSLIC> slic = cv::ximgproc::createSuperpixelSLIC(frame,algorithm+100,region_size,float(ruler));
            slic->iterate(num_iterations);
            if (min_element_size>0)
                slic->enforceLabelConnectivity(min_element_size);
            
            t = ((double) getTickCount() - t) / getTickFrequency();
            cout << " segmentation took " << (int) (t * 1000) << " ms with " << slic->getNumberOfSuperpixels() << " superpixels" << endl;
            
            // get the contours for displaying
            Mat mask;
            slic->getLabelContourMask(mask, true);
            frame.copyTo(result);
            result.setTo(Scalar(0, 0, 255), mask);
            
            Mat labels;
            slic->getLabels(labels);
            
            
            // for histogram

            
            // Quantize the hue to 30 levels
            // and the saturation to 32 levels
            int histSize[] = {255};
            float range[] = { 0, 180 };
            const float* ranges[] = { range };
            MatND hist;
            int channels[] = {0};
            
            inputMat.copyTo(coloredSP);// initialise
            
            superPixels.clear();

            for(int i = 0; i<slic->getNumberOfSuperpixels(); i++){
                int numPixels = 0;
                Vec3b meanColor;
                int value = 0;
                Mat mask = Mat::zeros(labels.rows, labels.cols, CV_8UC1);
                glm::vec2 centroid = glm::vec2(0,0);
                bool isTopRow = false;
                
                for(int r=0; r<labels.rows; r++){
                    for(int c=0; c<labels.cols; c++){
                        if(labels.at<int>(r,c) == i){
                            uchar val = frame.at<uchar>(r,c);
                            value += val;
                            numPixels++;
                            mask.at<uchar>(r,c) = 255;
                            
                            centroid.x += r;
                            centroid.y += c;
                            
                            if(r==0) isTopRow = true;
                        }
                    }
                }
                meanColor.val[0] = value/numPixels;
                meanColor.val[1] = value/numPixels;
                meanColor.val[2] = value/numPixels;
                
                centroid = centroid/numPixels;
                
                calcHist( &frame, 1, channels, mask, hist, 1, histSize, ranges, true, false );
                
                SuperPixel newSP;
                newSP.label = i;
                newSP.meanColor = meanColor;
                newSP.mask = mask;
                hist.copyTo(newSP.hist);
                newSP.centroids.push_back(centroid);
                newSP.isTopRow = isTopRow;
                superPixels.push_back(newSP);
                
                
                for(int r=0; r<labels.rows; r++){
                    for(int c=0; c<labels.cols; c++){
                        if(labels.at<int>(r,c) == i){
                            coloredSP.at<Vec3b>(r,c) = meanColor;
                        }
                    }
                }
                
            }
            
            for(int j = 0; j<5; j++){
            
            for( auto i = superPixels.begin(); i != superPixels.end(); )
            {
                for( auto i2 = i+1; i2 != superPixels.end(); )
                {
                    //                    if(sp1.label == sp2.label) continue;
                    /*
                     compare methods
                     CV_COMP_CORREL Correlation
                     CV_COMP_CHISQR Chi-Square
                     CV_COMP_INTERSECT Intersection
                     CV_COMP_BHATTACHARYYA Bhattacharyya distance
                     CV_COMP_HELLINGER Synonym for CV_COMP_BHATTACHARYYA
                     */
                    
                    double histCompaire = compareHist(i->hist, i2->hist,CV_COMP_CORREL);
                    
                    float minDistance = 1024;
                    for(auto & c : i2->centroids){
                        float dist = i->getMinDist(c);
                        if(dist < minDistance) minDistance = dist;
                    }
                    if( histCompaire > combine_treshold  && minDistance<maxDistance){
                    

//                    if((abs(i->meanColor[0]-i2->meanColor[0])<(combine_treshold)*255 && glm::distance(i->centroid, i2->centroid)<maxDistance) ){
                        bitwise_or(i->mask, i2->mask, i->mask);
                        
                        int value = (i->meanColor.val[0]+i2->meanColor.val[0])/2;

                        i->meanColor.val[0] = value;
                        i->meanColor.val[1] = value;
                        i->meanColor.val[2] = value;
                        for(auto & c : i2->centroids) i->centroids.push_back(c);

                        
                        calcHist( &frame, 1, channels, i->mask, i->hist, 1, histSize, ranges, true, false );
                        
                        i2 = superPixels.erase(i2);
                    }
                    else ++i2;
                }
                ++i;
            }
            }
            for( auto i = superPixels.begin(); i != superPixels.end(); )
            {
                for( auto i2 = i+1; i2 != superPixels.end(); )
                {
                    
                    if(i->isTopRow && i2->isTopRow){
                        bitwise_or(i->mask, i2->mask, i->mask);
                        
                        int value = (i->meanColor.val[0]+i2->meanColor.val[0])/2;
                        
                        i->meanColor.val[0] = value;
                        i->meanColor.val[1] = value;
                        i->meanColor.val[2] = value;
                        
                        for(auto & c : i2->centroids) i->centroids.push_back(c);

                        
                        calcHist( &frame, 1, channels, i->mask, i->hist, 1, histSize, ranges, true, false );
                        
                        i2 = superPixels.erase(i2);
                    }
                    else ++i2;
                }
                ++i;
            }
            
            
            for(auto & sp: superPixels){
                if(sp.isTopRow){
                    Mat invMask;
                    invert(sp.mask, invMask);
                    sp.contour.findContours(invMask);
                }
                else sp.contour.findContours(sp.mask);
                
                // create dst with background color of your choice
                cv::Mat dst(frame.size(),frame.type(),sp.meanColor);
                
                // now copy
                dst.copyTo(final, sp.mask);
            }
            
            doUpdate = false;
        }
        
    }




    
    class SuperPixel{
    public:
        int label;
        Vec3b meanColor;
        Mat mask;
        MatND hist;
        vector<glm::vec2> centroids;
        bool isTopRow;
        
        float getMinDist(glm::vec2 centroid){
            float minDistance = 1024;
            for(auto & c : centroids){
                float dist = glm::distance(c, centroid);
                if(dist < minDistance) minDistance = dist;
            }
            return minDistance;
        }
      

        ContourFinder contour;
//        bool hasContour;
//
//        bool createContour(){
//
//        }
        
        
        ofFloatPixels getBoundaryPixels(){
            Mat invMask;
            mask.copyTo(invMask);
            invert(invMask, invMask);
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
            
            double minVal, maxVal;
            minMaxLoc( raw_dist, &minVal, &maxVal );
            minVal = abs(minVal);
            maxVal = abs(maxVal);
            
            ofFloatPixels boundarys;
            boundarys.allocate(1024, 1024, 3);
            /// Depicting the  distances graphically
            Mat drawing = toCv(boundarys);//Mat::zeros( src.size(), CV_8UC3 );
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
//            ofImage ofImg;
//            ofImg.setFromPixels(boundarys);
//            ofImg.save("distMap.jpg", OF_IMAGE_QUALITY_BEST);
            return boundarys;
        }
        
        
        
    };
    

    
    void filters(Mat & src, Mat & dst){
        // flags RECURS_FILTER = 1 NORMCONV_FILTER = 2
//        dst = correctGamma(src, gamma);
        
//        for( int y = 0; y < src.rows; y++ ) {
//            for( int x = 0; x < src.cols; x++ ) {
//                for( int c = 0; c < 3; c++ ) {
//                    dst.at<Vec3b>(y,x)[c] =
//                    saturate_cast<uchar>( contrast*( src.at<Vec3b>(y,x)[c] ) + brightness );
//                }
//            }
//        }
        
        if(doEdgePreservingFiltering) edgePreservingFilter(src, dst, 1, sigma_s_EPF, sigma_r_EPF);
        if(doStylisation) stylization(src, dst, sigma_s_S, sigma_r_S);
        if(doAnistriopicDiffusion)ximgproc::anisotropicDiffusion(src, dst, alpha, k, niters);
        
        
    }
    
    void draw(){
        ofxCv::drawMat(result,0,0,512,512);
        ofxCv::drawMat(coloredSP,512,0,512,512);
        ofxCv::drawMat(final,1024,0,512,512);
        
    }
    
    vector<SuperPixel> superPixels;

    Mat result, coloredSP, final;
    bool doUpdate;
    
//    ofParameter<float> gamma{"gamma", 2.2, 0, 5};
//    ofParameter<int> contrast{"contrast", 1, 1, 5};
//    ofParameter<int> brightness{"brightness", 0, 0, 100};
    ofParameter<bool> doEdgePreservingFiltering{"doEdgePreservingFiltering", false};
    ofParameter<int> sigma_s_EPF{"sigma_s_EPF", 60, 0, 200};
    ofParameter<float> sigma_r_EPF{"sigma_r_EPF", 0.6, 0, 1};
    ofParameter<bool> doStylisation{"doStylisation", false};
    ofParameter<int> sigma_s_S{"sigma_s_S", 60, 0, 200};
    ofParameter<float> sigma_r_S{"sigma_r_S", 0.6, 0, 1};
    ofParameter<bool> doAnistriopicDiffusion{"doAnistriopicDiffusion", true};
    ofParameter<float> alpha{"alpha", 0.2, 0, 1};
    ofParameter<int> k{"k", 10, 1, 30};
    ofParameter<int> niters{"iterations", 4, 1, 10};
    
    ofParameterGroup parametersFilters{"Filter",  doEdgePreservingFiltering,sigma_s_EPF, sigma_r_EPF, doStylisation, sigma_s_S, sigma_r_S,doAnistriopicDiffusion,alpha,k, niters };
    
    
    
    ofParameter<int> algorithm{"Algorithm", 0, 0, 2};
    ofParameter<int> region_size{"Region size", 50, 1, 200};
    ofParameter<int> ruler{"Ruler", 30, 1, 100};
    ofParameter<int> min_element_size{"min element size", 50, 1, 100};
    ofParameter<int> num_iterations{"Iterations", 3, 1, 30};
    ofParameter<float> combine_treshold{"combine_treshold", 0.3, 0, 1};
    ofParameter<float> maxDistance{"maxDistance", 100, 0, 1024};

    
    ofParameterGroup parametersSLIC{"SLIC", algorithm, region_size, ruler, min_element_size, num_iterations,combine_treshold,maxDistance};
    
    Mat correctGamma( Mat& img, double gamma ) {
        double inverse_gamma = 1.0 / gamma;
        
        Mat lut_matrix(1, 256, CV_8UC1 );
        uchar * ptr = lut_matrix.ptr();
        for( int i = 0; i < 256; i++ )
            ptr[i] = (int)( pow( (double) i / 255.0, inverse_gamma ) * 255.0 );
        
        Mat result;
        LUT( img, lut_matrix, result );
        
        return result;
    }
    
    Scalar hsv_to_rgb(Scalar c) {
        Mat in(1, 1, CV_32FC3);
        Mat out(1, 1, CV_32FC3);
        
        float * p = in.ptr<float>(0);
        
        p[0] = (float)c[0] * 360.0f;
        p[1] = (float)c[1];
        p[2] = (float)c[2];
        
        cvtColor(in, out, COLOR_HSV2RGB);
        
        Scalar t;
        
        Vec3f p2 = out.at<Vec3f>(0, 0);
        
        t[0] = (int)(p2[0] * 255);
        t[1] = (int)(p2[1] * 255);
        t[2] = (int)(p2[2] * 255);
        
        return t;
        
    }
    
    Scalar color_mapping(int segment_id) {
        
        double base = (double)(segment_id) * 0.618033988749895 + 0.24443434;
        
        return hsv_to_rgb(Scalar(fmod(base, 1.2), 0.95, 0.80));
        
    }
    
};


//    void slic(cv::Mat inputMat){
//
//        if(doUpdate){
//            Mat frame;
//            inputMat.copyTo(frame);
//
//            filters(frame, frame);
//
//            Mat converted;
//            cvtColor(frame, converted, COLOR_BGR2HSV);
//
//            double t = (double) getTickCount();
//
//            Ptr<cv::ximgproc::SuperpixelSLIC> slic = cv::ximgproc::createSuperpixelSLIC(converted,algorithm+100,region_size,float(ruler));
//            slic->iterate(num_iterations);
//            if (min_element_size>0)
//                slic->enforceLabelConnectivity(min_element_size);
//
//            t = ((double) getTickCount() - t) / getTickFrequency();
//            cout << " segmentation took " << (int) (t * 1000) << " ms with " << slic->getNumberOfSuperpixels() << " superpixels" << endl;
//
//            // get the contours for displaying
//            Mat mask;
//            slic->getLabelContourMask(mask, true);
//            frame.copyTo(result);
//            result.setTo(Scalar(0, 0, 255), mask);
//
//            Mat labels;
//            slic->getLabels(labels);
//
//            vector<SuperPixel> superPixels;
//
//            // for histogram
//            Mat frameHSV;
//            cvtColor(frame, frameHSV, CV_BGR2HSV);
//
//            // Quantize the hue to 30 levels
//            // and the saturation to 32 levels
//            int hbins = 30, sbins = 32, vbins = 32;
//            int histSize[] = {hbins, sbins, vbins};
//            // hue varies from 0 to 179, see cvtColor
//            float hranges[] = { 0, 180 };
//            // saturation varies from 0 (black-gray-white) to
//            // 255 (pure spectrum color)
//            float sranges[] = { 0, 256 };
//            float vranges[] = { 0, 256 };
//            const float* ranges[] = { hranges, sranges, vranges };
//            MatND hist;
//            // we compute the histogram from the 0-th and 1-st channels
//            int channels[] = {0, 1, 2};
//
//            frame.copyTo(coloredSP);// initialise
//
//            for(int i = 0; i<slic->getNumberOfSuperpixels(); i++){
//                int numPixels = 0;
//                Vec3b meanColor;
//                int r_col = 0;
//                int g_col  = 0;
//                int b_col  = 0;
//                Mat mask = Mat::zeros(labels.rows, labels.cols, CV_8UC1);
//                glm::vec2 centroid = glm::vec2(0,0);
//                bool isTopRow;
//
//                for(int r=0; r<labels.rows; r++){
//                    for(int c=0; c<labels.cols; c++){
//                        if(labels.at<int>(r,c) == i){
//                            Vec3b color = frame.at<Vec3b>(r,c);
//                            r_col += color.val[0];
//                            g_col += color.val[1];
//                            b_col += color.val[2];
//
//                            centroid.x += r;
//                            centroid.y += c;
//
//                            numPixels++;
//
//                            mask.at<uchar>(r,c) = 255;
//
//                            if(r==0) isTopRow = true;
//
//                        }
//                    }
//                }
//                meanColor.val[0] = r_col/numPixels;
//                meanColor.val[1] = g_col/numPixels;
//                meanColor.val[2] = b_col/numPixels;
//
//                centroid = centroid/numPixels;
//
//                calcHist( &frameHSV, 1, channels, mask, hist, 2, histSize, ranges, true, false );
//
//                SuperPixel newSP;
//                newSP.label = i;
//                newSP.meanColor = meanColor;
//                newSP.mask = mask;
//                hist.copyTo(newSP.hist);
//                newSP.centroids.push_back(centroid);
//                newSP.isTopRow = isTopRow;
//                superPixels.push_back(newSP);
//
//
//                for(int r=0; r<labels.rows; r++){
//                    for(int c=0; c<labels.cols; c++){
//                        if(labels.at<int>(r,c) == i){
//                            coloredSP.at<Vec3b>(r,c) = meanColor;
//                        }
//                    }
//                }
//
//            }
//
//            for( auto i = superPixels.begin(); i != superPixels.end(); )
//            {
//                for( auto i2 = i+1; i2 != superPixels.end(); )
//                {
//                    //                    if(sp1.label == sp2.label) continue;
//                    /*
//                     compare methods
//                     CV_COMP_CORREL Correlation
//                     CV_COMP_CHISQR Chi-Square
//                     CV_COMP_INTERSECT Intersection
//                     CV_COMP_BHATTACHARYYA Bhattacharyya distance
//                     CV_COMP_HELLINGER Synonym for CV_COMP_BHATTACHARYYA
//                     */
//
//                    double histCompaire = compareHist(i->hist, i2->hist,CV_COMP_CORREL);
//
//                    float minDistance = 1024;
//                    for(auto & c : i2->centroids){
//                        float dist = i->getMinDist(c);
//                        if(dist < minDistance) minDistance = dist;
//                    }
//                    if( histCompaire > combine_treshold  && minDistance<maxDistance){
//                    //                    cout << i->label<< "/" << i2->label<<" "<<histCompaire << endl;
////                    if((abs(i->meanColor[0]-i2->meanColor[0])<(1.-combine_treshold)*255)  && glm::distance(i->centroid, i2->centroid)<maxDistance){
//                        bitwise_or(i->mask, i2->mask, i->mask);
//                        i->meanColor = (i->meanColor+i2->meanColor)/2;
//
//                        calcHist( &frameHSV, 1, channels, i->mask, i->hist, 2, histSize, ranges, true, false );
//
//                        for(auto & c : i2->centroids) i->centroids.push_back(c);
//
//
//                        i2 = superPixels.erase(i2);
//                    }
//                    else ++i2;
//                }
//                ++i;
//            }
//
//
//            for(auto & sp: superPixels){
//                // create dst with background color of your choice
//                cv::Mat dst(frame.size(),frame.type(),sp.meanColor);
//
//                // now copy
//                dst.copyTo(final, sp.mask);
//            }
//
//            doUpdate = false;
//        }
//
//    }

#endif /* Segmentator_h */

