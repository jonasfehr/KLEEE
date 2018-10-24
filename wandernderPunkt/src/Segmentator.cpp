//
//  Segmentator.cpp
//  wandernderPunkt
//
//  Created by Jonas Fehr on 03/09/2018.
//

#include "Segmentator.hpp"

using namespace cv;

Segmentator::Segmentator(){
    ofAddListener(parametersSLIC.parameterChangedE(), this, &Segmentator::listenerFunction);
    ofAddListener(parametersFilters.parameterChangedE(), this, &Segmentator::listenerFunction);
    doUpdate = true;
}

void Segmentator::listenerFunction(ofAbstractParameter& e){
    doUpdate = true;
}

void Segmentator::filters(Mat & src, Mat & dst){
    if(doEdgePreservingFiltering) edgePreservingFilter(src, dst, 1, sigma_s_EPF, sigma_r_EPF);
    if(doStylisation) stylization(src, dst, sigma_s_S, sigma_r_S);
    if(doAnistriopicDiffusion)ximgproc::anisotropicDiffusion(src, dst, alpha, k, niters);
}

void Segmentator::draw(){
    ofxCv::drawMat(result,0,0,512,512);
    ofxCv::drawMat(coloredSP,512,0,512,512);
    
//    final = result;
//    cv::Mat masked;
//    masked.zeros(1024, 1024, 3);
    result.copyTo(final);
    for(auto & sp : superPixels){
        if(sp.segment == 0){
            Vec3b segCol = Vec3b(255,255,255);
            cv::Mat dst(final.size(),final.type(),segCol);
            dst.copyTo(final, sp.mask);
        }
        else if(sp.segment != -1){
            ofColor color;
            color.setHsb(sp.segment/10.*255, 255, 255);
            Vec3b segCol = Vec3b(color.r,color.g,color.b);
            cv::Mat dst(final.size(),final.type(),segCol);
            dst.copyTo(final, sp.mask);
        }
    }
    
    ofxCv::drawMat(final,0,0,512,512);

}

//void Segmentator::drawSelection(){
//    ofxCv::drawMat(result,0,0,512,512);
//    ofxCv::drawMat(coloredSP,512,0,512,512);
//    ofxCv::drawMat(final,1024,0,512,512);
//
//}


void Segmentator::slic(cv::Mat inputMat){
    
    if(doUpdate){
        Mat frame;
        inputMat.copyTo(frame);
        
        filters(frame, frame);
        
        Mat converted;
        cvtColor(frame, converted, COLOR_BGR2HSV);
        
        double t = (double) getTickCount();
        
        Ptr<cv::ximgproc::SuperpixelSLIC> slic = cv::ximgproc::createSuperpixelSLIC(converted,algorithm+100,region_size,float(ruler));
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
        Mat frameHSV;
        cvtColor(frame, frameHSV, CV_BGR2HSV);
        
        // Quantize the hue to 30 levels
        // and the saturation to 32 levels
        int hbins = 30, sbins = 32, vbins = 32;
        int histSize[] = {hbins, sbins, vbins};
        // hue varies from 0 to 179, see cvtColor
        float hranges[] = { 0, 180 };
        // saturation varies from 0 (black-gray-white) to
        // 255 (pure spectrum color)
        float sranges[] = { 0, 256 };
        float vranges[] = { 0, 256 };
        const float* ranges[] = { hranges, sranges, vranges };
        MatND hist;
        // we compute the histogram from the 0-th and 1-st channels
        int channels[] = {0, 1, 2};
        
        frame.copyTo(coloredSP);// initialise
        
        for(int i = 0; i<slic->getNumberOfSuperpixels(); i++){
            int numPixels = 0;
            Vec3b meanColor;
            int r_col = 0;
            int g_col  = 0;
            int b_col  = 0;
            Mat mask = Mat::zeros(labels.rows, labels.cols, CV_8UC1);
            glm::vec2 centroid = glm::vec2(0,0);
            bool isTopRow;
            
            for(int r=0; r<labels.rows; r++){
                for(int c=0; c<labels.cols; c++){
                    if(labels.at<int>(r,c) == i){
                        Vec3b color = frame.at<Vec3b>(r,c);
                        r_col += color.val[0];
                        g_col += color.val[1];
                        b_col += color.val[2];
                        
                        centroid.x += r;
                        centroid.y += c;
                        
                        numPixels++;
                        
                        mask.at<uchar>(r,c) = 255;
                        
                        if(r==0) isTopRow = true;
                        
                    }
                }
            }
            meanColor.val[0] = r_col/numPixels;
            meanColor.val[1] = g_col/numPixels;
            meanColor.val[2] = b_col/numPixels;
            
            centroid = centroid/numPixels;
            
            calcHist( &frameHSV, 1, channels, mask, hist, 2, histSize, ranges, true, false );
            
            SuperPixel newSP;
            newSP.label = i;
            newSP.meanColor = meanColor;
            newSP.mask = mask;
            hist.copyTo(newSP.hist);
            newSP.centroids.push_back(centroid);
//            if(isTopRow) newSP.segment = 1;
//            else newSP.segment = -1;
            superPixels.push_back(newSP);
            
            
            for(int r=0; r<labels.rows; r++){
                for(int c=0; c<labels.cols; c++){
                    if(labels.at<int>(r,c) == i){
                        coloredSP.at<Vec3b>(r,c) = meanColor;
                    }
                }
            }
            
        }
        
//        for( auto i = superPixels.begin(); i != superPixels.end(); )
//        {
//            for( auto i2 = i+1; i2 != superPixels.end(); )
//            {
//                //                    if(sp1.label == sp2.label) continue;
//                /*
//                 compare methods
//                 CV_COMP_CORREL Correlation
//                 CV_COMP_CHISQR Chi-Square
//                 CV_COMP_INTERSECT Intersection
//                 CV_COMP_BHATTACHARYYA Bhattacharyya distance
//                 CV_COMP_HELLINGER Synonym for CV_COMP_BHATTACHARYYA
//                 */
//
//                double histCompaire = compareHist(i->hist, i2->hist,CV_COMP_CORREL);
//
//                float minDistance = 1024;
//                for(auto & c : i2->centroids){
//                    float dist = i->getMinDist(c);
//                    if(dist < minDistance) minDistance = dist;
//                }
//                if( histCompaire > combine_treshold  && minDistance<maxDistance){
//                    //                    cout << i->label<< "/" << i2->label<<" "<<histCompaire << endl;
//                    //                    if((abs(i->meanColor[0]-i2->meanColor[0])<(1.-combine_treshold)*255)  && glm::distance(i->centroid, i2->centroid)<maxDistance){
//                    bitwise_or(i->mask, i2->mask, i->mask);
//                    i->meanColor = (i->meanColor+i2->meanColor)/2;
//
//                    calcHist( &frameHSV, 1, channels, i->mask, i->hist, 2, histSize, ranges, true, false );
//
//                    for(auto & c : i2->centroids) i->centroids.push_back(c);
//
//
//                    i2 = superPixels.erase(i2);
//                }
//                else ++i2;
//            }
//            ++i;
//        }
//
//
//        for(auto & sp: superPixels){
//            // create dst with background color of your choice
//            cv::Mat dst(frame.size(),frame.type(),sp.meanColor);
//
//            // now copy
//            dst.copyTo(final, sp.mask);
//        }
        
        doUpdate = false;
    }
}

// ---------------------------------------

void Segmentator::slicGray(cv::Mat inputMat){
    
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
            if(isTopRow) newSP.segment = 1;
            else newSP.segment = -1;
            superPixels.push_back(newSP);
            
            
            for(int r=0; r<labels.rows; r++){
                for(int c=0; c<labels.cols; c++){
                    if(labels.at<int>(r,c) == i){
                        coloredSP.at<Vec3b>(r,c) = meanColor;
                    }
                }
            }
            
        }
        
//        for(int j = 0; j<5; j++){
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
//
//
//                        //                    if((abs(i->meanColor[0]-i2->meanColor[0])<(combine_treshold)*255 && glm::distance(i->centroid, i2->centroid)<maxDistance) ){
//                        bitwise_or(i->mask, i2->mask, i->mask);
//
//                        int value = (i->meanColor.val[0]+i2->meanColor.val[0])/2;
//
//                        i->meanColor.val[0] = value;
//                        i->meanColor.val[1] = value;
//                        i->meanColor.val[2] = value;
//                        for(auto & c : i2->centroids) i->centroids.push_back(c);
//
//
//                        calcHist( &frame, 1, channels, i->mask, i->hist, 1, histSize, ranges, true, false );
//
//                        i2 = superPixels.erase(i2);
//                    }
//                    else ++i2;
//                }
//                ++i;
//            }
//        }
//        for( auto i = superPixels.begin(); i != superPixels.end(); )
//        {
//            for( auto i2 = i+1; i2 != superPixels.end(); )
//            {
//
//                if(i->isTopRow && i2->isTopRow){
//                    bitwise_or(i->mask, i2->mask, i->mask);
//
//                    int value = (i->meanColor.val[0]+i2->meanColor.val[0])/2;
//
//                    i->meanColor.val[0] = value;
//                    i->meanColor.val[1] = value;
//                    i->meanColor.val[2] = value;
//
//                    for(auto & c : i2->centroids) i->centroids.push_back(c);
//
//
//                    calcHist( &frame, 1, channels, i->mask, i->hist, 1, histSize, ranges, true, false );
//
//                    i2 = superPixels.erase(i2);
//                }
//                else ++i2;
//            }
//            ++i;
//        }
//
//
//        for(auto & sp: superPixels){
//            if(sp.isTopRow){
//                Mat invMask;
//                invert(sp.mask, invMask);
//                sp.contour.findContours(invMask);
//            }
//            else sp.contour.findContours(sp.mask);
//
//            // create dst with background color of your choice
//            cv::Mat dst(frame.size(),frame.type(),sp.meanColor);
//
//            // now copy
//            dst.copyTo(final, sp.mask);
//        }
//
        doUpdate = false;
    }
    
}

void Segmentator::manualSelect(){
    if(ofGetMousePressed()){
        int r = int(ofGetMouseY()*2);
        int c = int(ofGetMouseX()*0.666666);
//        cout << r << "/" << c << endl;
        
        for(auto & sp:superPixels){

//            cout << sp.mask.rows << "/" << sp.mask.cols << endl;

            if( r<1024 && c<1024 && r>=0 && c>=0){
            Vec3b col = sp.mask.at<Vec3b>(cv::Point(c,r));
                if(col.val[0] > 0 && col.val[1] > 0 && col.val[2] > 0){
                    if(ofGetKeyPressed('0')){
                        sp.segment = -1;
                    }
                    if(ofGetKeyPressed('m')){
                        sp.segment = 0;
                    }
                    if(ofGetKeyPressed('1')){
                        sp.segment = 1;
                    }
                    if(ofGetKeyPressed('2')){
                        sp.segment = 2;
                    }
                    if(ofGetKeyPressed('3')){
                        sp.segment = 3;
                    }
                    if(ofGetKeyPressed('4')){
                        sp.segment = 4;
                    }
                    if(ofGetKeyPressed('5')){
                        sp.segment = 5;
                    }
                    if(ofGetKeyPressed('6')){
                        sp.segment = 6;
                    }
                    if(ofGetKeyPressed('7')){
                        sp.segment = 7;
                    }
                    if(ofGetKeyPressed('8')){
                        sp.segment = 8;
                    }
                    if(ofGetKeyPressed('9')){
                        sp.segment = 9;
                    }
                }
            }
        }
    }
}

void Segmentator::groupSegmentSuperPixels(){
    for( auto i = superPixels.begin(); i != superPixels.end(); )
    {
        for( auto i2 = i+1; i2 != superPixels.end(); )
        {
            
            if(i->segment == i2->segment && i->segment!=-1){
                bitwise_or(i->mask, i2->mask, i->mask);
                
                for(auto & c : i2->centroids) i->centroids.push_back(c);
                
                i2 = superPixels.erase(i2);
            }
            else ++i2;
        }
        ++i;
    }
//    
//    
//    for(auto & sp: superPixels){
//        if(sp.isTopRow){
//            Mat invMask;
//            invert(sp.mask, invMask);
//            sp.contour.findContours(invMask);
//        }
//        else sp.contour.findContours(sp.mask);
//    }
}
