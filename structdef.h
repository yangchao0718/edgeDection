//proecss global struct variable
#ifndef STRUCTDEF_H
#define STRUCTDEF_H
#include <opencv2/highgui/highgui.hpp>  
#include <opencv2/imgproc/imgproc.hpp>  
#include "highgui.h"  
#include "structdef.h"
#include <cv.h>  
#include <cvaux.h>
using namespace cv;
struct MyStruct
{
	int m_num;
	int m_index;
};
struct startPT 
{
	int thresold;
	Point2i C;
	Point2i A;
	Point2i B;
	int nType;
	int direction;
};
#endif