#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "ofxImageSegmentation.h"
#include "ofxCv.h"
#include <iostream>
#include "MeanShift.h"

using namespace cv;
using namespace ofxCv;
class ofApp : public ofBaseApp{

  public:
	void setup();
	void update();
	void draw();

	void keyPressed(int key);
	void keyReleased(int key);
	
	ofVideoGrabber videoInput;
	ofxImageSegmentation segmentation;
	ofImage segmentedImage;

	ofxPanel gui;
	ofxSlider<float> sigma;
	ofxSlider<float> k;
	ofxSlider<int> min_size;
    
    ofImage input;
    
    Mat tmp_frame, bgmask, out_frame, img, Img;

    
    static void refineSegments(const Mat& img, Mat& mask, Mat& dst)
    {
        int niters = 3;
        vector<vector<cv::Point> > contours;
        vector<Vec4i> hierarchy;
        Mat temp;
        dilate(mask, temp, Mat(), cv::Point(-1,-1), niters);
        erode(temp, temp, Mat(), cv::Point(-1,-1), niters*2);
        dilate(temp, temp, Mat(), cv::Point(-1,-1), niters);
        findContours( temp, contours, hierarchy, RETR_CCOMP, CHAIN_APPROX_SIMPLE );
        dst = Mat::zeros(img.size(), CV_8UC3);
        if( contours.size() == 0 )
        return;
        // iterate through all the top-level contours,
        // draw each connected component with its own random color
        int idx = 0, largestComp = 0;
        double maxArea = 0;
        for( ; idx >= 0; idx = hierarchy[idx][0] )
        {
            const vector<cv::Point>& c = contours[idx];
            double area = fabs(contourArea(Mat(c)));
            if( area > maxArea )
            {
                maxArea = area;
                largestComp = idx;
            }
        }
        Scalar color( 0, 0, 255 );
        drawContours( dst, contours, largestComp, color, FILLED, LINE_8, hierarchy );
    }
    
    void mergeSegments(Mat & image,Mat & segments, int & numOfSegments)
    {
        //To collect pixels from each segment of the image
        vector<Mat> samples;
        //In case of multiple merging iterations, the numOfSegments should be updated
        int newNumOfSegments = numOfSegments;
        
        //Initialize the segment samples
        for(int i=0;i<=numOfSegments;i++)
        {
            Mat sampleImage;
            samples.push_back(sampleImage);
        }
        
        //collect pixels from each segments
        for(int i = 0; i < segments.rows; i++ )
        {
            for(int j = 0; j < segments.cols; j++ )
            {
                //check what segment the image pixel belongs to
                int index = segments.at<int>(i,j);
                if(index >= 0 && index<numOfSegments)
                {
                    samples[index].push_back(image(cv::Rect(j,i,1,1)));
                }
            }
        }
        
        //create histograms
        vector<MatND> hist_bases;
        Mat hsv_base;
        /// Using 35 bins for hue component
        int h_bins = 35;
        /// Using 30 bins for saturation component
        int s_bins = 30;
        int histSize[] = { h_bins,s_bins };
        
        // hue varies from 0 to 256, saturation from 0 to 180
        float h_ranges[] = { 0, 256 };
        float s_ranges[] = { 0, 180 };
        
        const float* ranges[] = { h_ranges, s_ranges };
        
        // Use the 0-th and 1-st channels
        int channels[] = { 0,1 };
        
        // To store the histograms
        MatND hist_base;
        for(int c=1;c<numOfSegments;c++)
        {
            if(samples[c].dims>0){
                //convert the sample to HSV
                cvtColor( samples[c], hsv_base, CV_BGR2HSV );
                //calculate the histogram
                calcHist( &hsv_base, 1, channels, Mat(), hist_base,2, histSize, ranges, true, false );
                //normalize the histogram
                normalize( hist_base, hist_base, 0, 1, NORM_MINMAX, -1, Mat() );
                //append to the collection
                hist_bases.push_back(hist_base);
            }else
            {
                hist_bases.push_back(MatND());
            }
            
            hist_base.release();
        }
        
        //To store the similarity of histograms
        double similarity = 0;
        
        //to keep the track of already merged segments
        vector<bool> mearged;
        
        //initialize the merged segments tracker
        for(int k = 0; k < hist_bases.size(); k++)
        {
            mearged.push_back(false);
        }
        
        //calculate the similarity of the histograms of each pair of segments
        for(int c=0;c<hist_bases.size();c++)
        {
            for(int q=c+1;q<hist_bases.size();q++)
            {
                //if the segment is not merged alreay
                if(!mearged[q])
                {
                    if(hist_bases[c].dims>0 && hist_bases[q].dims>0)
                    {
                        //calculate the histogram similarity
                        similarity = compareHist(hist_bases[c],hist_bases[q],CV_COMP_BHATTACHARYYA);
                        //if similay
                        if(similarity>0.8)
                        {
                            mearged[q]=true;
                            if(q!=c)
                            {
                                //reduce number of segments
                                newNumOfSegments--;
                                for(int i = 0; i < segments.rows; i++ )
                                {
                                    for(int j = 0; j < segments.cols; j++ )
                                    {
                                        int index = segments.at<int>(i,j);
                                        //merge the segment q with c
                                        if(index==q){
                                            segments.at<int>(i,j) = c;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        numOfSegments = newNumOfSegments;
    }
    
    Mat watershedSegment(Mat & image, int & noOfSegments)
    {
        //To store the gray version of the image
        Mat gray;
        //To store the thresholded image
        Mat ret;
        
        //convert the image to grayscale
        cvtColor(image,gray,CV_BGR2GRAY);
        //imshow("Gray Image",gray);
        
        //threshold the image
        threshold(gray,ret,0,255,CV_THRESH_BINARY_INV+CV_THRESH_OTSU);
        //imshow("Image after OTSU Thresholding",ret);
        
        //Execute morphological-open
        morphologyEx(ret,ret,MORPH_OPEN,Mat::ones(9,9,CV_8SC1),cv::Point(4,4),2);
        //imshow("Thresholded Image after Morphological open",ret);
        
        //get the distance transformation
        Mat distTransformed(ret.rows,ret.cols,CV_32FC1);
        distanceTransform(ret,distTransformed,CV_DIST_L2,3);
        
        //normalize the transformed image in order to display
        normalize(distTransformed, distTransformed, 0.0, 1, NORM_MINMAX);
        //imshow("Distance Transformation",distTransformed);
        
        //threshold the transformed image to obtain markers for watershed
        threshold(distTransformed,distTransformed,0.1,1,CV_THRESH_BINARY);
        
        //Renormalize to 0-255 to further calculations
        normalize(distTransformed, distTransformed, 0.0, 255.0, NORM_MINMAX);
        distTransformed.convertTo(distTransformed,CV_8UC1);
        //imshow("Thresholded Distance Transformation",distTransformed);
        
        //calculate the contours of markers
        int compCount = 0;
        vector<vector<cv::Point> > contours;
        vector<Vec4i> hierarchy;
        findContours(distTransformed, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
        
        if( contours.empty() )
        return Mat();
        Mat markers(distTransformed.size(), CV_32S);
        markers = Scalar::all(0);
        int idx = 0;
        
        //draw contours
        for( ; idx >= 0; idx = hierarchy[idx][0], compCount++ )
        drawContours(markers, contours, idx, Scalar::all(compCount+1), -1, 8, hierarchy, INT_MAX);
        
        if( compCount == 0 )
        return Mat();
        
        //calculate the time taken to the watershed algorithm
        double t = (double)getTickCount();
        
        //apply watershed with the markers as seeds
        watershed( image, markers );
        t = (double)getTickCount() - t;
        //printf( "execution time = %gms\n", t*1000./getTickFrequency() );
        
        //create displayable image of segments
        //Mat mat;
        //Mat wshed = createSegmentationDisplay(markers,compCount, mat);
        
        //imshow( "watershed transform", wshed );
        noOfSegments = compCount;
        
        //returns the segments
        return markers;
    }
    
    /**
     * This is an example method showing how to use this implementation.
     *
     * @param image The original image.
     * @return wshedWithImage A merged image of the original and the segments.
     */
    Mat watershedWithMarkersAndHistograms(Mat image) {
        
        //to store the number of segments
        int numOfSegments = 0;
        
        //Apply watershed
        Mat segments = watershedSegment(image, numOfSegments);
        
        //Merge segments in order to reduce over segmentation
        mergeSegments(image, segments, numOfSegments);
        
        //To display the merged segments
        //Mat mat (segments);
        //Mat wshed = createSegmentationDisplay(segments, numOfSegments, image);
        
        //To display the merged segments blended with the image
        Mat wshedWithImage = createSegmentationDisplay(segments, numOfSegments, image);
        
        //Display the merged segments
        //imshow("Merged segments",wshed);
        //Display the merged segments blended with the image
        //imshow("Merged segments with image",wshedWithImage);
        
        //waitKey(0);
        
        return wshedWithImage;
    }
    
    Mat createSegmentationDisplay(const Mat & segments, int numOfSegments, const Mat & image)
    {
        //create a new image
        Mat wshed(image.size(), CV_8UC3);
        
        //Create color tab for coloring the segments
        vector<Vec3b> colorTab;
        for(int i = 0; i < numOfSegments; i++ )
        {
            int b = theRNG().uniform(0, 255);
            int g = theRNG().uniform(0, 255);
            int r = theRNG().uniform(0, 255);
            colorTab.push_back(Vec3b((uchar)b, (uchar)g, (uchar)r));
        }
        
        //assign different color to different segments
        for(int i = 0; i < segments.rows; i++ )
        {
            for(int j = 0; j < segments.cols; j++ )
            {
                int index = segments.at<int>(i,j);
                
                if( index == -1 )
                wshed.at<Vec3b>(i,j) = Vec3b(255,255,255);
                else if( index <= 0 || index > numOfSegments )
                wshed.at<Vec3b>(i,j) = Vec3b(0,0,0);
                else
                wshed.at<Vec3b>(i,j) = colorTab[index - 1];
            }
        }
        Mat normWshed;
        if (image.channels()==1) {
            cvtColor(wshed, wshed, CV_BGR2GRAY);
        }
        normalize(wshed, normWshed, 0, 255, NORM_MINMAX, image.type());
        
        //cout << "M = "<< endl << " "  << normWshed << endl << endl;
        //cout << "M = "<< endl << " "  << image.type() << endl << endl;
        //cout << "M = "<< endl << " "  << normWshed.type() << endl << endl;
        
        addWeighted(normWshed, 0.3, image, 0.7, 0, normWshed);
        return normWshed;
    }
    
    Mat watershedWithMarkers(Mat input) {
        
        // Change the background from white to black, since that will help later to extract
        // better results during the use of Distance Transform
        for( int x = 0; x < input.rows; x++ ) {
            for( int y = 0; y < input.cols; y++ ) {
                if ( input.at<Vec3b>(x, y) == Vec3b(255,255,255) ) {
                    input.at<Vec3b>(x, y)[0] = 0;
                    input.at<Vec3b>(x, y)[1] = 0;
                    input.at<Vec3b>(x, y)[2] = 0;
                }
            }
        }
        // Show output image
        //imshow("Black Background Image", input);
        
        // Create a kernel that we will use for accuting/sharpening our image
        Mat kernel = (Mat_<float>(3,3) <<
                      1,  1, 1,
                      1, -8, 1,
                      1,  1, 1); // an approximation of second derivative, a quite strong kernel
        
        // do the laplacian filtering as it is
        // well, we need to convert everything in something more deeper then CV_8U
        // because the kernel has some negative values,
        // and we can expect in general to have a Laplacian image with negative values
        // BUT a 8bits unsigned int (the one we are working with) can contain values from 0 to 255
        // so the possible negative number will be truncated
        Mat imgLaplacian;
        Mat sharp = input; // copy source image to another temporary one
        filter2D(sharp, imgLaplacian, CV_32F, kernel);
        input.convertTo(sharp, CV_32F);
        Mat imgResult = sharp - imgLaplacian;
        // convert back to 8bits gray scale
        imgResult.convertTo(imgResult, CV_8UC3);
        imgLaplacian.convertTo(imgLaplacian, CV_8UC3);
        // imshow( "Laplace Filtered Image", imgLaplacian );
        //imshow( "New Sharped Image", imgResult );
        
        input = imgResult; // copy back
        // Create binary image from source image
        Mat bw;
        cvtColor(input, bw, CV_BGR2GRAY);
        threshold(bw, bw, 40, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
        //imshow("Binary Image", bw);
        
        // Perform the distance transform algorithm
        Mat dist;
        distanceTransform(bw, dist, CV_DIST_L2, 3);
        // Normalize the distance image for range = {0.0, 1.0}
        // so we can visualize and threshold it
        normalize(dist, dist, 0, 1., NORM_MINMAX);
        //imshow("Distance Transform Image", dist);
        
        // Threshold to obtain the peaks
        // This will be the markers for the foreground objects
        threshold(dist, dist, .4, 1., CV_THRESH_BINARY);
        // Dilate a bit the dist image
        Mat kernel1 = Mat::ones(3, 3, CV_8UC1);
        dilate(dist, dist, kernel1);
        //imshow("Peaks", dist);
        
        // Create the CV_8U version of the distance image
        // It is needed for findContours()
        Mat dist_8u;
        dist.convertTo(dist_8u, CV_8U);
        
        // Find total markers
        vector<vector<cv::Point> > contours;
        findContours(dist_8u, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
        
        // Create the marker image for the watershed algorithm
        Mat markers = Mat::zeros(dist.size(), CV_32SC1);
        
        // Draw the foreground markers
        for (size_t i = 0; i < contours.size(); i++)
        drawContours(markers, contours, static_cast<int>(i), Scalar::all(static_cast<int>(i)+1), -1);
        
        // Draw the background marker
        circle(markers, cv::Point(5,5), 3, CV_RGB(255,255,255), -1);
        //imshow("Markers", markers*10000);
        
        // Perform the watershed algorithm
        watershed(input, markers);
        Mat mark = Mat::zeros(markers.size(), CV_8UC1);
        markers.convertTo(mark, CV_8UC1);
        bitwise_not(mark, mark);
        //imshow("Markers_v2", mark); // uncomment this if you want to see how the mark image looks like at that point
        
        int numOfSegments = contours.size();
        Mat wshed = createSegmentationDisplay(markers, numOfSegments, input);
        
        return wshed;
    }

};
