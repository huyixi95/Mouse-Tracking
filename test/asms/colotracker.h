#ifndef COLOTRACKER_H
#define COLOTRACKER_H

#include <opencv2/opencv.hpp>
#include <iostream>

#include "histogram.h"
#include "region.h"


#define SHOWDEBUGWIN

//BGR
//#define BIN_1 128
//#define BIN_2 128
//#define BIN_3 128

//HSV 
#define BIN_1 60
#define BIN_2 32
#define BIN_3 64

class ColorTracker {

public:

	ColorTracker() {}

	~ColorTracker() {}

	void init(cv::Mat img, cv::Rect box) {
		init(img, box.x, box.y, box.br().x, box.br().y);
	}

	// Set last object position - starting position for next tracking step
	inline void setLastBBox(int x1, int y1, int x2, int y2) {
		lastPosition.setBBox(x1, y1, x2 - x1, y2 - y1, 1, 1);
	}

	inline BBox * getBBox() {
		BBox * bbox = new BBox();
		*bbox = lastPosition;
		return bbox;
	}

	inline cv::Rect getLastBox() {
		return cv::Rect(lastPosition.x, lastPosition.y, 
			lastPosition.width, lastPosition.height);
	}

	cv::Rect track(cv::Mat & img, double x1, double y1, double x2, double y2);

	cv::Rect update(cv::Mat& img) {
		return track(img, lastPosition.x, lastPosition.y, lastPosition.x + lastPosition.width,
			lastPosition.y + lastPosition.height);
	}

	//int frame;
	int sumIter;

protected:

	void init(cv::Mat & img, int x1, int y1, int x2, int y2);

	cv::Point histMeanShift(double x1, double y1, double x2, double y2);

	cv::Point histMeanShiftIsotropicScale(double x1, double y1, double x2, double y2, double * scale, int * msIter = NULL);

	void extractBackgroundHistogram(int x1, int y1, int x2, int y2, Histogram &hist);

	void extractForegroundHistogram(int x1, int y1, int x2, int y2, Histogram &hist);

	void extractCircleBackgroundHistogram(int x1, int y1, int x2, int y2, Histogram &hist);

	void extractCircleForegroundHistogram(int x1, int y1, int x2, int y2, Histogram &hist);

	inline void preprocessImage(cv::Mat& img) {
		cv::Mat ra[3] = { im1, im2, im3 };
		cv::cvtColor(img, img, CV_BGR2HSV);
		cv::split(img, ra);
	}

	inline double kernelProfile_Epanechnikov(double x) {
		return (x <= 1) ? (2.0 / CV_PI) * (1 - x) : 0;
		//return (x <= 1) ? (2.0 / 3.14)*(1 - x) : 0;
	}

	inline double kernelProfile_EpanechnikovDeriv(double x) {
		return (x <= 1) ? (-2.0 / CV_PI) : 0;
		//return (x <= 1) ? (-2.0 / 3.14) : 0;
	}

private:

	BBox lastPosition;
	cv::Mat im1;
	cv::Mat im2;
	cv::Mat im3;

	cv::Mat im1_old;
	cv::Mat im2_old;
	cv::Mat im3_old;

	Histogram q_hist;
	Histogram q_orig_hist;
	Histogram b_hist;

	double defaultWidth;
	double defaultHeight;

	double wAvgBg;
	double bound1;
	double bound2;
};

#endif // COLOTRACKER_H
