#include "ofApp.h"
using namespace cv;
using namespace ofxCv;
//-------------------------------------------------------------- setup();
void ofApp::setup(){
    cout << cv::getBuildInformation() << endl;

    ofSetVerticalSync(true);
	ofSetCircleResolution(80);
	ofBackground(54, 54, 54);

//    gui.setup("SEGMENTATION PARAMETERS");
//    gui.add(sigma.setup("SIGMA", segmentation.sigma, 0, 2.0));
//    gui.add(k.setup("K", segmentation.k, 0, 500));
//    gui.add(min_size.setup("MIN SIZE", segmentation.min, 0, 50));
	
//    videoInput.initGrabber(640,480);
    
    input.load("gartenBW.JPG");
    input.resize(640,480);
    
    imitate(tmp_frame, input);
    imitate(bgmask, input);
    imitate(out_frame, input);
    tmp_frame = toCv(input);
    
    imitate(img, input);
    img = toCv(input);

//    resize(img, img, cv::Size(256, 256), 0, 0, 1);
    
    // Convert color from RGB to Lab
    cvtColor(img, img, CV_RGB2Lab);
    
    // Initilize Mean Shift with spatial bandwith and color bandwith
//    MeanShift MSProc(12, 24);
    MeanShift MSProc(8, 16);
    // Filtering Process
    MSProc.MSFiltering(img);
    // Segmentation Process include Filtering Process (Region Growing)
        MSProc.MSSegmentation(img);
    
    // Print the bandwith
    cout<<"the Spatial Bandwith is "<<MSProc.hs<<endl;
    cout<<"the Color Bandwith is "<<MSProc.hr<<endl;
    
    // Convert color from Lab to RGB
    cvtColor(img, out_frame, CV_Lab2RGB);
    
    

//    //http://www.coldvision.io/2016/04/18/image-segmentation-watershed-c-opencv-3-x-cuda/
//    out_frame = watershedWithMarkersAndHistograms(tmp_frame);
//    out_frame = watershedWithMarkers(tmp_frame);
    
    
//    imitate(img, input);
//    img = toCv(input);
//
//    img_float = np.float32(img)  // Convert image from unsigned 8 bit to 32 bit float
//    criteria = (cv2.TERM_CRITERIA_EPS+cv2.TERM_CRITERIA_MAX_ITER, 10, 1)
//
//// Defining the criteria ( type, max_iter, epsilon )
//// cv2.TERM_CRITERIA_EPS - stop the algorithm iteration if specified accuracy, epsilon, is reached.
//// cv2.TERM_CRITERIA_MAX_ITER - stop the algorithm after the specified number of iterations, max_iter.
//// cv2.TERM_CRITERIA_EPS + cv2.TERM_CRITERIA_MAX_ITER - stop the iteration when any of the above condition is met.
//// max_iter - An integer specifying maximum number of iterations.In this case it is 10
//// epsilon - Required accuracy.In this case it is 1
//    int k = 50  // Number of clusters
////    ret, label, centers = cv2.kmeans(img_float, k, None, criteria, 50, cv2.KMEANS_RANDOM_CENTERS)
//
//    Mat label;
//
//    int origRows = img.rows;
//    Mat colVec = img.reshape(1, img.rows*img.cols); // change to a Nx3 column vector
//    Mat colVecD, bestLabels, centers, clustered;
//
//    colVec.convertTo(colVecD, CV_32FC3, 1.0/255.0); // convert to floating point
//
//    kmeans(colVecD, k, label, TermCriteria(TermCriteria::EPS+TermCriteria::MAX_ITER, 10, 1), 10, 1, centers );
//    Mat labelsImg = bestLabels.reshape(1, origRows); // single channel image of labels
//
//    // apply kmeans algorithm with random centers approach
//    center = np.uint8(centers)
//// Convert the image from float to unsigned integer
//    res = center[label.flatten()]
//// This will flatten the label
//    res2 = res.reshape(img.shape)
//// Reshape the image
//    cv2.imshow("K Means", res2)  // Display image
//    cv2.imwrite("1.jpg", res2)  // Write image onto disk
//    meanshift = cv2.pyrMeanShiftFiltering(img, sp=8, sr=16, maxLevel=1, termcrit=(cv2.TERM_CRITERIA_EPS+cv2.TERM_CRITERIA_MAX_ITER, 5, 1))
//// Apply meanshift algorithm on to image
////    cv2.imshow("Output of meanshift", meanshift)
//// Display image
////    cv2.imwrite("2.jpg", meanshift)
//// Write image onto disk
//    Mat gray = cvtColor(img, COLOR_BGR2GRAY)
//// Convert image from RGB to GRAY
//    ret, thresh = cv2.threshold(gray, 0, 255, cv2.THRESH_BINARY+cv2.THRESH_OTSU)
//// apply thresholding to convert the image to binary
//    fg = cv2.erode(thresh, None, iterations=1)
//// erode the image
//    bgt = cv2.dilate(thresh, None, iterations=1)
//// Dilate the image
//    ret, bg = cv2.threshold(bgt, 1, 128, 1)
//// Apply thresholding
//    marker = cv2.add(fg, bg)
//// Add foreground and background
//    canny = cv2.Canny(marker, 110, 150)
//// Apply canny edge detector
//    new, contours, hierarchy = cv2.findContours(canny, cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)
//// Finding the contors in the image using chain approximation
//    marker32 = np.int32(marker)
//// converting the marker to float 32 bit
//    cv2.watershed(img,marker32)
//// Apply watershed algorithm
//    m = cv2.convertScaleAbs(marker32)
//    ret, thresh = cv2.threshold(m, 0, 255, cv2.THRESH_BINARY+cv2.THRESH_OTSU)
//// Apply thresholding on the image to convert to binary image
//    thresh_inv = cv2.bitwise_not(thresh)
//// Invert the thresh
//    res = cv2.bitwise_and(img, img, mask=thresh)
//// Bitwise and with the image mask thresh
//    res3 = cv2.bitwise_and(img, img, mask=thresh_inv)
//// Bitwise and the image with mask as threshold invert
//    res4 = cv2.addWeighted(res, 1, res3, 1, 0)
//// Take the weighted average
//    final = cv2.drawContours(res4, contours, -1, (0, 255, 0), 1)
//// Draw the contours on the image with green color and pixel width is 1
//    cv2.imshow("Watershed", final)  // Display the image
//    cv2.imwrite("3.jpg", final)  // Write the image
//    cv2.waitKey()  // Wait for key stroke
//
//
    
}
    
//    {
//    Mat1b gray;
//    cvtColor(tmp_frame, gray, COLOR_BGR2GRAY);
//
//    Mat1b thresh;
//    threshold(gray, thresh, 0, 255, THRESH_BINARY | THRESH_OTSU);
//
//    // noise removal
//    Mat1b kernel = getStructuringElement(MORPH_RECT, cv::Size(3,3));
//    Mat1b opening;
//    morphologyEx(thresh, opening, MORPH_OPEN, kernel, cv::Point(-1, -1), 2);
//
//    Mat1b kernelb = getStructuringElement(MORPH_RECT, cv::Size(21, 21));
//    Mat1b background;
//    morphologyEx(thresh, background, MORPH_DILATE, kernelb);
//    background = ~background;
//
//    // Perform the distance transform algorithm
//    Mat1f dist_transform;
//    distanceTransform(opening, dist_transform, CV_DIST_L2, 5);
//
//    // Normalize the distance image for range = {0.0, 1.0}
//    // so we can visualize and threshold it
//    normalize(dist_transform, dist_transform, 0, 1., NORM_MINMAX);
//
//    // Threshold to obtain the peaks
//    // This will be the markers for the foreground objects
//    Mat1f dist_thresh;
//    threshold(dist_transform, dist_thresh, 0.5, 1., CV_THRESH_BINARY);
//
//    Mat1b dist_8u;
//    dist_thresh.convertTo(dist_8u, CV_8U);
//
//    // Find total markers
//    vector<vector<cv::Point> > contours;
//    findContours(dist_8u, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
//
//    // Create the marker image for the watershed algorithm
//    Mat1i markers(dist_thresh.rows, dist_thresh.cols, int(0));
//
//    // Background as 1
//    Mat1i one(markers.rows, markers.cols, int(1));
//    bitwise_or(one, markers, markers, background);
//
//    // Draw the foreground markers (from 2 up)
//    for (int i = 0; i < int(contours.size()); i++)
//    drawContours(markers, contours, i, Scalar(i+2), -1);
//
//    // Perform the watershed algorithm
//    Mat3b dbg;
//    cvtColor(opening, dbg, COLOR_GRAY2BGR);
//    watershed(dbg, markers);
//
//    Mat res;
//    markers.convertTo(res, CV_8U);
//    normalize(res, res, 0, 255, NORM_MINMAX);
//
//    out_frame = res;
//
//}

//-------------------------------------------------------------- update();
void ofApp::update(){

//    segmentation.sigma = sigma;
//    segmentation.k = k;
//    segmentation.min = min_size;
//    segmentation.segment(input);
//    segmentedImage.setFromPixels(segmentation.getSegmentedPixels());
//    segmentedImage.update();
//

//    Ptr<BackgroundSubtractorMOG2> bgsubtractor=createBackgroundSubtractorMOG2();
//    bgsubtractor->setVarThreshold(10);
//
//    bgsubtractor->apply(tmp_frame, bgmask, -1);
//    refineSegments(tmp_frame, bgmask, out_frame);
}

//-------------------------------------------------------------- draw();
void ofApp::draw(){
//    videoInput.draw(0,0);
//    input.draw(0,0);
//    if(segmentedImage.isAllocated()){
//        segmentedImage.draw(videoInput.getWidth(),0);
//        //draw all the little masks below
//        ofImage image;
//        for(int i = 0; i < segmentation.numSegments; i++){
//            image.setFromPixels(segmentation.getSegmentMask(i));
//            image.update();
//            image.draw(i*160,240,160,120);
//        }
//    }

    input.update();
    input.draw(0,0);
    
    drawMat(out_frame, input.getWidth(), 0);
    
//    gui.draw();
}

//-------------------------------------------------------------- KeyBoard Events
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}
