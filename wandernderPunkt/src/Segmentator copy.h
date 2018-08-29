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
        ofAddListener(parametersSuperpixels.parameterChangedE(), this, &Segmentator::listenerFunction);
    }
    
    void listenerFunction(ofAbstractParameter& e){
        init = false;
    }
    
    TextureColorSegmentation texColSeg;
    
    void textureColorSegmentation( Mat & inputMat){
                Mat frame;
                inputMat.copyTo(frame);
        
//                filters(frame, frame);
        
        texColSeg.hist(frame);
        //imwrite("originalImage.jpg",image);
        
        
        //resize(image, image, Size(320,480));
        
        ///namedWindow("image",2);
        ///imshow("image",image);
        
        int winSize=20;
        //pair<Mat, vector<pair<pair<Point, Vec3b> ,int > > >retVal=
        Mat combined = texColSeg.colSeg(frame, winSize);
        texColSeg.crop(combined, frame);
        
        result = frame;

    }
    
//    void spectralSegment( Mat& inputMat ) {
//
//        Mat frame;
//        inputMat.copyTo(frame);
//
//        filters(frame, frame);
//
//        /* First generate SLIC superpixels */
//        SLICSuperpixel slic( frame, no_of_superpixels );
//        slic.generateSuperPixels();
//
//
//        /* Perform spectral clusterings on the 400 superpixel cluster centers */
//        SuperpixelSegmentation segmenter( frame.size(), sigma_SpectralSegment );
//
//        vector<ColorRep> centers = slic.getCenters();
//        Mat clusters_index = slic.getClustersIndex();
//
//        segmenter.calculateEigenvectors( centers, slic.getS(), slic.getM() );
//        Mat mask = segmenter.applySegmentation( k_clusters, clusters_index );
//
//
//
//
//        /* Draw original image, and then all the segmented ones */
//        frame.copyTo( result);
//
//
//        /* Draw the superpixel clusters */
//        Mat slic_contour = slic.recolor();
//        cvtColor( slic_contour, slic_contour, CV_Lab2BGR );
//
//        vector<Point2i> contours = slic.getContours();
//        for( Point2i contour: contours )
//            slic_contour.at<Vec3b>( contour.y, contour.x ) = Vec3b(255, 0, 255);
//
//        slic_contour.copyTo( mask );
//
////
////        /* Finally draw the segmented image */
////        char temp[255];
////        for( int k = 0; k < k_clusters; k++ ) {
////            Rect region = regions[k+2];
////            image.copyTo( Mat(appended, region), mask == k );
////            sprintf( temp, "Segment [%d]", k + 1 );
////            addText( appended, temp, Point( region.x + 30, region.y + 30 ), font );
////        }
////
////
////        /* Uhh, try to fit my screen */
////        Size screen_size(1366 * 0.8, 768 * 0.8);
////        float resize_ratio = MIN( 1.0 * screen_size.width / appended.cols, 1.0 * screen_size.height / appended.rows  );
////
////        resize( appended, appended, Size(), resize_ratio, resize_ratio );
////        imshow( "", appended );
////        while( waitKey(10) != 'q' );
//    }
    
    void graphSegmentation(cv::Mat inputMat){
        Ptr<cv::ximgproc::segmentation::GraphSegmentation> gs = cv::ximgproc::segmentation::createGraphSegmentation();
        
            gs->setSigma(sigma_GraphSegment);
        
            gs->setK(k_GraphSegment);
        
            gs->setMinSize(minSize_GraphSegment);
        

        
        Mat output;
        
        Mat frame;
        inputMat.copyTo(frame);
        
        filters(frame, frame);
        
        
        gs->processImage(frame, output);
        
        double min, max;
        minMaxLoc(output, &min, &max);
        
        int nb_segs = (int)max + 1;
        
        std::cout << nb_segs << " segments" << std::endl;
        
        result = Mat::zeros(output.rows, output.cols, CV_8UC3);
        
        uint* p;
        uchar* p2;
        
        for (int i = 0; i < output.rows; i++) {
            
            p = output.ptr<uint>(i);
            p2 = result.ptr<uchar>(i);
            
            for (int j = 0; j < output.cols; j++) {
                Scalar color = color_mapping(p[j]);
                p2[j*3] = (uchar)color[0];
                p2[j*3 + 1] = (uchar)color[1];
                p2[j*3 + 2] = (uchar)color[2];
            }
        }
    }
    
    void selectiveSearch(cv::Mat inputMat){
        Mat frame;
        inputMat.copyTo(frame);
        
        filters(frame, frame);
        
        Ptr<cv::ximgproc::segmentation::SelectiveSearchSegmentation> gs = cv::ximgproc::segmentation::createSelectiveSearchSegmentation();
        gs->setBaseImage(frame);
        
        switch(strategy){
            case 0:
                gs->switchToSingleStrategy();
                break;
                
            case 1:
                gs->switchToSelectiveSearchFast();
                break;
            
            case 2:
                gs->switchToSelectiveSearchQuality();
            break;
                
            default:
                break;
        }

        
        std::vector<cv::Rect> rects;
        gs->process(rects);
        
        
        
        
        result = frame.clone();
            
            int i = 0;
            
        for(std::vector<cv::Rect>::iterator it = rects.begin(); it != rects.end(); ++it) {
                if (i++ < nb_rects) {
                    rectangle(result, *it, Scalar(0, 0, 255));
                }
            }
            
        

        
    }

    
    

    void slic(cv::Mat inputMat){
        
        Mat frame;
        inputMat.copyTo(frame);
        
        filters(frame, frame);
        
        result = frame;
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
        slic->getLabelContourMask(mask, true);
        result.setTo(Scalar(0, 0, 255), mask);
        
        Mat labels;
        slic->getLabels(labels);
        const int num_label_bits = 4;
        labels &= (1 << num_label_bits) - 1;
        labels *= 1 << (16 - num_label_bits);
//        imshow("test", labels);
        
//        cvtColor(labels, this->labels, COLOR_GRAY2RGB);

//        labels.copyTo(this->labels);

        
    }
    
    void superPixel(cv::Mat inputMat){
        
        Mat frame;
        inputMat.copyTo(frame);
        

        if(!init){
            width = frame.size().width;
            height = frame.size().height;
        seeds = cv::ximgproc::createSuperpixelSEEDS(width, height, frame.channels(), num_superpixels, num_levels, prior, num_histogram_bins, double_step);
            init = true;
        }

        Mat converted;
        cvtColor(frame, converted, COLOR_BGR2HSV);
        
        double t = (double) getTickCount();
        
        seeds->iterate(converted, num_iterations);
        result = frame;
        t = ((double) getTickCount() - t) / getTickFrequency();
        printf("SEEDS segmentation took %i ms with %3i superpixels\n",
               (int) (t * 1000), seeds->getNumberOfSuperpixels());
        
        /* retrieve the segmentation result */
        seeds->getLabels(labels);
        
//        const int num_label_bits = 2;
//        labels &= (1 << num_label_bits) - 1;
//        labels *= 1 << (16 - num_label_bits);
        
        /* get the contours for displaying */
        seeds->getLabelContourMask(mask, false);
        result.setTo(Scalar(0, 0, 255), mask);
        
    }
    
    void filters(Mat & src, Mat & dst){
        // flags RECURS_FILTER = 1 NORMCONV_FILTER = 2
        if(doEdgePreservingFiltering) edgePreservingFilter(src, dst, 1, sigma_sEPF, sigma_rEPF);
        if(doStylisation) stylization(src, dst, sigma_sS, sigma_rS);
        if(doAnistriopicDiffusion)ximgproc::anisotropicDiffusion(src, dst, alpha, k, niters);


    }
    
    void draw(){
        ofxCv::drawMat(result,0,0,512,512);
        ofxCv::drawMat(mask,512,0,512,512);
        ofxCv::drawMat(labels,1024,0,512,512);

    }
    
    ofParameter<int> num_superpixels{"Number of Superpixels", 400, 10, 1000};
    ofParameter<int> prior{"Smoothing Prior", 2, 1, 5};
    ofParameter<int> num_levels{"Number of Levels", 4, 1, 10};
    ofParameter<int> num_iterations{"Iterations", 3, 12, 30};

    bool double_step = false;
    int num_histogram_bins = 5;
    
    Mat result, mask,labels;
    int width, height;
    int display_mode = 0;
    bool init;
    
    Ptr<cv::ximgproc::SuperpixelSEEDS> seeds;
    ofParameterGroup parametersSuperpixels{"Superpixel", num_superpixels, prior, num_levels, num_iterations};

    ofParameter<bool> doEdgePreservingFiltering{"do EdgePreservingFiltering", false};
    ofParameter<int> sigma_sEPF{"sigma_s", 60, 0, 200};
    ofParameter<float> sigma_rEPF{"sigma_r", 0.6, 0, 1};
    ofParameter<bool> doStylisation{"do Stylisation", false};
    ofParameter<int> sigma_sS{"sigma_s", 60, 0, 200};
    ofParameter<float> sigma_rS{"sigma_r", 0.6, 0, 1};
    ofParameter<bool> doAnistriopicDiffusion{"do Stylisation", true};
    ofParameter<float> alpha{"alpha", 0.2, 0, 1};
    ofParameter<int> k{"k", 10, 1, 30};
    ofParameter<int> niters{"iterations", 4, 1, 10};

    ofParameterGroup parametersFilters{"Filter", doEdgePreservingFiltering,sigma_sEPF, sigma_rEPF, doStylisation, sigma_sS, sigma_rS,doAnistriopicDiffusion,alpha,k, niters };



    ofParameter<int> algorithm{"Algorithm", 0, 0, 2};
    ofParameter<int> region_size{"Region size", 50, 1, 200};
    ofParameter<int> ruler{"Ruler", 30, 1, 100};
    ofParameter<int> min_element_size{"Connectivity", 50, 1, 100};
    ofParameterGroup parametersSLIC{"SLIC", algorithm, region_size, ruler, min_element_size, num_iterations};

    ofParameter<int> strategy{"strategy", 0, 0, 2};
    ofParameter<int> nb_rects{"nb_rects", 10, 1, 100};
    ofParameterGroup parametersSelectiveSearch{"SelectivSearch", strategy, nb_rects};
    
    ofParameter<float> sigma_GraphSegment{"sigma", 0.5, 0, 1};
    ofParameter<float> k_GraphSegment{"k", 300, 1, 1500};
    ofParameter<int> minSize_GraphSegment{"min Size", 50, 1, 500};

    ofParameterGroup parametersGraphSegmentation{"GraphSegment", sigma_GraphSegment, k_GraphSegment, minSize_GraphSegment};
    
    
//    ofParameter<int> no_of_superpixels{"no_of_superpixels", 200, 1, 500};
//    ofParameter<int> k_clusters{"k_clusters", 4, 1, 20};
//    ofParameter<float> sigma_SpectralSegment{"sigma", 2.0, 0., 5};
//    
//    ofParameterGroup parametersSpectralSegment{"SpectralSegment", no_of_superpixels, k_clusters, sigma_SpectralSegment};
    
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

#endif /* Segmentator_h */
