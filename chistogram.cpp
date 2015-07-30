# include "chistogram.h"
#include "edge.h"
#include "outXls.h"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <vector>

void showHistogram(Mat im,string str)
{
	im.convertTo(im, CV_8U);
	cout << "th1=" <<TH1<< endl;
	string savePath;
	/// 设定bin数目
	int histSize = 256-TH1;

	/// 设定取值范围 ( R,G,B) )
	float range[] = { TH1, 255 };
	const float* histRange = { range };

	bool uniform = true; 
	bool accumulate = true;

	Mat r_hist;

	/// 计算直方图:
	calcHist(&im, 1, 0, Mat(), r_hist, 1, &histSize, &histRange, uniform, true);

	savePath = "./output/histogram_" + str + ".xls";
	outXls(savePath,r_hist,"float" );

	Mat accumulate_hist = Mat::zeros(r_hist.rows, 1, CV_32F);

	// 创建直方图画布
	int hist_w = 400; int hist_h = 400;
	int bin_w = cvRound((double)hist_w / histSize);

	Mat histImage(hist_w, hist_h, CV_8UC3, Scalar(0, 0, 0));

	//将负的直方图扩展进来
	
	//accumulate histogram
	for (int m = 0; m < r_hist.rows;m++)
	{
		accumulate_hist.at<float>(m, 0) = sum(r_hist,m);
	}
	outXls("./output/accuxls.xls", accumulate_hist, "float");

	/// 将直方图归一化到范围 [ 0, histImage.rows ],[0,400]
	normalize(r_hist, r_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat());
	//normalize(accumulate_hist, accumulate_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat());
	
	/// 在直方图画布上画出直方图
	for (int i = 1; i < histSize; i++)
	{
		/*Point up(bin_w*(i - 1), hist_h - cvRound(r_hist.at<float>(i - 1)));
		Point bottom(bin_w*(i - 1), hist_h);
		line(histImage, bottom, up, Scalar(0, 0, 255), 2, 8, 0);*/

		/*line(histImage, Point(bin_w*(i - 1), hist_h - cvRound(accumulate_hist.at<float>(i - 1))),
			Point(bin_w*(i), hist_h - cvRound(accumulate_hist.at<float>(i))),
				Scalar(0, 0, 255), 2, 8, 0);*/
		line(histImage, Point(bin_w*(i - 1), hist_h - cvRound(r_hist.at<float>(i - 1))),
			Point(bin_w*(i), hist_h - cvRound(r_hist.at<float>(i))),
			Scalar(0, 0, 255), 2, 8, 0);
	}


	/// 显示直方图
	imshow(str, histImage);

}