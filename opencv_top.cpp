#include "opencv_top.h"
#include "top.h"
#include "highgui.hpp"
#include "imgproc.hpp"



using namespace cv;
using namespace std;




// This is doing the image filter using OpenCV code in SW
void opencv_image_filter(IplImage* src, IplImage* dst) {
    IplImage* tmp = cvCreateImage(cvGetSize(src), src->depth, src->nChannels);
	// copy the input stream
    cvCopy(src, tmp);
	// convert to a Matrix
	cv::Mat srcMat(tmp);
	cv::Mat dstMat(dst);
        //cv::Mat cdstMat(dst);


	// Convert to grayscale (1 channel)
	cvtColor(srcMat, dstMat, CV_BGR2GRAY);
	// Perform Gaussian Blur with 5x5 kernel and sigma of 1.4
	cv::GaussianBlur(dstMat, dstMat, cv::Size(5,5), 1.4, 1.4);
	// Perform Canny filtering
        Canny(dstMat, dstMat, 100, 140);
	// Convert back to RGB (3 channel)
//	cvtColor(dstMat, dstMat, CV_GRAY2RGB, src->nChannels);
       
        //Hough transform

 /*       vector<Vec2f> lines;
        HoughLines(dstMat, lines, 1, 1, 500, 0, 0 ); 
    
     for( size_t i = 0; i < lines.size(); i++ )
  {
     float rho = lines[i][0], theta = lines[i][1];
     Point pt1, pt2;
     double a = cos(theta), b = sin(theta);
     double x0 = a*rho, y0 = b*rho;
     pt1.x = cvRound(x0 + 1000*(-b));
     pt1.y = cvRound(y0 + 1000*(a));
     pt2.x = cvRound(x0 - 1000*(-b));
     pt2.y = cvRound(y0 - 1000*(a));
     line( dstMat, pt1, pt2, Scalar(255,0,0), 3, CV_AA);
  }
*/



 vector<Vec4i> lines;
  HoughLinesP(dstMat, lines, 1, CV_PI/180, 1000, 2000, 5 );
  for( size_t i = 0; i < lines.size(); i++ )
  {
    Vec4i l = lines[i];
    line( dstMat, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0,0,255), 3, CV_AA);
  }     






//	cvtColor(dstMat, dstMat, CV_GRAY2RGB, src->nChannels);  

      cvtColor(dstMat, dstMat, CV_GRAY2BGR);



   // imshow("original", srcMat);   
    //imshow("detected lines", dstMat);



	IplImage dstImg = dstMat;
	// Convert back to IplImage and output
	cvCopy(&dstImg, dst);
	cvReleaseImage(&tmp);    
}

// This is doing the image filter using HLS code in SW
void sw_image_filter(IplImage* src, IplImage* dst) {
    AXI_STREAM src_axi, dst_axi;
    IplImage2AXIvideo(src, src_axi);
    image_filter(src_axi, dst_axi, src->height, src->width);
    AXIvideo2IplImage(dst_axi, dst);
}
