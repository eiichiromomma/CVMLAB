/***********************************************************************
 MultiViewDemo.cpp : This program shows various results at once.
 OpenCV Demo program for RSJ2011
 E.Momma and T.Minagawa

 This demo process input camera image as follows, and show these results 
 1. Original
 2. HSV
 3. Gray Scale
 4. Binarization
 5. Edge detection
 6. Morphology
*************************************************************************/
#include "opencv2/opencv.hpp"

using namespace std;

int main(int argc, char** argv)
{
	cv::VideoCapture	capture( 0 );

	if( !capture.isOpened() ) {
		std::cout << "Failed to Open Camera" << std::endl;
		return -1;
	}

	// create window
	const string		windowNameCam = "Camera Image";
	cv::namedWindow( windowNameCam, CV_WINDOW_AUTOSIZE );

	// capture an image from camera
	cv::Mat	frame;
	capture >> frame;

	// create display image to show result
	int width = frame.cols * 3 / 2;
	int height = frame.rows;
	cv::Mat view_image(height, width, CV_8UC3);
	float ratio = 0.5;

	int small_width = width / 3;
	int small_height = height / 2;

	// divide result display image to 6 areas
	cv::Mat roiImage1(view_image, cv::Rect(0, 0, small_width, small_height));
	cv::Mat roiImage2(view_image, cv::Rect(small_width, 0, small_width, small_height));
	cv::Mat roiImage3(view_image, cv::Rect(2*small_width, 0, small_width, small_height));
	cv::Mat roiImage4(view_image, cv::Rect(0, small_height, small_width, small_height));
	cv::Mat roiImage5(view_image, cv::Rect(small_width, small_height, small_width, small_height));
	cv::Mat roiImage6(view_image, cv::Rect(2*small_width, small_height, small_width, small_height));
		
	cv::Mat resizeFrame(small_height, small_width, CV_8UC3);
	cv::Mat resizeFrame2(small_height, small_width, CV_8UC3);
	
	cv::Mat resizeGrayFrame(small_height, small_width, CV_8UC1);
	cv::Mat binFrame(small_height, small_width, CV_8UC1);
	cv::Mat edgeFrame(small_height, small_width, CV_8UC1);
	cv::Mat hsvFrame(small_height, small_width, CV_8UC3);

	cv::Point string_pt(10, small_height/2);

	// Main Loop
	do {
		// Capture an image from camera
		capture >> frame;

		// original image
		cv::resize(frame, resizeFrame, resizeFrame.size());
		resizeFrame.copyTo(roiImage1);
		cv::putText(roiImage1, "1 Original", string_pt, cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0,255,255), 3);

		// HSV image
		cv::cvtColor(resizeFrame, hsvFrame, CV_RGB2HSV);	// create HSV image
		hsvFrame.copyTo(roiImage2);
		cv::putText(roiImage2, "2 HSV", string_pt, cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0,255,255), 3);	// draw text on display window

		// gray scale
		cv::cvtColor(resizeFrame, resizeGrayFrame, CV_RGB2GRAY);
		cv::cvtColor(resizeGrayFrame, resizeFrame2, CV_GRAY2RGB);
		resizeFrame2.copyTo(roiImage3);
		cv::putText(roiImage3, "3 Gray", string_pt, cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0,255,255), 3);

		// binarization
		cv::adaptiveThreshold(resizeGrayFrame, binFrame, 255,cv::ADAPTIVE_THRESH_MEAN_C,cv::THRESH_BINARY, small_height/2 -1 + small_height%2, 0);	// binarization
		cv::cvtColor(binFrame, resizeFrame2, CV_GRAY2RGB);
		resizeFrame2.copyTo(roiImage4);
		cv::putText(roiImage4, "4 Binarize", string_pt, cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0,255,255), 3);

		// edge image
		cv::Canny(resizeGrayFrame, edgeFrame, 64, 128);	// canny operator (edge detection)
		cv::cvtColor(edgeFrame, resizeFrame2, CV_GRAY2RGB);
		resizeFrame2.copyTo(roiImage5);
		cv::putText(roiImage5, "5 Edge", string_pt, cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0,255,255), 3);

		// Morphology
		cv::Mat kernelMat = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5,5));
		cv::morphologyEx(resizeFrame, resizeFrame2, cv::MORPH_OPEN, kernelMat, cv::Point(-1,-1), 3);
//		cv::erode(resizeFrame, resizeFrame2, kernelMat, cv::Point(-1,-1), 3);
		resizeFrame2.copyTo(roiImage6);
		cv::putText(roiImage6, "6 Morphology", string_pt, cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0,255,255), 3);

		cv::imshow( windowNameCam, view_image);

	} while( cv::waitKey( 1) != 'q' );	// quit when a key 'q' is clicked

	return 0;
}
