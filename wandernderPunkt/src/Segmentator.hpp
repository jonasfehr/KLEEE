//
//  Segmentator.hpp
//  wandernderPunkt
//
//  Created by Jonas Fehr on 03/09/2018.
//

#ifndef Segmentator_hpp
#define Segmentator_hpp

#include "ofxGui.h"
#include "ofxCv.h"
#include "SuperPixel.hpp"

#include <opencv2/ximgproc.hpp>

class Segmentator{
public:
    
    Segmentator();
    
    void listenerFunction(ofAbstractParameter& e);
    void slicGray(cv::Mat inputMat);
    void slic(cv::Mat inputMat);
    void filters(cv::Mat & src, cv::Mat & dst);
    void draw();
    
    vector<SuperPixel> superPixels;
    
    cv::Mat result, coloredSP, final;
    bool doUpdate;
    
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
    
//    cv::Mat correctGamma( cv::Mat& img, double gamma ) {
//        double inverse_gamma = 1.0 / gamma;
//
//        cv::Mat lut_matrix(1, 256, CV_8UC1 );
//        uchar * ptr = lut_matrix.ptr();
//        for( int i = 0; i < 256; i++ )
//            ptr[i] = (int)( pow( (double) i / 255.0, inverse_gamma ) * 255.0 );
//
//        cv::Mat result;
//        LUT( img, lut_matrix, result );
//
//        return result;
//    }
//
//    Scalar hsv_to_rgb(Scalar c) {
//        cv::Mat in(1, 1, CV_32FC3);
//        cv::Mat out(1, 1, CV_32FC3);
//
//        float * p = in.ptr<float>(0);
//
//        p[0] = (float)c[0] * 360.0f;
//        p[1] = (float)c[1];
//        p[2] = (float)c[2];
//
//        cvtColor(in, out, COLOR_HSV2RGB);
//
//        Scalar t;
//
//        Vec3f p2 = out.at<Vec3f>(0, 0);
//
//        t[0] = (int)(p2[0] * 255);
//        t[1] = (int)(p2[1] * 255);
//        t[2] = (int)(p2[2] * 255);
//
//        return t;
//
//    }
//
//    Scalar color_mapping(int segment_id) {
//
//        double base = (double)(segment_id) * 0.618033988749895 + 0.24443434;
//
//        return hsv_to_rgb(Scalar(fmod(base, 1.2), 0.95, 0.80));
//
//    }
    
};
#endif /* Segmentator_hpp */
