#include "top.h"
#include "opencv_top.h"


int main (int argc, char** argv) {

    // Load input image
    IplImage* src = cvLoadImage(INPUT_IMAGE);
	// The open_CV image will be 3 channel RGB
    IplImage* dst_opencv = cvCreateImage(cvGetSize(src), src->depth, src->nChannels);
	// The HLS image will be 1 channel grayscale
    IplImage* dst_hls = cvCreateImage(cvGetSize(src), src->depth, 3);

	// Process the input image using the HLS implementation
    AXI_STREAM  src_axi, dst_axi;
    IplImage2AXIvideo(src, src_axi);
    image_filter(src_axi, dst_axi, src->height, src->width);
    AXIvideo2IplImage(dst_axi, dst_hls);
    cvSaveImage(OUTPUT_IMAGE, dst_hls);
    
	// Process the input image using the OpenCV implementation
    opencv_image_filter(src, dst_opencv);
    cvSaveImage(OUTPUT_IMAGE_GOLDEN, dst_opencv);

    cvReleaseImage(&src);
    cvReleaseImage(&dst_hls);
    cvReleaseImage(&dst_opencv);

	// Compare the resulting images (they will differ)
    char tempbuf[2000];
    sprintf(tempbuf, "diff --brief -w %s %s", OUTPUT_IMAGE, OUTPUT_IMAGE_GOLDEN);
    int ret = system(tempbuf);
	printf("Outputs generated!\n");
    return ret;

}
