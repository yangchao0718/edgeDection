#include "edge.h"  
#include"stdlib.h"
#include "chistogram.h"
#include "outXls.h"
#include <algorithm>
using namespace std;
using namespace cv;

Edge::Edge(Mat in)
{
	if (in.channels() == 3)
	{
		rgb = true;
		imshow("【原图】", in);
		split(in, srcBGR);
		srcBGR.at(0).convertTo(srcBGR.at(0), CV_32F);
		srcBGR.at(1).convertTo(srcBGR.at(1), CV_32F);
		srcBGR.at(2).convertTo(srcBGR.at(2), CV_32F);
		g_calImg_I = Mat::zeros(in.rows, in.cols, CV_32F);
		for (int i = 0; i < in.rows; ++i)
		{
			for (int j = 0; j < in.cols; ++j)
			{
				int k = srcBGR.at(0).at<float>(i, j);
				g_calImg_I.at<float>(i, j) = (srcBGR.at(0).at<float>(i, j) + srcBGR.at(1).at<float>(i, j) + srcBGR.at(2).at<float>(i, j)) / 3;
			}
		}
		outXls("./output/grayI.xls", g_calImg_I, "float");
		//yuv2gray.copyTo(gray);
		//outXls("./output/yuv2gray.xls", gray, "int");
		cvtColor(in, gray, CV_BGR2GRAY);
	}
	else
	{
		namedWindow("原图");
		imshow("原图", in);
		in.copyTo(gray);
	}
	ShowHelpText(in);
	//cvtColor(in, gray, CV_BGR2GRAY);

}
Edge::~Edge()
{

}

//Fill a missed point during the marching process for a inline boundary
//input:C and E is the inline edge point
//ouput:miss or not 
bool  GetMissedPoint(Point2i centPtB, Point2i C, Point2i  E, int direction, Point2i& missedPt)
{
	int  directC, directE, deltat, dx, dy, directP;

	dx = C.x - E.x;
	dy = C.y - E.y;
	directC = getIangle(centPtB, C);

	if (abs(dx) < 2 && abs(dy)<2) //already connected
		return false;

	else if ((abs(dx) == 2) && (abs(dy) == 2)){
		if (direction>0){
			directP = directC + 2;
			if (directP >= 8)
				directP -= 8;
		}
		else{
			directP = directC - 2;
			if (directP < 0)
				directP += 8;
		}

		missedPt.x = centPtB.x + a8i[directP];
		missedPt.y = centPtB.y + a8j[directP];
	}

	else if (abs(dx) == 2)
	{
		if (dy == 0)
		{
			if (centPtB.y == C.y){
				if (direction > 0){
					directP = directC + 2;
					if (directP >= 8)
						directP -= 8;
				}
				else{
					directP = directC - 2;
					if (directP<0)
						directP += 8;
				}
			}
			else{
				if (direction>0){
					directP = directC + 1;
					if (directP >= 8)
						directP -= 8;
				}
				else{
					directP = directC - 1;
					if (directP < 0)
						directP += 8;
				}
			}
			missedPt.x = centPtB.x + a8i[directP];
			missedPt.y = centPtB.y + a8j[directP];
		}
		else{
			missedPt.x = (C.x + E.x) / 2;
			if (C.y == centPtB.y)
				missedPt.y = E.y;
			else
				missedPt.y = C.y;
		}
	}

	else //abs(dy)==2
	{
		if (dx == 0)
		{
			if (centPtB.x == C.x)
			{
				if (direction > 0)
				{
					directP = directC + 2;
					if (directP >= 8)
						directP -= 8;
				}
				else{
					directP = directC - 2;
					if (directP<0)
						directP += 8;
				}
			}
			else{
				if (direction>0){
					directP = directC + 1;
					if (directP >= 8)
						directP -= 8;
				}
				else{
					directP = directC - 1;
					if (directP < 0)
						directP += 8;
				}
			}
			missedPt.x = centPtB.x + a8i[directP];
			missedPt.y = centPtB.y + a8j[directP];
		}
		else{//dx == 1
			missedPt.y = (C.y + E.y) / 2;
			if (C.x == centPtB.x)
				missedPt.x = E.x;
			else
				missedPt.x = C.x;
		}
	}

	//if (g_imgFlg.at<int>(missedPt.x, missedPt.y) == 0)
	//{
	//	return true;
	//}
	//else
	//	return false;
	return true;

}

//彩色图像的向量差值，用于比较是否属于同一个区域
float absAB(Point2i A1, Point2i A2)
{
	float tmp;
	if (rgb)
	{
		tmp = abs(srcBGR[0].at<float>(A1.x, A1.y) - srcBGR[0].at<float>(A2.x, A2.y)) +
			abs(srcBGR[1].at<float>(A1.x, A1.y) - srcBGR[1].at<float>(A2.x, A2.y)) +
			abs(srcBGR[2].at<float>(A1.x, A1.y) - srcBGR[2].at<float>(A2.x, A2.y));
	}
	else
	{
		tmp = abs(g_calImg.at<float>(A1.x, A1.y) - g_calImg.at<float>(A2.x, A2.y));
	}
	return tmp;
}

void calcStartPixelType(Point2i centerPt){
	int nType;
	startPT tmp_S;
	int i, j;
	i = centerPt.x;
	j = centerPt.y;
	int thresold = g_PTh.at<int>(i, j);
	int indext;

	nType = g_PType.at<int>(i, j);
	//g_PType.at<int>(i, j) = nType;


	//if ((nType >= 3 && nType <= 7) && g_PBigSmall.at<int>(i, j) == 1)//nType == 3 || nType == 6 || nType == 5 || nType == 4 || nType == 7  &&g_PBigSmall.at<int>(i,j)==0
	//{

	//	if (thresold >= TH1&&TH1 + 5 >= thresold)
	//	{
	//		disappearedPoint.push_back(centerPt);
	//		//将起点信息添加到向量
	//		/*tmp_S.thresold = thresold;
	//		tmp_S.C = centerPt;
	//		tmp_S.A = A;
	//		tmp_S.B = B;
	//		tmp_S.nType = nType;
	//		tmp_S.direction = direction;
	//		S37.push_back(tmp_S);*/
	//	}
	//	else if (thresold > TH1 + 5)
	//	{
	//		if (g_PBigSmall.at<int>(i, j) == 1)
	//		{
	//			g_edge[0].at<float>(centerPt.x, centerPt.y) = 0;
	//			g_edge[1].at<float>(centerPt.x, centerPt.y) = 255;
	//			g_edge[2].at<float>(centerPt.x, centerPt.y) = 0;

	//		}
	//		else
	//		{
	//			g_edge[0].at<float>(centerPt.x, centerPt.y) = 0;
	//			g_edge[1].at<float>(centerPt.x, centerPt.y) = 0;
	//			g_edge[2].at<float>(centerPt.x, centerPt.y) = 255;
	//		}


	//	}
	//	else
	//	{
	//		//thresold too small
	//		//cout << "small" << endl;
	//		g_edge[0].at<float>(centerPt.x, centerPt.y) = 1;
	//		g_edge[1].at<float>(centerPt.x, centerPt.y) = 1;
	//		g_edge[2].at<float>(centerPt.x, centerPt.y) = 1;
	//	}

	//	if (g_PNdifference.at<int>(centerPt.x, centerPt.y) == 2 && g_PPdifference.at<int>(centerPt.x, centerPt.y) == 2)//g_PNdifference.at<int>(centerPt.x, centerPt.y) == 2 && g_PPdifference.at<int>(centerPt.x, centerPt.y) == 2
	//	{
	//		g_edge[0].at<float>(centerPt.x, centerPt.y) = 0;
	//		g_edge[1].at<float>(centerPt.x, centerPt.y) = 0;
	//		g_edge[2].at<float>(centerPt.x, centerPt.y) = 255;
	//	}

	//	/*g_edge[0].at<float>(centerPt.x, centerPt.y) = 0;
	//	g_edge[1].at<float>(centerPt.x, centerPt.y) = 255;
	//	g_edge[2].at<float>(centerPt.x, centerPt.y) = 0;*/

	//	//将起点信息添加到向量
	//	/*	tmp_S.thresold = thresold;
	//		tmp_S.C = centerPt;
	//		tmp_S.A = A;
	//		tmp_S.B = B;
	//		tmp_S.nType = nType;
	//		tmp_S.direction = direction;*/
	//	//S37.push_back(tmp_S);
	//	/*if (nType == 6 || nType == 5 || nType == 4)
	//	{
	//	S5.push_back(tmp_S);
	//	}
	//	else
	//	S37.push_back(tmp_S);*/

	//}

	//if (g_PNumbOfS.at<int>(i, j) >= 2 && g_PNumbOfS.at<int>(i, j) <= 7)
	//{
	//	if (g_PBigSmall.at<int>(i, j) == 1)
	//	{
	//		g_edge[0].at<float>(centerPt.x, centerPt.y) = 255;
	//		g_edge[1].at<float>(centerPt.x, centerPt.y) = 255;
	//		g_edge[2].at<float>(centerPt.x, centerPt.y) = 0;
	//	}
	//	else
	//	{
	//		g_edge[0].at<float>(centerPt.x, centerPt.y) = 0;
	//		g_edge[1].at<float>(centerPt.x, centerPt.y) = 255;
	//		g_edge[2].at<float>(centerPt.x, centerPt.y) = 255;
	//	}
	//	
	//}

	//if (g_PNdifference.at<int>(centerPt.x, centerPt.y) == 1 && g_PPdifference.at<int>(centerPt.x, centerPt.y) == 1)
	//{
	//	g_edge[0].at<float>(centerPt.x, centerPt.y) = 0;
	//	g_edge[1].at<float>(centerPt.x, centerPt.y) = 0;
	//	g_edge[2].at<float>(centerPt.x, centerPt.y) = 255;
	//}
	//if (g_PNumbOfS.at<int>(i, j) == 2 &&g_PNdifference.at<int>(centerPt.x, centerPt.y) == g_PPdifference.at<int>(centerPt.x, centerPt.y) && g_PPdifference.at<int>(centerPt.x, centerPt.y)>0 && g_PPdifference.at<int>(centerPt.x, centerPt.y)<5)
	//{
	//	g_edge[0].at<float>(centerPt.x, centerPt.y) = 0;
	//	g_edge[1].at<float>(centerPt.x, centerPt.y) = 255;
	//	g_edge[2].at<float>(centerPt.x, centerPt.y) = 0;
	//}
}

//Input:min_max为1表示找最大，为0表示找最小
//Output：返回最大或最小值及及相应下标,如果没找到返回-1
MyStruct searchMValue(Point2i centPt, int min_max)
{
	bool found = false;
	int  flag;
	int neig8[8];
	int CV;
	CV = g_calImg.at<float>(centPt.x, centPt.y);
	if (min_max > 0)//findmax
	{
		for (int k = 0; k < 8; k++)
		{
			neig8[k] = -1;
		}
	}
	else//findmin
	{
		for (int k = 0; k < 8; k++)
		{
			neig8[k] = 257;
		}
	}

	for (int i = 0; i < 8; i++)
	{
		flag = g_imgFlg.at<int>(centPt.x + a8i[i], centPt.y + a8j[i]);
		if (!flag)
		{
			neig8[i] = g_calImg.at<float>(centPt.x + a8i[i], centPt.y + a8j[i]);
		}
	}

	if (min_max>0)
	{
		mmi = find_Max(neig8, 8);

		if (CV >= mmi.m_num)
		{
			return mmi;
		}
		else
		{
			mmi.m_index = -1;
			mmi.m_num = -1;
			return mmi;
		}
	}
	else
	{
		mmi = find_Min(neig8, 8);
		if (CV <= mmi.m_num)
		{
			return mmi;
		}
		else
		{
			mmi.m_index = -1;
			mmi.m_num = -1;
			return mmi;
		}
	}

}

//input:A,B...
//outPut:6 points +,6 points -
bool getSixPoints(Point2i A, Point2i B, vector<Point2i>&same_position, vector<Point2i>& diff_position, int direction){
	//find the 5th point from the same side
	int dBA = getIangle(B, A);
	Point2i  tmp_same, tmp_diff;
	if (direction > 0){

		for (int i = 0; i <= 6; i++)
		{
			if (!outOfrange(B.x + a8i[(dBA + i) % 8], B.y + a8j[(dBA + i) % 8]) || !outOfrange(B.x + a8i[(dBA - i + 8) % 8], B.y + a8j[(dBA - i + 8) % 8]))
			{
				tmp_same.x = B.x + a8i[(dBA + i) % 8]; tmp_same.y = B.y + a8j[(dBA + i) % 8];
				tmp_diff.x = B.x + a8i[(dBA - i + 7) % 8]; tmp_diff.y = B.y + a8j[(dBA - i + 7) % 8];
				same_position.push_back(tmp_same);
				diff_position.push_back(tmp_diff);
			}
			else
				return false;

		}

	}
	else{
		for (int i = 0; i <= 6; i++)
		{
			if (!outOfrange(B.x + a8i[(dBA + i) % 8], B.y + a8j[(dBA + i) % 8]) || !outOfrange(B.x + a8i[(dBA - i + 8) % 8], B.y + a8j[(dBA - i + 8) % 8]))
			{
				tmp_same.x = B.x + a8i[(dBA - i + 8) % 8]; tmp_same.y = B.y + a8j[(dBA - i + 8) % 8];
				tmp_diff.x = B.x + a8i[(dBA + i + 1) % 8]; tmp_diff.y = B.y + a8j[(dBA + i + 1) % 8];
				same_position.push_back(tmp_same);
				diff_position.push_back(tmp_diff);
			}
			else
				return false;
		}
	}

	return true;
}
//find a point samller than centPt
float getSmaller(float centPt, float* pt_diff_next, bool min_max) {
	float temp[5] = { 256, 256, 256, 256, 256 };
	float result = 0;
	for (int i = 0; i < 5; i++)
	{
		if (min_max)
		{
			temp[i] = centPt - pt_diff_next[i + 1];
		}
		else
		{
			temp[i] = pt_diff_next[i + 1] - centPt;
		}

	}
	sort(temp, temp + 5);
	for (int j = 0; j < 5; j++)
	{
		if (temp[j]>0)
		{
			result = temp[j];
			break;
		}
	}
	return result;
}
//B is center point,default:seek bigValue edge
bool TraceEdgeTwoBoundary3(Point2i A, Point2i B, Point2i &D, int  direction, bool min_max, Point2i AA, int type7, int tag, vector<Point2i> &stretchD){
	++calTime;
	bool buffer = g_PBuffer.at<int>(B.x, B.y);
	int flagD = false;
	float BV = g_calImg.at<float>(B.x, B.y);
	//the 4,5,6th's relative location
	float pt_same[9] = { 0, 0, 0, 0, 0, 0 };
	float pt_diff[9] = { 0, 0, 0, 0, 0, 0 };
	float pt_same_next4[9], pt_same_next5[9], pt_same_next6[9];
	float pt_diff_next4[9] = { 0, 0, 0, 0, 0, 0 };
	float pt_diff_next5[9] = { 0, 0, 0, 0, 0, 0 };
	float pt_diff_next6[9] = { 0, 0, 0, 0, 0, 0 };
	vector<Point2i> position_same, position_diff;
	int dBA = getIangle(B, A);
	int type = 0;
	//find the 5th point from the same side
	if (!getSixPoints(A, B, position_same, position_diff, direction))
	{
		return false;
	}

	//get the difference:same side set as maxValue and different side set as minValue
	float maxValue[4], minValue[4];
	//4th samePointsSet and differentPointsSet
	float same4[4] = { pt_same[0], pt_same[1], pt_same[2], BV };
	float diff4[4] = { pt_diff[0], pt_diff[1], pt_diff[2], pt_diff[3] };
	float same5[5] = { pt_same[0], pt_same[1], pt_same[2], pt_same[3], BV };
	float diff5[3] = { pt_diff[0], pt_diff[1], pt_diff[2] };
	float same6[6] = { pt_same[0], pt_same[1], pt_same[2], pt_same[3], pt_same[4], BV };
	float diff6[2] = { pt_diff[0], pt_diff[1] };

	//4th
	maxValue[0] = min(min(absAB(position_same.at(3), position_same.at(1)), absAB(position_same.at(3), position_same.at(2))), absAB(position_same.at(3), B));//same:1,2,BV     diff:0,1 
	//5th
	maxValue[1] = min(min(absAB(position_same.at(4), position_same.at(1)), absAB(position_same.at(4), position_same.at(2))), absAB(position_same.at(4), B));
	//6th
	maxValue[2] = min(min(absAB(position_same.at(5), position_same.at(1)), absAB(position_same.at(5), position_same.at(2))), absAB(position_same.at(5), B));
	//7th
	maxValue[3] = min(min(absAB(position_same.at(6), position_same.at(1)), absAB(position_same.at(6), position_same.at(2))), absAB(position_same.at(6), B));
	minValue[0] = min(absAB(position_same.at(3), position_diff.at(0)), absAB(position_same.at(3), position_diff.at(1)));
	minValue[1] = min(absAB(position_same.at(4), position_diff.at(0)), absAB(position_same.at(4), position_diff.at(1)));
	minValue[2] = min(absAB(position_same.at(5), position_diff.at(0)), absAB(position_same.at(5), position_diff.at(1)));
	minValue[3] = min(absAB(position_same.at(6), position_diff.at(0)), absAB(position_same.at(6), position_diff.at(1)));
	int tmp_minValue;
	float sameA = g_calImg.at<float>(A.x, A.y);
	float diffAC = pt_diff[1];
	int typeD = 0;
	Point2i cornerPt;
	//Whether pt5 satisfies the condition[pt5?pt6:pt4],//get smaller than pt5, pt_diff_next
	//result>big value in same area
	if (maxValue[1] <= minValue[1]) {//Take pt5 as center point,from different side find point:pt22-pt55,judge whether pt5 statisfies the bellow condition
		{
			if (maxValue[2] <= minValue[2])//&& minValue[1]>TH1
			{
				Point2i tmp_D = position_same.at(6);
				Point2i tmp_diff0 = position_diff.at(0);
				//判断是不是7：水平、斜向，分为二大类：起点刚进行延伸的判断；延伸过程中的判断
				if (calTime == 1)
				{
					//分为类型为3、7
					if (type == 7)
					{
						//类型为7，则延伸点不考虑7类型，可以画图看矛盾。
						//用6类型
						//退出7，标记6
						D = position_same.at(5);
						flagD = true;
						type = 6;
					}
					else if (type == 3)
					{
						//起点类型为3，延伸如果为7，要求满足如下条件
						tmp_minValue = absAB(position_same.at(6), position_diff.at(0));
						if (maxValue[3] <= tmp_minValue)
						{
							D = position_same.at(6);
							flagD = true;
							find7 = true;
							//type = 7;
						}
						else
						{
							//退出7，标记6
							D = position_same.at(5);
							flagD = true;
							type = 6;
						}
					}

				}
				else//延伸过程中的判断
				{//分为水平斜向两大类
					tmp_minValue = absAB(position_same.at(6), position_diff.at(0));
					if (abs(tmp_D.x - A.x) == 1 && abs(tmp_D.y - A.y) == 1)//slash
					{
						if (maxValue[3] <= tmp_minValue)
						{
							D = position_same.at(6);
							flagD = true;
							//type = 7;
						}
						else
						{	//退出7，标记6
							D = position_same.at(5);
							flagD = true;
							type = 6;
						}
					}
					else//herizion
					{
						tmp_minValue = absAB(position_same.at(6), position_diff.at(0));
						if (g_imgFlg.at<int>(tmp_diff0.x, tmp_diff0.y) != 2 && maxValue[3] <= tmp_minValue)
						{
							D = position_same.at(6);
							flagD = true;
							find7 = true;
							//type = 7;

						}
						else
						{	//退出7，标记6
							D = position_same.at(5);
							flagD = true;
							type = 6;
						}
					}

				}

			}//===========================================================================6th========================
			else//pt5
			{
				D = position_same.at(4);
				flagD = true;
				type = 5;
			}
		}
	}
	else if (maxValue[0] <= minValue[0]) {//pt4     && result4 >= maxValue[0] slow the condition  && minValue[0]>TH1

		D = position_same.at(3);
		flagD = true;
		type = 4;

	}
	else if (3)//pt3   abs(tmpCV3-BV)<TH1
	{
		D = position_same.at(2);
		flagD = true;
		type = 3;
	}
	else {//No match,tag last point

		g_edge[0].at<float>(B.x, B.y) = 255;
		g_edge[1].at<float>(B.x, B.y) = 255;
		g_edge[2].at<float>(B.x, B.y) = 0;
		return false;
	}

	//tag color to D
	if (outOfrange(D.x, D.y))
	{
		return false;
	}

	if (g_imgFlg.at<int>(D.x, D.y) == 0)//no tag    && g_PBuffer.at<int>(D.x, D.y) == buffer
	{

		//set color to D
		if (min_max)//big
		{
			g_edge[0].at<float>(D.x, D.y) = 0;
			g_edge[1].at<float>(D.x, D.y) = 0;
			g_edge[2].at<float>(D.x, D.y) = 255;

		}
		else//small
		{
			g_edge[0].at<float>(D.x, D.y) = 0;
			g_edge[1].at<float>(D.x, D.y) = 255;
			g_edge[2].at<float>(D.x, D.y) = 255;
		}

		g_imgFlg.at<int>(D.x, D.y) = tag;


		//tage both size along the edge
		if (!g_imgFlg.at<int>(B.x + a8i[(dBA + 1) % 8], B.y + a8j[(dBA + 1) % 8]))
		{
			g_imgFlg.at<int>(B.x + a8i[(dBA + 1) % 8], B.y + a8j[(dBA + 1) % 8]) = -tag;
		}
		if (!g_imgFlg.at<int>(B.x + a8i[(dBA + 2) % 8], B.y + a8j[(dBA + 2) % 8]))
		{
			g_imgFlg.at<int>(B.x + a8i[(dBA + 2) % 8], B.y + a8j[(dBA + 2) % 8]) = -tag;
		}
		if (!g_imgFlg.at<int>(B.x + a8i[(dBA - 1 + 8) % 8], B.y + a8j[(dBA - 1 + 8) % 8]))
		{
			g_imgFlg.at<int>(B.x + a8i[(dBA - 1 + 8) % 8], B.y + a8j[(dBA - 1 + 8) % 8]) = -tag;
		}
		if (!g_imgFlg.at<int>(B.x + a8i[(dBA - 2 + 8) % 8], B.y + a8j[(dBA - 2 + 8) % 8]))
		{
			g_imgFlg.at<int>(B.x + a8i[(dBA - 2 + 8) % 8], B.y + a8j[(dBA - 2 + 8) % 8]) = -tag;
		}

	}
	else//collision,tag contact point
	{
		SemiCirleEdge(stretchD, B, D, tag);
		return false;
	}


	position_same.clear(); position_diff.clear();
	g_numPixel++;
	return true;
}

void SemiCirleEdge(vector<Point2i>&stretchD, Point2i B, Point2i D, int tag)
{
	if (stretchD.size() < 3)
	{
		return;
	}
	int tagD = g_imgFlg.at<int>(D.x, D.y);
	vector<Point2i> SE;
	Point2i S, E;
	int index_S;
	if (tagD == -tag)//collsion with iteseltf
	{
		int k;
		for (k = 0; k < 8; k++)
		{
			int x = D.x + a8i[k];
			int y = D.y + a8j[k];
			Point2i tmp;
			tmp.x = x;
			tmp.y = y;
			if (g_imgFlg.at<int>(x, y) == tag&&tmp != B)
			{
				S = tmp;
				break;
			}
		}
		//if there is no other edge point in D8
		if (k == 7)
		{
			S = B;
		}
		//sort these point according index in stretchD
		E = B;
		int i;
		for (i = 0; i < stretchD.size(); ++i)
		{
			if (stretchD.at(i).x == S.x&&stretchD.at(i).y == S.y)
			{
				index_S = i;
				break;
			}

		}
		if (i == stretchD.size())
		{
			/*g_edge[0].at<float>(D.x, D.y) = 255;
			g_edge[1].at<float>(D.x, D.y) = 0;
			g_edge[2].at<float>(D.x, D.y) = 0;*/
			return;
		}
		if (stretchD.size() - index_S < 6)
		{
			//clear <S,E>
			for (int i = index_S; i < stretchD.size(); ++i)
			{
				SE.push_back(stretchD.at(i));
			}
			SE.push_back(D);
			clearShort(SE);
		}
		else
		{
			//clear <stretchD>
			/*for (int i = index_S; i < stretchD.size(); ++i)
			{
			SE.push_back(stretchD.at(i));
			}
			SE.push_back(D);
			clearShort(stretchD);*/
		}


	}
	else if (tagD == tag)
	{

	}
	else//collision with other edge
	{
		g_edge[0].at<float>(D.x, D.y) = 255;
		g_edge[1].at<float>(D.x, D.y) = 0;
		g_edge[2].at<float>(D.x, D.y) = 0;
	}


}
//输入：A、B、C、D、E，direction--[B为待处理的中心点，D、E为下一个B、C]
//输出：动态阈值th g_thresold.at<float>(centerPt.x, centerPt.y) = getDth(A, centerPt, AC, B, BC, direction);
float getDth(Point2i A, Point2i B, Point2i C, Point2i D, Point2i E, int direction)//  ABD  CE
{
	float dt = 0;
	int dA, dD, dC, dE, dTmp;
	vector<Point2i> pt1, pt2, pt3;
	Point2i tmp;
	int tmp1[8] = { -1, -1, -1, -1, -1, -1, -1, -1 }, tmp2[2] = { 256 }, tmp3[8] = { 256, 256, 256, 256, 256, 256, 256, 256 };//初始化有问题
	float max1, max2, max3, min;
	dA = getIangle(B, A);
	dD = getIangle(B, D);
	dC = getIangle(B, C);
	dE = getIangle(B, E);
	int BV = g_calImg.at<float>(B.x, B.y);
	if (direction > 0)
	{
		//1与边缘同侧的点
		pt1.push_back(A);
		while ((dA) != dD)
		{
			dA++;
			if (dA == 8)
			{
				dA -= 8;
			}
			tmp.x = B.x + a8i[dA];
			tmp.y = B.y + a8j[dA];
			pt1.push_back(tmp);
			//cout << "da=" << dA<<endl;
			//cout << "dd=" << dD << endl;
		}
		//pt1.push_back(D);

		//2A,B,D边缘线
		pt2.push_back(A);
		pt2.push_back(B);
		pt2.push_back(D);

		//3与边缘异侧
		pt3.push_back(C);
		if (dC == dE)
		{
			pt3.push_back(E);
		}
		else
		{
			while (dC != dE)
			{
				//cout << "2" << endl;
				dC--;
				if (dC == -1)
				{
					dC += 8;
				}
				tmp.x = B.x + a8i[dC];
				tmp.y = B.y + a8j[dC];
				pt3.push_back(tmp);
			}
			//pt3.push_back(E);
		}


	}
	else
	{
		//1与边缘同侧的点
		pt1.push_back(A);
		while (dA != dD)
		{
			dA--;
			if (dA == -1)
			{
				dA += 8;
			}
			tmp.x = B.x + a8i[dA];
			tmp.y = B.y + a8j[dA];
			pt1.push_back(tmp);
			//cout << "da=" << dA<<endl;
			//cout << "dd=" << dD << endl;
		}
		//pt1.push_back(D);

		//2A,B,D边缘线
		pt2.push_back(A);
		pt2.push_back(B);
		pt2.push_back(D);

		//3与边缘异侧
		pt3.push_back(C);
		if (dC == dE)
		{
			pt3.push_back(E);
		}
		else
		{
			while (dC != dE)
			{
				//cout << "2" << endl;
				dC++;
				if (dC == 8)
				{
					dC -= 8;
				}
				tmp.x = B.x + a8i[dC];
				tmp.y = B.y + a8j[dC];
				pt3.push_back(tmp);
			}
			//pt3.push_back(E);//把E放进了两次

		}

	}

	int tmpV;

	//边缘区域同侧取最大
	if (pt1.size() > 1)
	{
		for (int i = 0; i < pt1.size(); i++)
		{
			tmpV = g_calImg.at<float>(pt1.at(i).x, pt1.at(i).y);
			tmp1[i] = abs(tmpV - BV);
		}
		max1 = findMax(tmp1, pt1.size());
	}
	/*if (pt2.size() > 1)
	{
	for (int i = 0; i + 1<pt2.size(); i++)
	{
	tmp2[i] = abs(g_calImg.at<float>(pt2.at(i + 1).x, pt2.at(i + 1).y) - g_calImg.at<float>(pt2.at(i).x, pt2.at(i).y));
	}
	min = findMin(tmp2, pt2.size() - 1);
	}*/


	//以下tmp2暂无用到
	tmp2[0] = abs(g_calImg.at<float>(pt2.at(0).x, pt2.at(0).y) - g_calImg.at<float>(pt3.at(0).x, pt3.at(0).y));//A - C
	int l3;
	l3 = pt3.size() - 1;
	tmp2[1] = abs(g_calImg.at<float>(pt2.at(2).x, pt2.at(2).y) - g_calImg.at<float>(pt3.at(l3).x, pt3.at(l3).y));//D - E
	//min = findMin(tmp2, 2);

	//与边缘区域异侧取最小
	if (pt3.size() > 1)
	{
		for (int i = 0; i < pt3.size(); i++)
		{
			tmpV = g_calImg.at<float>(pt3.at(i).x, pt3.at(i).y);
			tmp3[i] = abs(tmpV - BV);
		}
		min = findMin(tmp3, pt3.size());
	}
	pt1.clear(); pt2.clear(); pt3.clear();


	//min>max1
	dt = min - max1;
	if (dt > 0)
	{
		dt = (min + max1) / 2;
	}
	else
		dt = TH1;



	return dt;


}
bool cmp(const void *elem1, const void *elem2)
{
	struct startPT *a = (startPT *)elem1;
	struct startPT *b = (startPT *)elem2;

	if (a->thresold > b->thresold)
		return true;
	return false;
}
bool cmp1(struct startPT a, struct startPT b)
{
	//Descend sort
	if (a.thresold > b.thresold)
		return true;
	return false;
}

void Edge::edgeDetection1()
{
	int dx, dy, direction;//边缘的方向相对坐标
	Point2i A, B, AC, BC, D, E, centPt, stPt, AA;

	g_medImg.convertTo(g_medImg, CV_32F);
	int num = 0;
	bool keyi = true;
	int i, j;
	int x, y;
	bool min_max;
	vector<Point2i> stretchD1, stretchE1, stretchD2, stretchE2, startP;
	int tagS = 0;
	//对得到的起点集按阈值进行排序
	sort(S37.begin(), S37.end(), cmp1);
	sort(S5.begin(), S5.end(), cmp1);
	int lengthS37 = S37.size();
	int lengthS5 = S5.size();
	bool isStartPt = false;
	int type;
	int tag37 = 0;
	int tag5 = 10000;
	for (int k = 0; k < lengthS37; k++)
	{
		centPt = S37.at(k).C;
		tagS = g_imgFlg.at<int>(centPt.x, centPt.y);
		isStartPt = startPoint(centPt.x, centPt.y);
		i = centPt.x; j = centPt.y;
		int th = g_PTh.at<int>(i, j);

		if (!tagS&&th >= TH1&&isStartPt)
		{
			++tag37;
			A = S37.at(k).A;
			B = S37.at(k).B;
			min_max = 1;
			type = S37.at(k).nType;
			direction = S37.at(k).direction;

			startP.push_back(A);
			startP.push_back(B);
			startP.push_back(centPt);

			//set color and tag to S,A,B
			g_edge[0].at<float>(centPt.x, centPt.y) = 0;
			g_edge[1].at<float>(centPt.x, centPt.y) = 255;
			g_edge[2].at<float>(centPt.x, centPt.y) = 0;

			g_edge[0].at<float>(A.x, A.y) = 0;
			g_edge[1].at<float>(A.x, A.y) = 255;
			g_edge[2].at<float>(A.x, A.y) = 255;

			g_edge[0].at<float>(B.x, B.y) = 0;
			g_edge[1].at<float>(B.x, B.y) = 255;
			g_edge[2].at<float>(B.x, B.y) = 255;

			g_imgFlg.at<int>(A.x, A.y) = tag37;
			g_imgFlg.at<int>(centPt.x, centPt.y) = tag37;
			g_imgFlg.at<int>(B.x, B.y) = tag37;

			//3分别向两个方向进行延伸	
			keyi = true;
			stPt = centPt;
			calTime = 0;
			while (keyi){
				AA = B;
				keyi = TraceEdgeTwoBoundary3(stPt, A, D, direction, min_max, AA, type, tag37, stretchD1);
				stretchD1.push_back(D);
				stPt = A;
				A = D;
			}
			calTime = 0;
			stPt = centPt;
			keyi = true;
			while (keyi){
				AA = A;
				keyi = TraceEdgeTwoBoundary3(stPt, B, D, -direction, min_max, AA, type, tag37, stretchD2);
				stretchD2.push_back(D);
				stPt = B;
				B = D;
			}

			if (stretchD1.size() <= LTH&&stretchD2.size() <= LTH)
			{
				//清除
				clearShort(stretchD1);
				clearShort(stretchD2);
				clearShort(startP);

			}

			stretchD1.clear();
			stretchD2.clear();
			startP.clear();
		}


	}

	//对起点类型为5的进行处理
	for (int k = 0; k < lengthS5; k++)
	{
		centPt = S5.at(k).C;
		tagS = g_imgFlg.at<int>(centPt.x, centPt.y);
		isStartPt = startPoint(centPt.x, centPt.y);
		i = centPt.x; j = centPt.y;
		int th = g_PTh.at<int>(i, j);

		if (!tagS&&th >= TH1&&isStartPt)
		{
			++tag5;
			A = S5.at(k).A;
			B = S5.at(k).B;
			min_max = 1;
			type = S5.at(k).nType;
			direction = S5.at(k).direction;

			startP.push_back(A);
			startP.push_back(B);
			startP.push_back(centPt);

			//set color and tag to S,A,B
			g_edge[0].at<float>(centPt.x, centPt.y) = 0;
			g_edge[1].at<float>(centPt.x, centPt.y) = 255;
			g_edge[2].at<float>(centPt.x, centPt.y) = 0;

			g_edge[0].at<float>(A.x, A.y) = 0;
			g_edge[1].at<float>(A.x, A.y) = 255;
			g_edge[2].at<float>(A.x, A.y) = 255;

			g_edge[0].at<float>(B.x, B.y) = 0;
			g_edge[1].at<float>(B.x, B.y) = 255;
			g_edge[2].at<float>(B.x, B.y) = 255;

			g_imgFlg.at<int>(A.x, A.y) = 1;
			g_imgFlg.at<int>(centPt.x, centPt.y) = 1;
			g_imgFlg.at<int>(B.x, B.y) = 1;

			//3分别向两个方向进行延伸	
			//keyi = false;
			keyi = true;
			stPt = centPt;
			calTime = 0;
			while (keyi){

				//keyi = TraceEdgeTwoBoundary1(stPt, A, AC, D, E, direction);
				AA = B;
				keyi = TraceEdgeTwoBoundary3(stPt, A, D, direction, min_max, AA, type, tag5, stretchD1);

				stretchD1.push_back(D);
				//stretchE1.push_back(E);

				stPt = A;
				A = D;
				//AC = E;
			}
			calTime = 0;
			stPt = centPt;
			//keyi = false;
			keyi = true;
			while (keyi){
				//keyi = TraceEdgeTwoBoundary1(stPt, B, BC, D, E, -direction);
				AA = A;
				keyi = TraceEdgeTwoBoundary3(stPt, B, D, -direction, min_max, AA, type, tag5, stretchD2);

				stretchD2.push_back(D);
				//stretchE2.push_back(E);
				stPt = B;
				B = D;
				//BC = E;
			}
			if (stretchD1.size() <= LTH&&stretchD2.size() <= LTH)
			{
				//清除
				clearShort(stretchD1);
				clearShort(stretchD2);
				clearShort(startP);

			}

			stretchD1.clear();
			stretchD2.clear();
			startP.clear();

		}

	}

	//set color to  disappearedPoint
	vector<Point2i>::iterator first1 = disappearedPoint.begin();
	while (first1 != disappearedPoint.end())
	{
		Point2i pt = *first1;
		if (g_PBigSmall.at<int>(pt.x, pt.y) == 1)
		{
			g_edge[0].at<float>(pt.x, pt.y) = 255;
			g_edge[1].at<float>(pt.x, pt.y) = 0;
			g_edge[2].at<float>(pt.x, pt.y) = 255;
		}
		else
		{
			g_edge[0].at<float>(pt.x, pt.y) = 255;
			g_edge[1].at<float>(pt.x, pt.y) = 0;
			g_edge[2].at<float>(pt.x, pt.y) = 0;
		}

		++first1;
	}
	//set color to  triBranchPoints
	/*
	vector<Point2i>::iterator first2 = triBranchPoint.begin();
	while (first2 != triBranchPoint.end())
	{
	Point2i pt = *first2;
	if (g_PBigSmall.at<int>(pt.x,pt.y) == 1)
	{
	g_edge[0].at<float>(pt.x, pt.y) = 255;
	g_edge[1].at<float>(pt.x, pt.y) = 255;
	g_edge[2].at<float>(pt.x, pt.y) = 0;
	}

	++first2;
	}
	//*/
	outXls("./output/imFlag.xls", g_imgFlg, "int");

	//输出混合图像【边缘+原图】
	getMix();

	//得到混合的彩色边缘点云图
	g_edge[0].convertTo(g_edge[0], CV_8UC1);
	g_edge[1].convertTo(g_edge[1], CV_8UC1);
	g_edge[2].convertTo(g_edge[2], CV_8UC1);
	merge(g_edge, g_mergImg);

	imshow("g_mergImg", g_mergImg);


	imwrite("./output/median.bmp", g_mergImg);
	merge(g_srcImageChannels, g_srcImageBGR);
	imwrite("./output/tagEdge.bmp", g_srcImageBGR);

	//将得到的彩色边缘图二值化
	Mat bimap;
	cvtColor(g_mergImg, bimap, CV_BGR2GRAY);
	threshold(bimap, bimap, 2, 255, THRESH_BINARY);

	string str, str1;
	//char buffer[10];
	//_itoa(nTH, buffer, 10);
	LPCWSTR svDir;
	//mkdir

	//CreateDirectory(TEXT("aa\\bb"), NULL);
	//CreateDirectory(svDir,NULL)

	if (nTH < 10)
	{
		str = "output/tmp/" + num2str(0) + num2str(nTH) + ".png";
		//str1 = "output/" + num2str(0) + num2str(nTH) + ".bmp";
	}
	else
		str = "output/tmp/" + num2str(nTH) + ".png";

	//imwrite(str1, g_mergImg);
	imwrite(str, bimap);
	//imshow("边缘图", bimap);

	//得到不同星型的混合边缘图
	//getTypeEdge(false);

	//添加噪音
	//if (CONTRADICTION)
	//{
	//	salt(g_srcImage);
	//	//imwrite("./output/CONTRADICTION.bmp", g_srcImage);
	//}

	//以后的循环初始化
	nextInitial();
	S37.clear(); S5.clear();

}
void Edge::edgeDetection2()
{
	float ptN8, centPt;
	int n1 = 0, n2 = 0, n3 = 0;
	Point2i pt;
	int cntPixel = 0;
	//找到前两个边缘点作为起始延伸点
	for (int i = 1; i < gray.rows - 1; i++)
	{
		for (int j = 1; j < gray.cols - 1; j++)
		{
			pt.x = i;
			pt.y = j;
			//get the type image
			if (g_nTypeFlag.at<int>(pt.x, pt.y) == 0)
			{
				nSection1(pt);
			}

		}

	}

	cout << "GT_PIXELS=" << cntPixel << endl;
	//得到不同星型的混合边缘图
	getTypeEdge();
}
void Edge::edgeDetection3(){
	Point2i A, AC, B, BC;
	Point2i pt;
	int indext;

	//1 找到过渡边点，
	for (int i = 1; i < gray.rows - 1; i++)
	{
		for (int j = 1; j < gray.cols - 1; j++)
		{
			//int flagA = false, flagB = false;
			pt.x = i;
			pt.y = j;
			////中心点类型
			//int cv = g_imgFlg2.at<int>(pt.x, pt.y);
			//int tmpFlag, tmpFlagNext;
			//int index;
			////2 找到两个方向上的AB,CD
			////灰度图像
			////如果第一个点和中心点相同，则找一个和中点不相同的点
			//if (cv == 11 && g_imgFlg.at<int>(i, j) == 0 && startPoint(i, j))
			//{
			//	//find 11 in anticlockwise
			//	for (index = 0; index < 8; ++index)
			//	{
			//		tmpFlag = g_imgFlg2.at<int>(i + a8i[index], j + a8j[index]);
			//		tmpFlagNext = g_imgFlg2.at<int>(i + a8i[index + 1], j + a8j[index + 1]);
			//		if (tmpFlag == 11)
			//		{
			//			//11,12.others
			//			if (tmpFlagNext == 12)
			//			{
			//				A.x = i + a8i[index];
			//				A.y = j + a8j[index];
			//				AC.x = i + a8i[index + 1];
			//				AC.y = j + a8j[index + 1];

			//				flagA = true;
			//				break;

			//			}
			//			else if (tmpFlagNext != 11)
			//			{
			//				++index;
			//			}
			//		}
			//	}
			//	//find 11 in clockwise
			//	if (flagA)
			//	{
			//		for (index = 8; index > 0; --index)
			//		{
			//			tmpFlag = g_imgFlg2.at<int>(i + a8i[index], j + a8j[index]);
			//			tmpFlagNext = g_imgFlg2.at<int>(i + a8i[index - 1], j + a8j[index - 1]);
			//			if (tmpFlag == 11)
			//			{
			//				//11,12.others
			//				if (tmpFlagNext == 12)
			//				{
			//					B.x = i + a8i[index];
			//					B.y = j + a8j[index];
			//					BC.x = i + a8i[index - 1];
			//					BC.y = j + a8j[index - 1];

			//					if (A == B || AC == BC)
			//					{
			//						break;
			//					}

			//					flagB = true;
			//					break;

			//				}
			//				else if (tmpFlagNext != 11)
			//				{
			//					--index;
			//				}
			//			}
			//		}
			//	}

			//}
			//else if (cv == 12 && g_imgFlg.at<int>(i, j) == 0 && startPoint(i, j))
			//{
			//	//find 11 in anticlockwise
			//	for (index = 0; index < 8; ++index)
			//	{
			//		tmpFlag = g_imgFlg2.at<int>(i + a8i[index], j + a8j[index]);
			//		tmpFlagNext = g_imgFlg2.at<int>(i + a8i[index + 1], j + a8j[index + 1]);
			//		if (tmpFlag == 12)
			//		{
			//			//11,12.others
			//			if (tmpFlagNext == 11)
			//			{
			//				A.x = i + a8i[index];
			//				A.y = j + a8j[index];
			//				AC.x = i + a8i[index + 1];
			//				AC.y = j + a8j[index + 1];

			//				flagA = true;
			//				break;

			//			}
			//			else if (tmpFlagNext != 12)
			//			{
			//				++index;
			//			}
			//		}
			//	}
			//	//find 11 in clockwise
			//	if (flagA)
			//	{
			//		for (index = 8; index > 0; --index)
			//		{
			//			tmpFlag = g_imgFlg2.at<int>(i + a8i[index], j + a8j[index]);
			//			tmpFlagNext = g_imgFlg2.at<int>(i + a8i[index - 1], j + a8j[index - 1]);
			//			if (tmpFlag == 12)
			//			{
			//				//11,12.others
			//				if (tmpFlagNext == 11)
			//				{
			//					B.x = i + a8i[index];
			//					B.y = j + a8j[index];
			//					BC.x = i + a8i[index - 1];
			//					BC.y = j + a8j[index - 1];

			//					if (A == B || AC == BC)
			//					{
			//						break;
			//					}

			//					flagB = true;
			//					break;

			//				}
			//				else if (tmpFlagNext != 12)
			//				{
			//					--index;
			//				}
			//			}
			//		}
			//	}

			//}


			//if (flagB&&flagA)
			//{
			//	//g_edge[0].at<float>(pt.x, pt.y) = 0;
			//	//g_edge[1].at<float>(pt.x, pt.y) = 255;
			//	//g_edge[2].at<float>(pt.x, pt.y) = 0;

			//	//g_imgFlg.at<int>(pt.x, pt.y) = 1;//起点

			//	//进行延伸操作
			//	calTime = 0;
			//	//traceEdge(A, AC);
			//	calTime = 0;
			//	//traceEdge(B, BC);



			//}

			//tag small edge
			int flag;
			Point2i pt;

			flag = imgFlg3.at<int>(i, j);
			pt.x = i;
			pt.y = j;

			switch (flag)
			{
			case IDEABIG://理想大边
				setColor(pt, 0, 0, 255);//red
				break;
			case IDEASMALL://理想小边
				setColor(pt, 0, 255, 255);//yellow
				break;
			case BUFFERBIG://过渡大边
				setColor(pt, 255, 0, 0);//blue
				break;
			case BUFFERSMALL://过渡小边
				setColor(pt, 0, 255, 0);//green
				break;
			case NOTBIG://大非
				setColor(pt, 255, 0, 255);//magenta
				break;
			case NOTSMALL://小非
				setColor(pt, 0, 125, 125);//half yellow
				break;
			case CONTRADICTION://矛盾点
				setColor(pt, 255, 255, 0);//cyan
				break;
			case INNER://内部点
				//setColor(pt, 0, 0,0);//原图颜色
				break;
				//default:
				//cout << "flag error!\n";
			}

		}//end j

	}//end i



	//for (int i = 1; i < gray.rows - 1; i++)
	//{
	//	for (int j = 1; j < gray.cols - 1; j++)
	//	{
	//		int flag = g_imgFlg.at<int>(i, j);
	//		//find endpoint pt
	//		if (flag == 1)
	//		{
	//			pt.x = i;
	//			pt.y = j;
	//			//度为1的点的前一个点
	//			Point2i prePt = getDegree1PrePoint(pt);
	//			

	//			if (prePt.x != 0)
	//			{

	//				//得到所有端点
	//				g_edge[0].at<float>(pt.x, pt.y) = 0;
	//				g_edge[1].at<float>(pt.x, pt.y) = 255;
	//				g_edge[2].at<float>(pt.x, pt.y) = 255;

	//				stretchTime = 0;
	//				stretchEndPoint(prePt, pt);
	//			}
	//		}

	//	}
	//}

	//输出混合图像【边缘+原图】
	getMix();

	//得到混合的彩色边缘点云图
	g_edge[0].convertTo(g_edge[0], CV_8UC1);
	g_edge[1].convertTo(g_edge[1], CV_8UC1);
	g_edge[2].convertTo(g_edge[2], CV_8UC1);
	merge(g_edge, g_mergImg);

	imshow("g_mergImg", g_mergImg);


	imwrite("./output/median.bmp", g_mergImg);
	merge(g_srcImageChannels, g_srcImageBGR);
	imwrite("./output/tagEdge.bmp", g_srcImageBGR);

	//将得到的彩色边缘图二值化
	Mat bimap;
	cvtColor(g_mergImg, bimap, CV_BGR2GRAY);
	threshold(bimap, bimap, 2, 255, THRESH_BINARY);

	string str, str1;
	//char buffer[10];
	//_itoa(nTH, buffer, 10);
	LPCWSTR svDir;
	//mkdir

	//CreateDirectory(TEXT("aa\\bb"), NULL);
	//CreateDirectory(svDir,NULL)

	if (nTH < 10)
	{
		str = "output/tmp/" + num2str(0) + num2str(nTH) + ".png";
		//str1 = "output/" + num2str(0) + num2str(nTH) + ".bmp";
	}
	else
		str = "output/tmp/" + num2str(nTH) + ".png";

	//imwrite(str1, g_mergImg);
	imwrite(str, bimap);
	//imshow("边缘图", bimap);

	//得到不同星型的混合边缘图
	//getTypeEdge(false);

	//添加噪音
	//if (CONTRADICTION)
	//{
	//	salt(g_srcImage);
	//	//imwrite("./output/CONTRADICTION.bmp", g_srcImage);
	//}

	//以后的循环初始化
	//nextInitial();
}

void setColor(Point2i pt, int B, int G, int R)
{
	g_edge[0].at<float>(pt.x, pt.y) = B;
	g_edge[1].at<float>(pt.x, pt.y) = G;
	g_edge[2].at<float>(pt.x, pt.y) = R;
}
//get endpoint's prePoint
Point2i  getDegree1PrePoint(Point2i pt){
	Point2i temPt = 0;
	int num = 0;
	vector<Point2i> sameFlagPoint;
	for (int i = 0; i < 8; ++i)
	{
		int x = pt.x + a8i[i];
		int y = pt.y + a8j[i];
		if (g_imgFlg.at<int>(x, y) == 1 || g_imgFlg.at<int>(x, y) == 23)
		{
			++num;
			temPt.x = x;
			temPt.y = y;
			sameFlagPoint.push_back(temPt);
		}
	}
	if (num == 1)
	{
		return sameFlagPoint.at(0);
	}
	else if (num == 2)//处理直角转弯的情况 端点确定
	{
		Point2i p1, p2;
		p1 = sameFlagPoint.at(0);
		p2 = sameFlagPoint.at(1);

		if ((abs(p1.x - p2.x) + abs(p1.y - p2.y)) == 1)
		{
			if (p1.x == pt.x || p1.y == pt.y)
			{
				return p1;
			}
			else
				return p2;
		}
		else
			return 0;

	}
	else
		return 0;

}
//延伸边缘
/*
4个颜色的标记在imgflag2中，单边缘图标记在imgflag中
*/
bool stretchEndPoint(Point2i pt_pre, Point2i pt)
{
	int prePt = getIangle(pt, pt_pre);
	int ptFlag = g_imgFlg2.at<int>(pt.x, pt.y);
	Point2i tempPt;
	bool findNext = false;
	bool findSame = false;
	bool findDiff = true;
	//tage point besidew pt_pre
	int x0 = pt.x + a8i[(prePt + 1) % 8];
	int y0 = pt.y + a8j[(prePt + 1) % 8];
	int x1 = pt.x + a8i[(prePt - 1 + 8) % 8];
	int y1 = pt.y + a8j[(prePt - 1 + 8) % 8];
	if (g_imgFlg.at<int>(x0, y0) == 0)
	{
		g_imgFlg.at<int>(x0, y0) = 21;
	}
	if (g_imgFlg.at<int>(x1, y1) == 0)
	{
		g_imgFlg.at<int>(x1, y1) = 22;
	}
	g_imgFlg.at<int>(pt_pre.x, pt_pre.y) = 23;
	g_imgFlg.at<int>(pt.x, pt.y) = 23;

	//查找相同标记的点
	for (int i = 0; i < 8; ++i)
	{
		tempPt.x = pt.x + a8i[i];
		tempPt.y = pt.y + a8j[i];

		bool colision = isColision(tempPt);
		int imgFlag2 = g_imgFlg2.at<int>(tempPt.x, tempPt.y);
		int imgFlag = g_imgFlg.at<int>(tempPt.x, tempPt.y);

		if (imgFlag2 == ptFlag&&imgFlag == 0)//!colision&&
		{

			g_imgFlg.at<int>(tempPt.x, tempPt.y) = 1;
			g_edge[0].at<float>(tempPt.x, tempPt.y) = 255;
			g_edge[1].at<float>(tempPt.x, tempPt.y) = 255;
			g_edge[2].at<float>(tempPt.x, tempPt.y) = 0;
			if (colision)
			{
				break;
			}
			findSame = true;
			findNext = true;
			break;
		}




	}
	//如果没找到相同标记的点，则2找12，12找2
	if (!findSame)
	{
		for (int i = 0; i < 8; ++i)
		{
			tempPt.x = pt.x + a8i[i];
			tempPt.y = pt.y + a8j[i];

			bool colision = isColision(tempPt);
			int imgFlag2 = g_imgFlg2.at<int>(tempPt.x, tempPt.y);
			int imgFlag = g_imgFlg.at<int>(tempPt.x, tempPt.y);
			if (ptFlag == 2)
			{
				if (imgFlag2 == 12 && imgFlag == 0)//!colision&&
				{

					g_imgFlg.at<int>(tempPt.x, tempPt.y) = 1;
					g_edge[0].at<float>(tempPt.x, tempPt.y) = 255;
					g_edge[1].at<float>(tempPt.x, tempPt.y) = 255;
					g_edge[2].at<float>(tempPt.x, tempPt.y) = 0;
					if (colision)
					{
						break;
					}
					findDiff = false;
					findNext = true;
					break;
				}

			}

			if (ptFlag == 12)
			{
				if (imgFlag2 == 2 && imgFlag == 0)//!colision&&
				{

					g_imgFlg.at<int>(tempPt.x, tempPt.y) = 1;
					g_edge[0].at<float>(tempPt.x, tempPt.y) = 255;
					g_edge[1].at<float>(tempPt.x, tempPt.y) = 255;
					g_edge[2].at<float>(tempPt.x, tempPt.y) = 0;
					if (colision)
					{
						break;
					}
					findDiff = false;
					findNext = true;
					break;
				}
			}
		}

		//if (findDiff)
		//{
		//	for (int i = 0; i < 8; ++i)
		//	{
		//		tempPt.x = pt.x + a8i[i];
		//		tempPt.y = pt.y + a8j[i];

		//		bool colision = isColision(tempPt);
		//		int imgFlag2 = g_imgFlg2.at<int>(tempPt.x, tempPt.y);
		//		int imgFlag = g_imgFlg.at<int>(tempPt.x, tempPt.y);
		//		if (imgFlag2 >0 && imgFlag == 0)//!colision&&
		//		{
		//			g_imgFlg.at<int>(tempPt.x, tempPt.y) = 1;
		//			g_edge[0].at<float>(tempPt.x, tempPt.y) = 0;
		//			g_edge[1].at<float>(tempPt.x, tempPt.y) = 255;
		//			g_edge[2].at<float>(tempPt.x, tempPt.y) = 0;
		//			findNext = true;
		//			break;
		//		}
		//	}
		//}

	}
	if (findNext)
	{
		stretchEndPoint(pt, tempPt);
	}

	return true;
}
//判断pt的8邻域是否有边缘点，即延伸过程中是否碰撞到了已有边
bool isColision(Point2i pt)
{
	for (int i = 0; i < 8; ++i)
	{
		if (!outOfrange(pt.x + a8i[i], pt.y + a8j[i]) && g_imgFlg.at<int>(pt.x + a8i[i], pt.y + a8j[i]) == 1)
		{
			return true;
		}
	}
	return false;
}
bool traceEdge(Point2i A1, Point2i A2)
{
	//cout << "..." << endl;
	++calTime;
	if (calTime > 100)
	{
		//return false;
	}
	Point2i E;//next edge point 
	int sumA1 = getSameFlagNumInFlag2(A1, 1);
	int sumA2 = getSameFlagNumInFlag2(A2, 1);
	if (sumA1 > sumA2)
	{
		E = A1;
	}
	else
	{
		E = A2;
	}
	//set color to E
	g_edge[0].at<float>(E.x, E.y) = 0;
	g_edge[1].at<float>(E.x, E.y) = 0;
	g_edge[2].at<float>(E.x, E.y) = 255;
	g_imgFlg.at<int>(E.x, E.y) = 1;

	//find next A,AC
	int flagA = false, flagB = false;
	int i, j;
	i = E.x;
	j = E.y;
	Point2i A, AC, B, BC;
	//中心点类型
	int cv = g_imgFlg2.at<int>(E.x, E.y);
	int tmpFlag, tmpFlagNext;
	int index;
	//2 找到两个方向上的AB,CD
	//灰度图像
	//如果第一个点和中心点相同，则找一个和中点不相同的点
	if (cv == 11)
	{
		//find 11 in anticlockwise
		for (index = 0; index < 8; ++index)
		{
			tmpFlag = g_imgFlg2.at<int>(i + a8i[index], j + a8j[index]);
			tmpFlagNext = g_imgFlg2.at<int>(i + a8i[index + 1], j + a8j[index + 1]);
			if (tmpFlag == 11)
			{
				//11,12.others
				if (tmpFlagNext == 12)
				{

					A.x = i + a8i[index];
					A.y = j + a8j[index];
					AC.x = i + a8i[index + 1];
					AC.y = j + a8j[index + 1];

					flagA = true;
					break;

				}
				/*	else if (tmpFlagNext != 11)
					{
					++index;
					}*/
			}
		}
		//find 11 in clockwise

		for (index = 8; index > 0; --index)
		{
			tmpFlag = g_imgFlg2.at<int>(i + a8i[index], j + a8j[index]);
			tmpFlagNext = g_imgFlg2.at<int>(i + a8i[index - 1], j + a8j[index - 1]);
			if (tmpFlag == 11)
			{
				//11,12.others
				if (tmpFlagNext == 12)
				{
					B.x = i + a8i[index];
					B.y = j + a8j[index];
					BC.x = i + a8i[index - 1];
					BC.y = j + a8j[index - 1];

					if (A == B || AC == BC)
					{
						break;
					}

					flagB = true;
					break;

				}
				/*	else if (tmpFlagNext != 11)
					{
					--index;
					}*/
			}
		}


	}
	else if (cv == 12)
	{
		//find 12 in anticlockwise
		for (index = 0; index < 8; ++index)
		{
			tmpFlag = g_imgFlg2.at<int>(i + a8i[index], j + a8j[index]);
			tmpFlagNext = g_imgFlg2.at<int>(i + a8i[index + 1], j + a8j[index + 1]);
			if (tmpFlag == 12)
			{
				//11,12.others
				if (tmpFlagNext == 11)
				{

					A.x = i + a8i[index];
					A.y = j + a8j[index];
					AC.x = i + a8i[index + 1];
					AC.y = j + a8j[index + 1];

					flagA = true;
					break;

				}
				/*else if (tmpFlagNext != 12)
				{
				++index;
				}*/
			}
		}
		//find 11 in clockwise	
		for (index = 8; index > 0; --index)
		{
			tmpFlag = g_imgFlg2.at<int>(i + a8i[index], j + a8j[index]);
			tmpFlagNext = g_imgFlg2.at<int>(i + a8i[index - 1], j + a8j[index - 1]);
			if (tmpFlag == 12)
			{
				//11,12.others
				if (tmpFlagNext == 11)
				{
					B.x = i + a8i[index];
					B.y = j + a8j[index];
					BC.x = i + a8i[index - 1];
					BC.y = j + a8j[index - 1];

					if (A == B || AC == BC)
					{
						break;
					}

					flagB = true;
					break;

				}
				/*	else if (tmpFlagNext != 12)
					{
					--index;
					if (index==0)
					{
					cout << "index" << index << endl;
					}
					}*/
			}
		}

	}
	//bool trace = true;
	//while (trace)
	//{
	//	trace = traceEdge(A, AC);
	//}
	//trace = true;
	//while (trace)
	//{
	//	trace = traceEdge(B, BC);
	//}

	if (flagA)
	{
		if (g_imgFlg.at<int>(A.x, A.y) == 0 && g_imgFlg.at<int>(AC.x, AC.y) == 0)
		{

			traceEdge(A, AC);
		}
		else
			return false;
	}
	if (flagB)
	{
		if (g_imgFlg.at<int>(B.x, B.y) == 0 && g_imgFlg.at<int>(BC.x, BC.y) == 0)
		{
			//A1 = B;
			//A2 = BC;
			traceEdge(B, BC);
		}
		else
			return false;
	}

	return true;
}

int getSameFlagNumInFlag2(Point2i pt, int flag)
{
	int x1;
	int result = 0;
	for (int i = 0; i < 8; ++i)
	{
		x1 = g_imgFlg2.at<int>(pt.x + a8i[i], pt.y + a8j[i]);
		if (x1 == flag)
		{
			++result;
		}
	}
	return result;
}
void displayMixAttribute(Point2i pt, int th, int sortType, int buffer, int sameType, int numPP, int numPN, int numPS, bool big_small)
{
	g_edge[0].at<float>(pt.x, pt.y) = 255;
	g_edge[1].at<float>(pt.x, pt.y) = 0;
	g_edge[2].at<float>(pt.x, pt.y) = 255;
}

template <class T>
string num2str(T num)
{
	stringstream ss;
	ss << num;

	return ss.str();
}
LPCWSTR stringToLPCWSTR(std::string orig)
{
	size_t origsize = orig.length() + 1;
	const size_t newsize = 100;
	size_t convertedChars = 0;
	wchar_t *wcstring = (wchar_t *)malloc(sizeof(wchar_t)*(orig.length() - 1));
	mbstowcs_s(&convertedChars, wcstring, origsize, orig.c_str(), _TRUNCATE);


	return wcstring;
}
void clearShort(vector <Point2i> &Pt)
{
	int n = Pt.size();
	int x, y;
	for (int i = 0; i < n; i++)
	{
		x = Pt.at(i).x;
		y = Pt.at(i).y;
		g_edge[0].at<float>(x, y) = 0;
		g_edge[1].at<float>(x, y) = 0;
		g_edge[2].at<float>(x, y) = 0;
		g_imgFlg.at<int>(x, y) = 30000;
		//--g_numPixel;
	}
}
//输入一个点，输出：是否符合起点条件，符合返回true
bool startPoint(int i, int j)
{
	//可以在此设置一个下标越界异常处理判断
	//判断8邻域中是否有标记点
	for (int k = 0; k < 8; k++)
	{
		if (g_imgFlg.at<int>(i + a8i[k], j + a8j[k]) && !outOfrange(i + a8i[k], j + a8j[k]))
			return false;
	}
	return true;
}
bool isTag(Point2i centPt)
{
	//判断8邻域中是否有标记点
	for (int k = 0; k < 8; k++)
	{
		if (!outOfrange(centPt.x + a8i[k], centPt.y + a8j[k]) && g_nSectionFlg.at<int>(centPt.x + a8i[k], centPt.y + a8j[k]) == 1)
			return true;
	}
	return false;
}
void getMix()
{

	if (g_srcImage.channels() == 3)//分离三个通道
	{
		split(g_srcImage, g_srcImageChannels);//把原图划分三个通道

		//imshow("第二个通道", g_srcImageChannels.at(1));
		g_srcImageChannels.at(0).convertTo(g_srcImageChannels.at(0), CV_32F);
		g_srcImageChannels.at(1).convertTo(g_srcImageChannels.at(1), CV_32F);
		g_srcImageChannels.at(2).convertTo(g_srcImageChannels.at(2), CV_32F);
		outXls("./output/blue.xls", g_srcImageChannels.at(0), "float");
		outXls("./output/green.xls", g_srcImageChannels.at(1), "float");
		outXls("./output/red.xls", g_srcImageChannels.at(2), "float");

	}
	//else if (g_srcImage.channels() == 1)//转换为三通道
	//{
	//	//定义一个三通道图像
	//	Mat imBGR(gray.rows, gray.cols, CV_8UC3, Scalar::all(0));
	//	cvtColor(g_srcImage, imBGR, CV_GRAY2BGR);
	//	split(g_srcImage, g_srcImageChannels);
	//}

	for (int i = 1; i < g_srcGrayImage.rows - 1; i++)
	{
		for (int j = 1; j < g_srcGrayImage.cols - 1; j++)
		{
			if (g_edge[0].at<float>(i, j)>0 || g_edge[1].at<float>(i, j)>0 || g_edge[2].at<float>(i, j) > 0)
			{
				(g_srcImageChannels.at(0)).at<float>(i, j) = g_edge[0].at<float>(i, j);
				(g_srcImageChannels.at(1)).at<float>(i, j) = g_edge[1].at<float>(i, j);
				(g_srcImageChannels.at(2)).at<float>(i, j) = g_edge[2].at<float>(i, j);
			}
		}
	}
	(g_srcImageChannels.at(0)).convertTo((g_srcImageChannels.at(0)), CV_8UC1);
	(g_srcImageChannels.at(1)).convertTo((g_srcImageChannels.at(1)), CV_8UC1);
	(g_srcImageChannels.at(2)).convertTo((g_srcImageChannels.at(2)), CV_8UC1);


}
void nextInitial()
{
	g_angle = Mat::ones(gray.rows, gray.cols, CV_32F)*(-1);
	g_intAngle = Mat::zeros(gray.rows, gray.cols, CV_32S);
	g_nTypeFlag = Mat::zeros(gray.rows, gray.cols, CV_32S);
	g_dstImage = Mat::zeros(gray.rows, gray.cols, CV_32F);
	g_edge[0] = Mat::zeros(gray.rows, gray.cols, CV_32F);
	g_edge[1] = Mat::zeros(gray.rows, gray.cols, CV_32F);
	g_edge[2] = Mat::zeros(gray.rows, gray.cols, CV_32F);
	g_mergImg = Mat::zeros(gray.rows, gray.cols, CV_8UC3);
	g_type = Mat::zeros(gray.rows, gray.cols, CV_32S);
	g_srcImageBGR = Mat::zeros(gray.rows, gray.cols, CV_8UC3);
	nTypeEdge[0] = Mat::zeros(gray.rows, gray.cols, CV_32S);
	nTypeEdge[1] = Mat::zeros(gray.rows, gray.cols, CV_32S);
	nTypeEdge[2] = Mat::zeros(gray.rows, gray.cols, CV_32S);
	g_imgFlg = Mat::zeros(gray.rows, gray.cols, CV_32S);
}


//判断是否下标越界
bool outOfrange(int i, int j)
{
	if (i >= 0 && i <= gray.rows - 1 && j >= 0 && j <= gray.cols - 1)
		return false;
	else
	{
		//cout << "超出边界范围！\n";
		return true;
	}

}


int  lengthAB(int A, int B, int direction)
{
	if (A == B)
	{
		return 1;
	}
	if (direction < 0)
	{
		if ((A + 1) % 8 == B)
		{
			return 2;
		}
		if ((A + 2) % 8 == B)
		{
			return 3;
		}
		else if ((A + 3) % 8 == B)
		{
			return 4;
		}
		else if ((A + 4) % 8 == B)
		{
			return 5;
		}
		else if ((A + 5) % 8 == B)
		{
			return 6;
		}
		else if ((A + 6) % 8 == B)
		{
			return 7;
		}
		else
			cout << "lengthAB_Error!" << endl;
	}
	else//逆时针,-
	{
		if ((A + 7) % 8 == B)
		{
			return 2;
		}
		if ((A + 6) % 8 == B)
		{
			return 3;
		}
		else if ((A + 5) % 8 == B)
		{
			return 4;
		}
		else if ((A + 4) % 8 == B)
		{
			return 5;
		}
		else if ((A + 3) % 8 == B)
		{
			return 6;
		}
		else  if ((A + 2) % 8 == B)
		{
			return 7;
		}
		else
			cout << "lengthAB_Error!" << endl;
	}

}


//边缘初始化
void Edge::Init()
{
	g_numPixel = 0;
	//processing bgr's value

	g_thresold = Mat::ones(gray.rows, gray.cols, CV_32F)*(-1);
	g_type = Mat::zeros(gray.rows, gray.cols, CV_32S);
	g_angle = Mat::ones(gray.rows, gray.cols, CV_32F)*(-1);//*11可以统一改变初始值
	g_intAngle = Mat::zeros(gray.rows, gray.cols, CV_32S);
	g_dstImage = Mat::zeros(gray.rows, gray.cols, CV_32F);
	g_edge[0] = Mat::zeros(gray.rows, gray.cols, CV_32F);
	g_edge[1] = Mat::zeros(gray.rows, gray.cols, CV_32F);
	g_edge[2] = Mat::zeros(gray.rows, gray.cols, CV_32F);
	g_mergImg = Mat::zeros(gray.rows, gray.cols, CV_8UC3);
	g_circle = Mat::zeros(gray.rows, gray.cols, CV_32S);
	g_srcImageBGR = Mat::zeros(gray.rows, gray.cols, CV_8UC3);

	g_imgFlg = Mat::zeros(gray.rows, gray.cols, CV_32S);
	g_imgFlg2 = Mat::zeros(gray.rows, gray.cols, CV_32S);
	imgFlg3 = Mat::zeros(gray.rows, gray.cols, CV_32S);
	g_nTypeFlag = Mat::zeros(gray.rows, gray.cols, CV_32S);
	g_nSectionFlg = Mat::zeros(gray.rows, gray.cols, CV_32S);

	nTypeEdge[0] = Mat::zeros(gray.rows, gray.cols, CV_32S);
	nTypeEdge[1] = Mat::zeros(gray.rows, gray.cols, CV_32S);
	nTypeEdge[2] = Mat::zeros(gray.rows, gray.cols, CV_32S);


	g_nType = Mat::ones(gray.rows, gray.cols, CV_32S) * 8;
	maxminD = Mat::zeros(gray.rows, gray.cols, CV_32F);
	opmaxminD = Mat::zeros(gray.rows, gray.cols, CV_32F);
	g_biGrad = Mat::zeros(gray.rows, gray.cols, CV_32F);
	g_dstBiGrad = Mat::zeros(gray.rows, gray.cols, CV_32F);

	g_minGrad = Mat::zeros(gray.rows, gray.cols, CV_32F);
	g_maxGrad = Mat::zeros(gray.rows, gray.cols, CV_32F);
	g_grad0 = Mat::zeros(gray.rows, gray.cols, CV_32F);
	g_grad1 = Mat::zeros(gray.rows, gray.cols, CV_32F);
	g_grad2 = Mat::zeros(gray.rows, gray.cols, CV_32F);
	g_grad3 = Mat::zeros(gray.rows, gray.cols, CV_32F);
	g_grad4 = Mat::zeros(gray.rows, gray.cols, CV_32F);
	g_grad5 = Mat::zeros(gray.rows, gray.cols, CV_32F);
	g_grad6 = Mat::zeros(gray.rows, gray.cols, CV_32F);
	g_grad7 = Mat::zeros(gray.rows, gray.cols, CV_32F);

	g_PBigSmall = Mat::zeros(gray.rows, gray.cols, CV_32S);
	g_PBuffer = Mat::zeros(gray.rows, gray.cols, CV_32S);
	g_PTh = Mat::zeros(gray.rows, gray.cols, CV_32S);
	g_PType = Mat::zeros(gray.rows, gray.cols, CV_32S);
	g_PNdifference = Mat::zeros(gray.rows, gray.cols, CV_32S);
	g_PPdifference = Mat::zeros(gray.rows, gray.cols, CV_32S);
	g_PNumbOfS = Mat::zeros(gray.rows, gray.cols, CV_32S);
	bigThanTh = Mat::zeros(254, 1, CV_32S);
	//【中值滤波】
	medianBlur(gray, g_medImg, 3);
	//imshow("1", gray);

	gray.copyTo(g_calImg);
	gray.copyTo(g_mixGS);
	//g_srcImage.copyTo(g_calImg);

	g_calImg.convertTo(g_calImg, CV_32F);//转换类型，便于加减运算
	g_mixGS.convertTo(g_mixGS, CV_32F);

	float tempG[8] = { 0 };
	float maxG = 0, minG = 0;
	int x, y;
	Point2i centerPoint;
	gray.convertTo(gray, CV_32F);
	g_maxGrad = Mat::zeros(gray.rows, gray.cols, CV_32F);

	//计算图像梯度，第一行最后一行第一列最后一列先不考虑梯度，设定为0
	for (int i = 1; i < gray.rows - 1; i++)
	{
		for (int j = 1; j < gray.cols - 1; j++)
		{
			centerPoint.x = i;
			centerPoint.y = j;

			for (int k = 0; k < 8; k++)
			{
				x = i + a8i[k];
				y = j + a8j[k];
				tempGrad[k] = (gray.at<float>(x, y) - gray.at<float>(i, j));
			}
			//get eight different direction Grad
			g_grad0.at<float>(i, j) = tempGrad[0];
			g_grad1.at<float>(i, j) = tempGrad[1];
			g_grad2.at<float>(i, j) = tempGrad[2];
			g_grad3.at<float>(i, j) = tempGrad[3];
			g_grad4.at<float>(i, j) = tempGrad[4];
			g_grad5.at<float>(i, j) = tempGrad[5];
			g_grad6.at<float>(i, j) = tempGrad[6];
			g_grad7.at<float>(i, j) = tempGrad[7];

			//get minGrad & maxGrad
			g_minGrad.at<float>(i, j) = findMin(tempGrad, 8);
			g_maxGrad.at<float>(i, j) = findMax(tempGrad, 8);
			maxminD.at<float>(i, j) = g_maxGrad.at<float>(i, j) - g_minGrad.at<float>(i, j);
			//to some picture,maxminD are  all big than 0

			/*	if (maxminD.at<float>(i,j)<0)
				{
				opmaxminD.at<float>(i, j) = -maxminD.at<float>(i, j);
				}*/
			//opmaxminD.at<float>(i, j) = g_minGrad.at<float>(i, j);

			if (g_maxGrad.at<float>(i, j) > gTH)
			{
				g_biGrad.at<int>(i, j) = 1;
			}
			centerPoint.x = i;
			centerPoint.y = j;
			calcPixelAttribute_new(centerPoint);
			//calcPixelAttribute(centerPoint);
			calcStartPixelType(centerPoint);
		}
	}
	//output PixelAttributes
	outXls("./output/01 g_PTh.xls", g_PTh, "int");
	outXls("./output/03 g_PBuffer.xls", g_PBuffer, "int");
	outXls("./output/04 g_PBigSmall.xls", g_PBigSmall, "int");

	//get global th
	/*getN1();
	N2 = 2*(gray.cols + gray.rows);
	int tmp=10000,absN1N2,index;
	for (int l = 0; l < bigThanTh.rows;l++)
	{
	absN1N2 = abs(bigThanTh.at<int>(l, 0) - N2);
	if (absN1N2<tmp)
	{
	tmp = absN1N2;
	index = l+1;
	}
	}
	TH1 = index;
	cout << "an estimated Th:" << index << endl;*/

	//g_PTh.convertTo(g_PTh, CV_8UC1);
	//imshow("th", g_PTh);

	//g_PTh.convertTo(g_PTh, CV_32S);
	//showHistogram(g_maxGrad, "maxGrad");
	//showHistogram(g_minGrad, "negative");

	//梯度图5*5>8进行个数统计
	//filter2D(g_biGrad, g_dstBiGrad, -1, g_kernel, Point(-1, -1), 0.0, BORDER_DEFAULT);

}
void getN1()
{
	int num;
	int x, y;
	for (int k = 1; k < 255; k++)
	{
		num = 0;
		for (int i = 1; i < gray.rows - 1; i++)
		{
			for (int j = 1; j < gray.cols - 1; j++)
			{
				x = g_PTh.at<int>(i, j);
				y = g_PType.at<int>(i, j);
				if (x >= k&&y>2 && y < 8)
				{
					bigThanTh.at<int>(k - 1, 0) = num++;
				}

			}
		}

	}

}

void InsertSort(int* array, int* index, int length)
{
	int i, j;
	for (i = 1; i < length; ++i)
	{
		int tmp = array[i];
		int tmp_index = index[i];
		for (j = i - 1; j >= 0 && array[j]>tmp; --j)
		{
			array[j + 1] = array[j];
			index[j + 1] = index[j];
		}
		array[j + 1] = tmp;
		index[j + 1] = tmp_index;
	}
}
void calcPixelAttribute(Point2i pt){
	int i, j, m, n;
	int maxNum = 0;
	int max_index, centerPoint_index;
	int max = -1;
	int ptValue;
	int point_9[9];
	int index[9] = { 0, 1, 2, 3, 4, 5, 6, 7, 8 };
	int point_vec_8_difference[8];
	int point_9_difference[8] = { 0 };
	int numPN = 0, numPP = 0, numPS = 0;
	int th = 0, type, buffer = 0, big_small = 0;
	i = pt.x;
	j = pt.y;
	if (rgb)
	{
		ptValue = g_calImg_I.at<float>(i, j);
		//point_9[8] is centerpoint
		point_9[8] = g_calImg_I.at<float>(i, j);
	}
	else
	{
		ptValue = g_calImg.at<float>(i, j);
		point_9[8] = g_calImg.at<float>(i, j);
	}

	for (int k = 0; k < 8; ++k)
	{
		m = a8i[k];
		n = a8j[k];
		if (rgb)
		{
			point_9[k] = g_calImg_I.at<float>(i + m, j + n);
		}
		else
		{
			point_9[k] = g_calImg.at<float>(i + m, j + n);
		}

	}
	for (int k = 0; k < 8; ++k)
	{
		int index;
		int absCenterPi;
		index = k + 1;
		absCenterPi = abs(point_9[8] - point_9[k]);
		if (index == 8)
		{
			index = 0;
		}
		point_9_difference[k] = point_9[index] - point_9[k];

		//count numPN,numPP
		if (point_9_difference[k] >= TH1)
		{
			++numPP;
		}
		if (point_9_difference[k] < -TH1)
		{
			++numPN;
		}
		if (absCenterPi <= TH1)
		{
			++numPS;
		}
	}
	InsertSort(point_9, index, 9);
	//point_vec_9.assign(point_9, point_9 + 9);
	//stable_sort 
	//stable_sort(point_vec_9.begin(), point_vec_9.end());
	//get index of centerpoint
	for (int k = 8; k >= 0; k--)
	{
		if (ptValue == point_9[k])
		{
			centerPoint_index = k;
			break;
		}
	}

	//Difference,get Max_index,TH
	for (int k = 0; k < 8; k++)
	{
		point_vec_8_difference[k] = point_9[k + 1] - point_9[k];
		if (point_vec_8_difference[k] >= TH1)
		{
			maxNum++;
		}

		if (point_vec_8_difference[k]>max)
		{
			max = point_vec_8_difference[k];
			max_index = k;
			if (!rgb)
			{
				th = max;//------------------------------1 th
			}

		}
	}

	//calculate th to bgr
	/*
	int max_i1 = pt.x + a8i[index[max_index]];
	int max_j1 = pt.y + a8j[index[max_index]];
	int max_i2 = pt.x + a8i[index[max_index + 1]];
	int max_j2 = pt.y + a8j[index[max_index + 1]];
	if (rgb)
	{
	int th_b = abs(srcBGR.at(0).at<float>(max_i1, max_j1) - srcBGR.at(0).at<float>(max_i2, max_j2));
	int th_g = abs(srcBGR.at(1).at<float>(max_i1, max_j1) - srcBGR.at(1).at<float>(max_i2, max_j2));
	int th_r = abs(srcBGR.at(2).at<float>(max_i1, max_j1) - srcBGR.at(2).at<float>(max_i2, max_j2));

	int sum = (th_b + th_g + th_r);//------------------------------1 th
	if (sum)
	{
	th = ((float)th_b / sum)*th_b + ((float)th_g / sum)*th_g + ((float)th_r / sum)*th_r;
	}
	else
	th = 0;
	}
	//*/
	if (th < TH1)
	{
		type = 8;
	}
	else
	{
		//calculate Type and big_small
		if (centerPoint_index <= max_index)
		{
			type = max_index;//--------------------------2 type
			big_small = 1;//-----------------------------4 big_small：1为small
		}
		else
		{
			type = 7 - max_index;//--------------------------2 type
			//big_small = 1;//-----------------------------4 big_small：1为small
		}
	}



	//record triBranchPoints
	if (maxNum == 3)
	{
		triBranchPoint.push_back(pt);
	}

	//whether Buffer or not
	//if (maxNum > 1)
	//{
	//	buffer = maxNum;//---------------------------------3 buffer
	//}
	buffer = maxNum;


	//assign
	g_PTh.at<int>(i, j) = th;
	g_PType.at<int>(i, j) = type;
	g_PBuffer.at<int>(i, j) = buffer;
	g_PBigSmall.at<int>(i, j) = big_small;
	g_PPdifference.at<int>(i, j) = numPP;
	g_PNdifference.at<int>(i, j) = numPN;
	g_PNumbOfS.at<int>(i, j) = numPS;

	//1 buffer area numPS==2
	if (0)//type > 2 && type < 8
	{
		if (!SORT)
		{
			if (buffer == 1)//理想边缘点
			{
				if (big_small)//小边
				{
					if (FOURCOLOR)
					{
						g_edge[0].at<float>(pt.x, pt.y) = 0;
						g_edge[1].at<float>(pt.x, pt.y) = 255;
						g_edge[2].at<float>(pt.x, pt.y) = 255;
					}
					/**/
					g_imgFlg2.at<int>(pt.x, pt.y) = 1;
				}
				else//大边
				{
					if (FOURCOLOR)
					{
						g_edge[0].at<float>(pt.x, pt.y) = 0;
						g_edge[1].at<float>(pt.x, pt.y) = 0;
						g_edge[2].at<float>(pt.x, pt.y) = 255;
					}
					/**/
					g_imgFlg2.at<int>(pt.x, pt.y) = 2;
				}
			}
			if (buffer > 1)//过渡边缘点
			{
				if (big_small)//小边
				{
					if (FOURCOLOR)
					{
						g_edge[0].at<float>(pt.x, pt.y) = 0;
						g_edge[1].at<float>(pt.x, pt.y) = 255;
						g_edge[2].at<float>(pt.x, pt.y) = 0;
					}
					/**/
					g_imgFlg2.at<int>(pt.x, pt.y) = 11;
				}
				else//大边
				{
					if (FOURCOLOR)
					{
						g_edge[0].at<float>(pt.x, pt.y) = 255;
						g_edge[1].at<float>(pt.x, pt.y) = 0;
						g_edge[2].at<float>(pt.x, pt.y) = 0;
					}
					/**/
					g_imgFlg2.at<int>(pt.x, pt.y) = 12;
				}
			}
		}
		else
		{
			//
		}

	}

	//if (type > 2 && type < 8)//
	//{
	//	int num;
	//	num = numPP + numPN;
	//	if (num==2)//理想边缘点
	//	{
	//		if (big_small)//小边
	//		{
	//			g_edge[0].at<float>(pt.x, pt.y) = 0;
	//			g_edge[1].at<float>(pt.x, pt.y) = 0;
	//			g_edge[2].at<float>(pt.x, pt.y) = 125;
	//		}
	//		else//大边
	//		{
	//			g_edge[0].at<float>(pt.x, pt.y) = 0;
	//			g_edge[1].at<float>(pt.x, pt.y) = 0;
	//			g_edge[2].at<float>(pt.x, pt.y) = 255;
	//		}
	//	}
	//	if (0)//过渡边缘点
	//	{
	//		if (big_small)//小边
	//		{
	//			g_edge[0].at<float>(pt.x, pt.y) = 125;
	//			g_edge[1].at<float>(pt.x, pt.y) = 0;
	//			g_edge[2].at<float>(pt.x, pt.y) = 0;
	//		}
	//		else//大边
	//		{
	//			g_edge[0].at<float>(pt.x, pt.y) = 255;
	//			g_edge[1].at<float>(pt.x, pt.y) = 0;
	//			g_edge[2].at<float>(pt.x, pt.y) = 0;
	//		}
	//	}
	//}


	//2 背景区域，用黄色表示；
	if (buffer == 0)//&&numPS>2&&numPS<8&&numPP==1&&numPN==1
	{
		/*g_edge[0].at<float>(pt.x, pt.y) = 0;
		g_edge[1].at<float>(pt.x, pt.y) = 255;
		g_edge[2].at<float>(pt.x, pt.y) = 255;*/
	}
	if (numPP == 4 && numPN == 4)
	{
		g_edge[0].at<float>(pt.x, pt.y) = 0;
		g_edge[1].at<float>(pt.x, pt.y) = 0;
		g_edge[2].at<float>(pt.x, pt.y) = 255;
	}
	if (numPP == 3 && numPN == 3)
	{
		g_edge[0].at<float>(pt.x, pt.y) = 0;
		g_edge[1].at<float>(pt.x, pt.y) = 255;
		g_edge[2].at<float>(pt.x, pt.y) = 0;
	}
	if (numPP == 2 && numPN == 2)
	{
		g_edge[0].at<float>(pt.x, pt.y) = 255;
		g_edge[1].at<float>(pt.x, pt.y) = 0;
		g_edge[2].at<float>(pt.x, pt.y) = 0;
	}
	if (numPP == 1 && numPN == 1)
	{
		g_edge[0].at<float>(pt.x, pt.y) = 0;
		g_edge[1].at<float>(pt.x, pt.y) = 255;
		g_edge[2].at<float>(pt.x, pt.y) = 255;
	}

	//displayMixAttribute(pt,th,type, buffer, numPS, numPP, numPN, numPS, big_small);

}
void calcPixelAttribute_new(const Point2i pt){

	//point's Attribute
	int th_low, th_high;
	int th;
	int type;
	int buffer;
	int big_small;
	Point2i temPt;
	vector<Point2i> neighbour;
	int x, y;
	int diff[8] = { 0 };
	int max_positive = 0;
	int min_negative = -1;
	Point2i max_positive_position, min_negative_position;
	vector<Point2i> area_A, area_B;
	bool findBig = false, findSmall = false;
	for (int i = 0; i < 8; ++i)
	{
		diff[i] = g_calImg.at<float>(pt.x + a8i[i + 1], pt.y + a8j[i + 1]) - g_calImg.at<float>(pt.x + a8i[i], pt.y + a8j[i]);
		if (diff[i] >= 0)
		{
			if (diff[i] >= max_positive)
			{
				max_positive = diff[i];
				max_positive_position.x = pt.x + a8i[i];
				max_positive_position.y = pt.y + a8j[i];
				findBig = true;
			}
		}
		else
		{
			if (diff[i] <= min_negative)
			{
				min_negative = diff[i];
				min_negative_position.x = pt.x + a8i[i];
				min_negative_position.y = pt.y + a8j[i];
				findSmall = true;
			}
		}
	}
	if (findBig&&findSmall)
	{
		th = min(abs(min_negative), abs(max_positive));

		//get two area:big area A,small area B;
		int big_area_direction, small_area_direction;
		Point2i temPt;
		big_area_direction = getIangle(pt, max_positive_position);
		small_area_direction = getIangle(pt, min_negative_position);

		//big area A
		int indexA1 = (big_area_direction + 1) % 8;//bigbehind-small
		int indexA2 = small_area_direction;
		while (indexA1 != indexA2)
		{

			temPt.x = pt.x + a8i[indexA1];
			temPt.y = pt.y + a8j[indexA1];
			area_A.push_back(temPt);
			++indexA1;
			if (indexA1 == 8)
			{
				indexA1 = 0;
			}
		}
		area_A.push_back(min_negative_position);
		int lenA = area_A.size();

		//small area B
		int indexB1 = (small_area_direction + 1) % 8;
		int indexB2 = big_area_direction;
		while (indexB1 != indexB2)
		{

			temPt.x = pt.x + a8i[indexB1];
			temPt.y = pt.y + a8j[indexB1];
			area_B.push_back(temPt);
			++indexB1;
			if (indexB1 == 8)
			{
				indexB1 = 0;
			}
		}
		area_B.push_back(max_positive_position);
		int lenB = area_B.size();

		//get samll value in  A:small_A;get big value in B:big_B;
		vector<int> diffA, diffB;
		int big_diffA = 0, big_diffB = 0, big_diffAB;
		int tmpDiff;
		int small_A, big_B;
		small_A = g_calImg.at<float>(area_A.at(0).x, area_A.at(0).y);
		big_B = g_calImg.at<float>(area_B.at(0).x, area_B.at(0).y);
		//if (lenA==1)
		//{
		//	big_diffA = 0;
		//}
		//if (lenB==1)
		//{
		//	big_diffB = 0;
		//}
		for (int i = 1; i < lenA; ++i)
		{
			if (g_calImg.at<float>(area_A.at(i).x, area_A.at(i).y)<small_A)
			{
				small_A = g_calImg.at<float>(area_A.at(i).x, area_A.at(i).y);
			}
			tmpDiff = abs(g_calImg.at<float>(area_A.at(i).x, area_A.at(i).y) - g_calImg.at<float>(area_A.at(i - 1).x, area_A.at(i - 1).y));
			if (tmpDiff>big_diffA)
			{
				big_diffA = tmpDiff;
			}
		}
		for (int i = 1; i<lenB; ++i)
		{
			if (g_calImg.at<float>(area_B.at(i).x, area_B.at(i).y)>big_B)
			{
				big_B = g_calImg.at<float>(area_B.at(i).x, area_B.at(i).y);
			}
			tmpDiff = abs(g_calImg.at<float>(area_B.at(i).x, area_B.at(i).y) - g_calImg.at<float>(area_B.at(i - 1).x, area_B.at(i - 1).y));
			if (tmpDiff > big_diffB)
			{
				big_diffB = tmpDiff;
			}
		}
		//area_A.clear(); area_B.clear();
		//get big_diffA and big_diffB;
		big_diffAB = max(big_diffA, big_diffB);
		th_high = th;
		th_low = big_diffAB + 1;


		//whether centerPoint belongs to A or B
		int centValue = g_calImg.at<float>(pt.x, pt.y);
		if (centValue > small_A)
		{
			//centValue belongs big part A
			big_small = 2;
			type = lenA;
		}
		else if (centValue < big_B)
		{
			//belong to small Part B
			big_small = 1;
			type = lenB;
		}
		else
		{
			int distanceA, distanceB;
			distanceA = abs(centValue - small_A);
			distanceB = abs(centValue - big_B);
			if (distanceA > distanceB)
			{
				//belong to small Part B
				big_small = 1;
				type = lenB;
			}
			else
			{
				//centValue belongs big part A
				big_small = 2;
				type = lenA;
			}
		}

		//whether centpoint is buffer area
		int diff_smallA_big_B;
		diff_smallA_big_B = abs(small_A - big_B);
		if (big_diffA > diff_smallA_big_B || big_diffB > diff_smallA_big_B)
		{
			//belong to buffer area
			buffer = 2;
		}
		else
			buffer = 1;
	}
	else//pure inner points
	{
		th_low = 0;
		th_high = 0;
		type = 8;
		big_small = -1;
		buffer = -1;
		g_imgFlg2.at<int>(pt.x, pt.y) = INNER;
	}


	//assign
	g_PTh.at<int>(pt.x, pt.y) = th_low;
	g_PType.at<int>(pt.x, pt.y) = type;
	g_PBuffer.at<int>(pt.x, pt.y) = buffer;
	g_PBigSmall.at<int>(pt.x, pt.y) = big_small;
	int tmpFlag;
	tmpFlag = imgFlg3.at<int>(pt.x, pt.y);
	if (type > 2 && type < 8 && th_high>10)// && th >= TH1 &&th_low>10 
	{
		if (th_low <= th_high)
		{
			if (buffer == 1)//理想边缘点
			{
				if (big_small == 1)//小边
				{
					/*if (FOURCOLOR)
					{
					g_edge[0].at<float>(pt.x, pt.y) = 0;
					g_edge[1].at<float>(pt.x, pt.y) = 255;
					g_edge[2].at<float>(pt.x, pt.y) = 255;
					}*/
					/**/
					if (tmpFlag == NOTBIG)
					{
						imgFlg3.at<int>(pt.x, pt.y) = CONTRADICTION;
					}
					else
					{

						imgFlg3.at<int>(pt.x, pt.y) = IDEASMALL;
						setPointFlag(pt, area_A, area_B, IDEASMALL);
					}

				}
				else if (big_small == 2)//大边
				{
					/*	if (FOURCOLOR)
						{
						g_edge[0].at<float>(pt.x, pt.y) = 0;
						g_edge[1].at<float>(pt.x, pt.y) = 0;
						g_edge[2].at<float>(pt.x, pt.y) = 255;
						}*/
					/**/
					if (tmpFlag == NOTSMALL)
					{
						imgFlg3.at<int>(pt.x, pt.y) = CONTRADICTION;
					}
					else
					{
						imgFlg3.at<int>(pt.x, pt.y) = IDEABIG;
						setPointFlag(pt, area_A, area_B, IDEABIG);
					}

				}
			}
			if (buffer == 2)//过渡边缘点
			{
				if (big_small == 1)//小边
				{
					/*if (FOURCOLOR)
					{
					g_edge[0].at<float>(pt.x, pt.y) = 0;
					g_edge[1].at<float>(pt.x, pt.y) = 255;
					g_edge[2].at<float>(pt.x, pt.y) = 0;
					}*/
					/**/
					if (tmpFlag == NOTBIG)
					{
						imgFlg3.at<int>(pt.x, pt.y) = CONTRADICTION;

					}
					else
					{
						imgFlg3.at<int>(pt.x, pt.y) = BUFFERSMALL;
						setPointFlag(pt, area_A, area_B, BUFFERSMALL);
					}

				}
				else if (big_small == 2)//大边
				{
					/*if (FOURCOLOR)
					{
					g_edge[0].at<float>(pt.x, pt.y) = 255;
					g_edge[1].at<float>(pt.x, pt.y) = 0;
					g_edge[2].at<float>(pt.x, pt.y) = 0;
					}*/
					/**/
					if (tmpFlag == NOTSMALL)
					{
						imgFlg3.at<int>(pt.x, pt.y) = CONTRADICTION;

					}
					else
					{
						imgFlg3.at<int>(pt.x, pt.y) = BUFFERBIG;
						setPointFlag(pt, area_A, area_B, BUFFERBIG);
					}

				}
			}
		}
		else
		{
			//CONTRADICTION
			/*g_edge[0].at<float>(pt.x, pt.y) = 255;
			g_edge[1].at<float>(pt.x, pt.y) = 255;
			g_edge[2].at<float>(pt.x, pt.y) = 0;*/
		}

	}

}

void setPointFlag(Point2i pt, vector<Point2i> big_section, vector<Point2i> small_section, int Flag)
{
	int lenA = big_section.size();
	int lenB = small_section.size();
	int tmpFlag, x;
	if (Flag == IDEABIG || Flag == BUFFERBIG)
	{
		//标记大非
		for (int i = 0; i < lenA; ++i)
		{
			tmpFlag = imgFlg3.at<int>(big_section.at(i).x, big_section.at(i).y);
			if (tmpFlag == BUFFERSMALL || tmpFlag == IDEASMALL)
			{
			}
			else
			{
				if (tmpFlag == INNER)
				{
					imgFlg3.at<int>(big_section.at(i).x, big_section.at(i).y) = NOTBIG;
				}
				else if (tmpFlag == NOTSMALL)
				{
					imgFlg3.at<int>(big_section.at(i).x, big_section.at(i).y) = CONTRADICTION;
				}

			}


		}
		//标记小非
		for (int j = 0; j < lenB; ++j)
		{
			tmpFlag = imgFlg3.at<int>(small_section.at(j).x, small_section.at(j).y);

			if (tmpFlag == BUFFERBIG || tmpFlag == IDEABIG)
			{

			}
			else
			{
				if (tmpFlag == INNER)
				{
					imgFlg3.at<int>(small_section.at(j).x, small_section.at(j).y) = NOTSMALL;
				}
				else if (tmpFlag == NOTBIG)
				{
					imgFlg3.at<int>(small_section.at(j).x, small_section.at(j).y) = CONTRADICTION;
				}

			}

		}

	}

	if (Flag == IDEASMALL || Flag == BUFFERSMALL)
	{
		//标记大非
		for (int i = 0; i < lenA; ++i)
		{
			tmpFlag = imgFlg3.at<int>(big_section.at(i).x, big_section.at(i).y);
			if (tmpFlag == BUFFERBIG || tmpFlag == IDEABIG)
			{

			}
			else
			{
				if (tmpFlag == INNER)
				{
					imgFlg3.at<int>(big_section.at(i).x, big_section.at(i).y) = NOTBIG;
				}
				else if (tmpFlag == NOTSMALL)
				{
					imgFlg3.at<int>(big_section.at(i).x, big_section.at(i).y) = CONTRADICTION;
				}

			}


		}
		//标记小非
		for (int j = 0; j < lenB; ++j)
		{
			tmpFlag = imgFlg3.at<int>(small_section.at(j).x, small_section.at(j).y);
			if (tmpFlag == BUFFERSMALL || tmpFlag == IDEASMALL)
			{
			}
			else
			{
				if (tmpFlag == INNER)
				{
					imgFlg3.at<int>(small_section.at(j).x, small_section.at(j).y) = NOTSMALL;
				}
				else if (tmpFlag == NOTBIG)
				{
					imgFlg3.at<int>(small_section.at(j).x, small_section.at(j).y) = CONTRADICTION;
				}

			}


		}
	}

}
//Canny检测函数
void Edge::on_Canny(int, void*)
{
	//cvtColor(g_srcImage, g_srcGrayImage, CV_BGR2GRAY);
	// 先使用 3x3内核来降噪
	g_srcGrayImage.convertTo(g_cannyDetectedEdges, CV_8UC1);
	//blur(g_srcGrayImage, g_cannyDetectedEdges, Size(3, 3));

	// 运行我们的Canny算子  
	Canny(g_cannyDetectedEdges, g_cannyDetectedEdges, g_cannyLowThreshold, g_cannyLowThreshold * 2, 3);

	////先将g_dstImage内的所有元素设置为0  
	//g_dstImage = Scalar::all(0);

	////使用Canny算子输出的边缘图g_cannyDetectedEdges作为掩码，来将原图g_srcImage拷到目标图g_dstImage中  
	//g_srcImage.copyTo(g_dstImage, g_cannyDetectedEdges);

	//显示效果图  
	//namedWindow("Canny", CV_WINDOW_NORMAL);
	imshow("canny", g_cannyDetectedEdges);
	imwrite("./canny.png", g_cannyDetectedEdges);
	//---------------------------------------------------------------------------------------------------------------
	if (g_srcImage.channels() == 3)//分离三个通道
	{
		split(g_srcImage, g_srcImageChannels_canny);//把原图划分三个通道

		//imshow("第二个通道", g_srcImageChannels.at(1));
		g_srcImageChannels_canny.at(0).convertTo(g_srcImageChannels_canny.at(0), CV_32F);
		g_srcImageChannels_canny.at(1).convertTo(g_srcImageChannels_canny.at(1), CV_32F);
		g_srcImageChannels_canny.at(2).convertTo(g_srcImageChannels_canny.at(2), CV_32F);
	}

	g_cannyDetectedEdges.convertTo(g_cannyDetectedEdges, CV_32F);
	for (int i = 1; i < g_srcGrayImage.rows - 1; i++)
	{
		for (int j = 1; j < g_srcGrayImage.cols - 1; j++)
		{


			if (g_cannyDetectedEdges.at<float>(i, j) > 0)
			{
				g_srcImageChannels_canny.at(0).at<float>(i, j) = 0;
				g_srcImageChannels_canny.at(1).at<float>(i, j) = 0;
				g_srcImageChannels_canny.at(2).at<float>(i, j) = g_cannyDetectedEdges.at<float>(i, j);
			}

		}
	}
	g_cannyDetectedEdges.convertTo(g_cannyDetectedEdges, CV_8UC1);

	(g_srcImageChannels_canny.at(0)).convertTo((g_srcImageChannels_canny.at(0)), CV_8UC1);
	(g_srcImageChannels_canny.at(1)).convertTo((g_srcImageChannels_canny.at(1)), CV_8UC1);
	(g_srcImageChannels_canny.at(2)).convertTo((g_srcImageChannels_canny.at(2)), CV_8UC1);

	merge(g_srcImageChannels_canny, g_srcImageBGR_canny);
	imwrite("./output/tagEdge_canny.bmp", g_srcImageBGR_canny);
	//--------------------------------------------------------------------------------------------------------------------
}

//运行帮助
static void ShowHelpText(Mat Image)
{
	cout << "图像的尺寸：" << "宽=" << Image.cols << "    " << "高=" << Image.rows << endl << endl;
	cout << "图像的通道数：" << Image.channels() << endl;
}
//求最大最小及其对应下标
//template <class T>
MyStruct find_Min(int *arr, int N)
{

	int min = arr[0];
	int index = 0;
	for (int i = 1; i < N; i++)
	{
		if (min>arr[i])
		{
			min = arr[i];
			index = i;
		}
	}
	mmi.m_num = min;
	mmi.m_index = index;
	return mmi;
}
//template <class T>
MyStruct find_Max(int *arr, int N)
{

	int max = arr[0];
	int index = 0;
	for (int i = 1; i < N; i++)
	{
		if (max < arr[i])
		{
			max = arr[i];
			index = i;
		}
	}
	mmi.m_num = max;
	mmi.m_index = index;
	return mmi;
}
template <class T>
T findMax(T *arr, int N)
{

	T max = arr[0];
	int index = 0;
	for (int i = 1; i < N; i++)
	{
		if (max < arr[i])
		{
			max = arr[i];
			index = i;
		}
	}

	return max;
}
template <class T>


T findMin(T *arr, int N)
{

	T min = arr[0];
	int index = 0;
	for (int i = 1; i < N; i++)
	{
		if (min > arr[i])
		{
			min = arr[i];
			index = i;
		}
	}

	return min;
}

//center point, A
int getIangle(Point2i centPt, Point2i A)
{
	int i;
	for (i = 0; i < 8; i++){
		if ((A.x - centPt.x) == a8i[i] && (A.y - centPt.y) == a8j[i])
			return i;
	}
}

void initMain()
{
	system("color 2f");//设置控制台背影颜色
	//---------------------------------------------------------------
	//接口控制函数，1表示是，0表示否
	controlFuction();//是否添加噪音
	//---------------------------------------------------------------

}
double deviation(vector<float> v)
{
	double sum = std::accumulate(std::begin(v), std::end(v), 0.0);
	double mean = sum / v.size(); //均值  

	double accum = 0.0;
	std::for_each(
		std::begin(v), std::end(v), [&](const double d)
	{
		accum += (d - mean)*(d - mean);
	}
	);

	double stdev;
	stdev = sqrt(accum / (v.size() - 1)); //方差 deviation(偏差)

	return stdev;
}
bool isInsec(vector <int> &ind)//2-6
{
	vector <int> index = ind;
	int n = index.size();
	//处理0，7两种首尾情况
	if (index.at(0) == 0)
	{
		if (index.at(1) == 1)//+
		{
			if (n == 2)
			{
				return false;
			}
			else
			{
				if (n == index.at(n - 1) + 1)
				{
					return false;
				}

			}
		}
		else if (index.at(1) == 7)//-
		{
			if (n == 2)
			{
				return false;
			}
			else
			{
				if (n == 8 - index.at(n - 1) + 1)
				{
					return false;
				}

			}
		}

	}
	else if (index.at(0) == 7)
	{
		if (index.at(1) == 0)//+
		{
			if (n == 2)
			{
				return false;
			}
			else if (index.at(n - 1) == n - 2)//31,42,53,64[n,n-2]
			{
				return false;

			}

		}
		else if (index.at(1) == 6)//-
		{
			if (n == 2)
			{
				return false;
			}
			else if (7 - index.at(n - 1) + 1 == n)//31,42,53,64[n,n-2]
			{
				return false;

			}
		}
	}
	else//正常情况
	{
		if (index.at(1) - index.at(0) > 0)//+
		{
			if ((index.at(0) + n - 1) % 8 == index.at(n - 1))
			{
				return false;
			}
		}
		else//-
		{
			if ((index.at(0) - n + 9) % 8 == index.at(n - 1))//8 +i
			{
				return false;
			}
		}
	}


	return true;


}
int nSection(Point2i centPt)
{
	int low, high;
	int centVal, ptVal;
	int n1 = 0, n2 = 0, n3 = 0;
	float std1, std2, std3;
	float sum;
	float ave;
	vector<float> sec1, sec2, sec3;
	vector<int> ind1, ind2, ind3;
	int len1, len2, len3;
	//Point2i pt;
	centVal = g_calImg.at<float>(centPt.x, centPt.y);
	low = centVal - TH1;
	high = centVal + TH1;

	//区域划分
	for (int i = 0; i < 8; i++)
	{
		ptVal = g_calImg.at<float>(centPt.x + a8i[i], centPt.y + a8j[i]);

		if (ptVal<low)
		{
			n1++;
			sec1.push_back(ptVal);
			ind1.push_back(i);
		}
		else if (ptVal>high)
		{
			n3++;
			sec3.push_back(ptVal);
			ind3.push_back(i);
		}
		else
		{
			sec2.push_back(ptVal);
			ind2.push_back(i);
			n2++;
		}
	}

	//一、存在三个分区，可能是噪音或者缓冲区域
	if (n1&&n2&&n3)
	{
		//
		for (int k = 0; k < 8; k++)
		{
			/*g_edge[0].at<float>(centPt.x + a8i[k], centPt.y + a8j[k])=255;
			g_edge[1].at<float>(centPt.x + a8i[k], centPt.y + a8j[k]) = 255;

			g_edge[2].at<float>(centPt.x + a8i[k], centPt.y + a8j[k]) = 255;*/

			/*g_imgFlg.at<int>(centPt.x, centPt.y) = 11;
			g_imgFlg.at<int>(centPt.x + a8i[k], centPt.y + a8j[k]) = 11;*/

		}
		//g_calImg.at<float>(centPt.x, centPt.y) = 255;
		return 3;
	}
	//二、存在二个分区，可能是噪音或者边缘区域
	else if ((n1 + n2) == 8)//n1,n2共两个分区
	{
		if (!isTag(centPt) && n2 > 1 && n2 < 7)//&&!isTag(centPt) !isTag(centPt) &&
		{
			//判断是否为交叉区域
			bool insec = false;
			if (ind1.size() < ind2.size())
			{
				insec = isInsec(ind1);
			}
			else
				insec = isInsec(ind2);

			//判断最大连续的分区

			//处理交叉分区：断言其中一个分区为噪音
			if (0)//insec
			{
				//求n1,n2区域的方差值
				//sort(sec1.begin(), sec1.end());
				//sort(sec2.begin(), sec2.end());
				std1 = deviation(sec1);
				std2 = deviation(sec2);
				len1 = maxSeqence(ind1);
				len2 = maxSeqence(ind2);
				//差值较大的为噪音区域
				if (std1 < std2&&len1>len2)//sec2 为噪音
				{
					//用非噪音区域均值来平滑噪音点
					sum = std::accumulate(sec1.begin(), sec1.end(), 0.0);
					ave = sum / sec1.size();
					int n = sec2.size();

					for (int k = 0; k < n; k++)
					{
						g_calImg.at<float>(centPt.x + a8i[ind2.at(k)], centPt.y + a8j[ind2.at(k)]) = ave;
						g_nSectionFlg.at<int>(centPt.x + a8i[ind2.at(k)], centPt.y + a8j[ind2.at(k)]) = 1;
					}
					//填充中心点
					g_calImg.at<float>(centPt.x, centPt.y) = ave;
					g_nSectionFlg.at<int>(centPt.x, centPt.y) = 1;
				}
				else if (std1 > std2)//sec1 为噪音
				{
					//用非噪音区域均值来平滑噪音点
					sum = std::accumulate(sec2.begin(), sec2.end(), 0.0);
					ave = sum / sec2.size();
					int n = sec1.size();
					for (int k = 0; k < n; k++)
					{
						g_calImg.at<float>(centPt.x + a8i[ind1.at(k)], centPt.y + a8j[ind1.at(k)]) = ave;
						g_nSectionFlg.at<int>(centPt.x + a8i[ind1.at(k)], centPt.y + a8j[ind1.at(k)]) = 1;
					}
					//填充中心点
					g_calImg.at<float>(centPt.x, centPt.y) = ave;
					g_nSectionFlg.at<int>(centPt.x, centPt.y) = 1;
				}

			}
		}
		return 2;
	}
	else if ((n3 + n2) == 8)//n3,n2共两个分区
	{
		if (!isTag(centPt) && n2 > 1 && n2 < 7)//&&!isTag(centPt) !isTag(centPt) &&
		{
			//判断是否为交叉区域
			bool insec = false;
			if (ind3.size() < ind2.size())
			{
				insec = isInsec(ind3);
			}
			else
				insec = isInsec(ind2);

			//处理交叉分区：断言其中一个分区为噪音
			if (0)//insec
			{
				//求n1,n2区域的方差值
				//sort(sec1.begin(), sec1.end());
				//sort(sec2.begin(), sec2.end());
				std3 = deviation(sec3);
				std2 = deviation(sec2);
				len3 = maxSeqence(ind3);
				len2 = maxSeqence(ind2);
				//差值较大的为噪音区域
				if (std3 < std2&&len3>len2)//sec2 为噪音
				{
					//用非噪音区域均值来平滑噪音点
					sum = std::accumulate(sec3.begin(), sec3.end(), 0.0);
					ave = sum / sec3.size();
					int n = sec2.size();

					for (int k = 0; k < n; k++)
					{
						g_calImg.at<float>(centPt.x + a8i[ind2.at(k)], centPt.y + a8j[ind2.at(k)]) = ave;
						g_nSectionFlg.at<int>(centPt.x + a8i[ind2.at(k)], centPt.y + a8j[ind2.at(k)]) = 1;
					}
					//填充中心点
					g_calImg.at<float>(centPt.x, centPt.y) = ave;
					g_nSectionFlg.at<int>(centPt.x, centPt.y) = 1;
				}
				else if (std3 > std2)//sec3 为噪音
				{
					//用非噪音区域均值来平滑噪音点
					sum = std::accumulate(sec2.begin(), sec2.end(), 0.0);
					ave = sum / sec2.size();
					int n = sec3.size();
					for (int k = 0; k < n; k++)
					{
						g_calImg.at<float>(centPt.x + a8i[ind3.at(k)], centPt.y + a8j[ind3.at(k)]) = ave;
						g_nSectionFlg.at<int>(centPt.x + a8i[ind3.at(k)], centPt.y + a8j[ind3.at(k)]) = 1;
					}
					//填充中心点
					g_calImg.at<float>(centPt.x, centPt.y) = ave;
					g_nSectionFlg.at<int>(centPt.x, centPt.y) = 1;
				}

			}
		}
		return 2;
	}
	else if (n1 + n3 == 8)//此种类型定义为中心点是噪音
	{
		return 1;
	}
	else//全部相同或不同
	{

		return 1;
	}


	//即两个分区
	//return true;

}
int nSection1(Point2i centPt)
{
	int low, high;
	int centVal, ptVal;
	int n1 = 0, n2 = 0, n3 = 0;
	float std1, std2, std3;
	float sum1 = 0.0f, sum2 = 0, sum3 = 0;
	float avg1 = 0, avg2 = 0;
	float sum;
	float ave;
	vector<float> sec1, sec2, sec3;
	vector<int> ind1, ind2, ind3;
	int len1, len2, len3;
	bool big_val = false;
	//Point2i pt;
	centVal = g_calImg.at<float>(centPt.x, centPt.y);
	low = centVal - TH1;
	high = centVal + TH1;
	int x, y;
	//区域划分
	for (int i = 0; i < 8; i++)
	{
		ptVal = g_calImg.at<float>(centPt.x + a8i[i], centPt.y + a8j[i]);

		if (ptVal<low)
		{
			n1++;
			sec1.push_back(ptVal);
			ind1.push_back(i);
		}
		else if (ptVal>high)
		{
			n3++;
			sec3.push_back(ptVal);
			ind3.push_back(i);
		}
		else
		{
			sec2.push_back(ptVal);
			ind2.push_back(i);
			n2++;
		}
	}

	if (n2 > 0 && n2 < 8)
	{
		if (n1 > 0)
		{
			sum1 = std::accumulate(std::begin(sec1), std::end(sec1), 0.0);
		}
		if (n3 > 0)
		{
			sum3 = std::accumulate(std::begin(sec3), std::end(sec3), 0.0);
		}
		sum2 = std::accumulate(std::begin(sec2), std::end(sec2), 0.0);
		avg2 = sum2 / n2;

		avg1 = (sum1 + sum3) / (n1 + n3);
		if (avg2 >= avg1)
		{
			big_val = true;
		}

	}
	else
	{
		//g_nType.at<int>(centPt.x, centPt.y) = n2;
	}


	if (big_val)
	{
		g_nType.at<int>(centPt.x, centPt.y) = n2;
	}
	//g_nType.at<int>(centPt.x, centPt.y) = n2;



	if (n2 == 0)
	{
		//标记中心点噪音

	}
	else//n2>0
	{
		if (n1 > n3&&n3 > 0)
		{
			//n1 为一个区域

			//n2 为一个区域

			//判断AB区，A区为大
			//n3 为噪音区域
			for (int n = 0; n < n3; n++)
			{
				x = centPt.x + a8i[ind3.at(n)];
				y = centPt.y + a8j[ind3.at(n)];
				//g_nTypeFlag.at<int>(x, y)=1;
				//tagTypeColor(255, 0, 255, x, y);
			}

		}
		else if (n1<n3&&n1>0)
		{
			//n3 为一个区域

			//n2 为一个区域


			//n1 为噪音区域
			for (int n = 0; n < n1; n++)
			{
				x = centPt.x + a8i[ind3.at(n)];
				y = centPt.y + a8j[ind3.at(n)];
				//g_nTypeFlag.at<int>(x, y)=1;
				//tagTypeColor(255, 0, 255, x, y);
			}
		}
		else if (n1 == n3)
		{
			//判断标准差，标准差大的为噪音区域

		}

	}
	//即两个分区
	return 2;

}
void tagTypeColor(int b, int g, int r, int i, int j)
{
	nTypeEdge[0].at < int >(i, j) = b;
	nTypeEdge[1].at < int >(i, j) = g;
	nTypeEdge[2].at < int >(i, j) = r;
}
//Input：两个分区的相应数组下标2,3,4,7
//Output:找到最长连续的下标，为几返回值为几，相同及没有连续相同的返0;
int maxSeqence(vector <int> &ind)
{
	int n = ind.size();
	int x0, x1, x11;
	int maxLength = 0;
	int max[6] = { 0, 0, 0, 0, 0, 0 };

	for (int k = 0; k < n; k++)
	{
		x0 = ind.at(k);

		for (int i = 1; i < n - k; i++)//传递的Ind长度为2-6
		{
			x1 = ind.at(k + i);//
			if ((x0 + i) % 8 == x1 || (x0 + 8 - i) % 8 == x1)
			{
				maxLength = i + 1;
			}
			else
				break;

		}
		max[k] = maxLength;
		//max.push_back(maxLength);


	}
	maxLength = findMax(max, 6);
	return maxLength;//返回连续长度
}
void controlFuction()
{

}

void getTypeEdge()
{
	Mat display_typeEdge(gray.rows, gray.cols, CV_8UC3, Scalar::all(0));
	for (int i = 1; i < gray.rows - 1; i++)
	{
		for (int j = 1; j < gray.cols - 1; j++)
		{

			switch (g_nType.at<int>(i, j))
			{
			case 0://白色点，孤立点
				tagTypeColor(255, 255, 255, i, j);
				g_type.at<int>(i, j) += 0;
				break;
			case 1://粉色
				tagTypeColor(255, 0, 255, i, j);//
				g_type.at<int>(i, j) += 1;
				break;
			case  2://浅青
				tagTypeColor(150, 150, 0, i, j);
				g_type.at<int>(i, j) += 2;
				break;
			case 3:
				//blue
				nTypeEdge[0].at < int >(i, j) = 255;
				nTypeEdge[1].at < int >(i, j) = 0;
				nTypeEdge[2].at < int >(i, j) = 0;
				g_type.at<int>(i, j) += 3;
				break;
			case 4:
				//green
				nTypeEdge[0].at < int >(i, j) = 0;
				nTypeEdge[1].at < int >(i, j) = 255;
				nTypeEdge[2].at < int >(i, j) = 0;
				g_type.at<int>(i, j) += 4;
				break;
			case 5:
				//red
				nTypeEdge[0].at < int >(i, j) = 0;
				nTypeEdge[1].at < int >(i, j) = 0;
				nTypeEdge[2].at < int >(i, j) = 255;
				g_type.at<int>(i, j) += 5;
				break;
			case 6://青色
				nTypeEdge[0].at < int >(i, j) = 255;
				nTypeEdge[1].at < int >(i, j) = 255;
				nTypeEdge[2].at < int >(i, j) = 0;
				g_type.at<int>(i, j) += 6;
				break;
			case 7:
				//0,255,255为黄色
				nTypeEdge[0].at < int >(i, j) = 0;
				nTypeEdge[1].at < int >(i, j) = 255;
				nTypeEdge[2].at < int >(i, j) = 255;
				g_type.at<int>(i, j) = +7;
				break;
			case 8:
				tagTypeColor(0, 0, 0, i, j);

				g_type.at<int>(i, j) += 8;
				break;

			default:
				//cout << "nType error!\n";
				break;

			}

		}
	}
	//outxls(nTypeEdge.at(0), "./output/ntype.xls");

	nTypeEdge[0].convertTo(nTypeEdge[0], CV_8UC1);
	nTypeEdge[1].convertTo(nTypeEdge[1], CV_8UC1);
	nTypeEdge[2].convertTo(nTypeEdge[2], CV_8UC1);

	merge(nTypeEdge, display_typeEdge);
	imshow("星型图", display_typeEdge);
	imwrite("./output/ntype.bmp", display_typeEdge);

	//display_typeEdge=Mat::zeros(gray.rows, gray.cols, CV_8UC3);
	//g_nType = Mat::zeros(gray.rows, gray.cols, CV_32S);//再次初始化
}

void deCONTRADICTION(int row, int col)
{
	int cenV, tmpV, num = 0, sum = 0;
	float sv[8] = { 0 };
	int index = -1;
	Point2i centPt;
	for (int i = 1; i < row - 1; i++)
	{
		for (int j = 1; j < col - 1; j++)
		{
			cenV = g_calImg.at<float>(i, j);

			for (int k = 0; k < 8; k++)
			{
				tmpV = g_calImg.at<float>(i + a8i[k], j + a8j[k]);
				if (abs(cenV - tmpV) <= TH1)
				{
					num++;
					index = k;
				}
				else//将和中心点不同的值相加，7或8
				{
					sum += tmpV;
				}


			}
			if (num == 0)
			{
				/*for (int k = 0; k < 8; k++)
				{
				g_edge[0].at<float>(i + a8i[k], j + a8j[k]) = 255;
				g_edge[1].at<float>(i + a8i[k], j + a8j[k]) = 0;
				g_edge[2].at<float>(i + a8i[k], j + a8j[k]) = 0;
				g_imgFlg.at<int>(i + a8i[k], j + a8j[k]) = 11;

				}*/
				//g_imgFlg.at<int>(i, j) = 11;
				//g_calImg.at<float>(i, j) = sum / 8;
			}
			else if (num == 1)
			{
				//g_calImg.at<float>(i, j) = sum / 7;
				//g_calImg.at<float>(i + a8i[index], j + a8j[index]) = sum / 7;
			}
			else if (num == 8)
			{
				//g_imgFlg.at<int>(i,j) = 11;
				for (int k = 0; k < 8; k++)
				{

					//g_imgFlg.at<int>(i + a8i[k], j + a8j[k]) = 11;

				}
			}

			//分区判断类型
			centPt.x = i;
			centPt.y = j;
			//nSection(centPt);
			num = 0;
			sum = 0;
		}
	}
}

void salt(cv::Mat& image){
	//unsigned int seed; /*申明初始化器的种子，注意是usigned int 型的*//
	srand(time(0));
	int flgedge;
	for (int k = 0; k < 100000; k++)
	{
		int i = rand() % image.cols;
		int j = rand() % image.rows;

		flgedge = g_imgFlg.at<int>(j, i);

		//添加边缘噪音
		if (!outOfrange(j, i) && flgedge)//&&g_imgFlg.at<int>(j,i)
		{
			if (image.channels() == 1)
			{
				image.at<uchar>(j, i) = 255;
			}
			else
			{
				image.at<cv::Vec3b>(j, i)[0] = (uchar)(rand() % 256);  // (uchar)(rand() % 256)
				image.at<cv::Vec3b>(j, i)[1] = (uchar)(rand() % 256);  // (uchar)(rand() % 256)
				image.at<cv::Vec3b>(j, i)[2] = (uchar)(rand() % 256);  // (uchar)(rand() % 256)
			}
		}
		//else if (k % 20 == 0)
		//{
		//	//添加非边缘噪音

		//	if (image.channels() == 1)
		//	{
		//		image.at<uchar>(j, i) = 255;
		//	}
		//	else
		//	{
		//		image.at<cv::Vec3b>(j, i)[0] = (uchar)(rand() % 256);  // (uchar)(rand() % 256)
		//		image.at<cv::Vec3b>(j, i)[1] = (uchar)(rand() % 256);  // (uchar)(rand() % 256)
		//		image.at<cv::Vec3b>(j, i)[2] = (uchar)(rand() % 256);  // (uchar)(rand() % 256)
		//	}

		//}


	}
	//imshow("dd", image);
	imwrite("./output/CONTRADICTIONPicture.bmp", image);
}


//法向量求反向并分为8类1-8
Point2i normalToXY(float nv)
{

	Point2i P;
	int nV = (int)(10 * nv);
	switch (nV)
	{
	case 0:
	case 5:
	case 10:
		P.x = -1; P.y = 1;
		break;
	case 15:
	case 20:
		P.x = -1; P.y = 0;
		break;
	case 25:
	case 30:
		P.x = -1; P.y = -1;
		break;
	case 35:
	case 40:
		P.x = 0; P.y = -1;
		break;
	case 45:
	case 50:
		P.x = 1; P.y = -1;
		break;
	case 55:
	case 60:
		P.x = 1; P.y = 0;
		break;
	case 65:
	case 70:
		P.x = 1; P.y = 1;
		break;
	case 75:
	case 80:
		P.x = 0; P.y = 1;
		break;
	case -10:
		break;
	default:
		cout << "nV=" << nV << endl;
		cout << "nv==" << nv << endl;
		printf("Error==================!\n");

	}
	return P;
}

float sum(Mat mat, int n)
{
	float sum = 0.0;
	int row = mat.rows;
	for (int i = n; i <= row; i++)
	{
		sum += mat.at<float>(i, 0);
	}
	return sum;
}



