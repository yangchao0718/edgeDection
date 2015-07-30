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
float absAB_BGR(Point2i A, int i, int j)
{
	float tmp;
	tmp = abs(srcBGR[0].at<float>(A.x, A.y) - srcBGR[0].at<float>(i, j)) +
		abs(srcBGR[1].at<float>(A.x, A.y) - srcBGR[1].at<float>(i, j)) +
		abs(srcBGR[2].at<float>(A.x, A.y) - srcBGR[2].at<float>(i, j));
	return tmp;
}

//输入：一个点centerPt
//输出：判断此点是否为边缘点，如果是则输出相邻的两个边缘点A,B,与中心点不同的点AC、BC
#if (START==1)
bool findStartPt1(Point2i centerPt, Point2i &A, Point2i &B, Point2i &AC, Point2i &BC, int &direction)
{
	bool  flagA, flagB;//标记是否找到A/B点
	flagA = flagB = false;
	int nType;
	Point2i stPt, E;
	int i, j;
	int bdA, bdB, bdTmp;
	int dx, dy;
	i = centerPt.x;
	j = centerPt.y;
	//与中心边缘点相邻的两个边缘点A，B，
	int indext;
	bool kflagMissed = false;
	Point2i missedPt;
	int dA, dB, dAC, dBC;
	float CV;
	int flagkk = g_imgFlg.at<int>(centerPt.x, centerPt.y);
	if (flagkk)
		return false;

	//下面函数用来找到中心点的相同的起止点A、B及不同的起止点AC、BC
	if (rgb)//彩色图像
	{
		//如果第一个点和中心点相同，则找连续两个和中点不相同的点
		if (absAB_BGR(centerPt, i, j + 1) <= TH2){
			for (indext = 1; indext<8; indext++)
			{
				if (absAB_BGR(centerPt, i + a8i[indext], j + a8j[indext])>TH2)
				{
					if (absAB_BGR(centerPt, i + a8i[indext + 1], j + a8j[indext + 1]) > TH2){
						A.x = i + a8i[indext - 1];
						A.y = j + a8j[indext - 1];
						AC.x = i + a8i[indext];
						AC.y = j + a8j[indext];
						flagA = true;
						break;
					}
					else{
						indext += 1;
					}
				}
			}
			for (indext = 7; indext > 0; indext--)
			{
				if (absAB_BGR(centerPt, i + a8i[indext], j + a8j[indext]) > TH2)
				{
					int ii = indext - 1;
					if (ii<0)  ii += 8;
					if (absAB_BGR(centerPt, i + a8i[ii], j + a8j[ii]) > TH2){
						B.x = i + a8i[indext + 1];
						B.y = j + a8j[indext + 1];
						BC.x = i + a8i[indext];
						BC.y = j + a8j[indext];
						if (AC == BC)
						{
							return false;
						}
						flagB = true;
						break;
					}
					else{
						indext -= 1;
					}
				}
			}
		}
		else{//如果第一个点和中心点不相同，则找连续两个和中点相同的点
			for (indext = 1; indext < 8; indext++)
			{
				if (absAB_BGR(centerPt, i + a8i[indext], j + a8j[indext]) <= TH2)
				{
					if (absAB_BGR(centerPt, i + a8i[indext + 1], j + a8j[indext + 1]) <= TH2)
					{
						A.x = i + a8i[indext];
						A.y = j + a8j[indext];
						AC.x = i + a8i[indext - 1];
						AC.y = j + a8j[indext - 1];
						flagA = true;

						break;
					}
					else
					{
						indext += 1;
					}

				}
			}
			for (indext = 7; indext > 0; indext--)
			{
				if (absAB_BGR(centerPt, i + a8i[indext], j + a8j[indext]) <= TH2)
				{
					int ii = indext - 1;
					if (ii < 0)  ii += 8;
					if (absAB_BGR(centerPt, i + a8i[ii], j + a8j[ii]) <= TH2)
					{
						B.x = i + a8i[indext];
						B.y = j + a8j[indext];
						BC.x = i + a8i[indext + 1];
						BC.y = j + a8j[indext + 1];
						if (AC == BC)
						{
							return false;
						}
						flagB = true;
						break;
					}
					else
					{
						indext -= 1;
					}

				}
			}
		}

	}

	//灰度图像
	else
	{
		//如果第一个点和中心点相同，则找连续两个和中点不相同的点
		CV = g_calImg.at<float>(centerPt.x, centerPt.y);
		if (abs(CV - g_calImg.at<float>(i, j + 1)) <= TH1){
			for (indext = 1; indext<8; indext++)
			{
				if (abs(CV - g_calImg.at<float>(i + a8i[indext], j + a8j[indext]))>TH1)
				{
					if (abs(CV - g_calImg.at<float>(i + a8i[indext + 1], j + a8j[indext + 1])) > TH1){
						A.x = i + a8i[indext - 1];
						A.y = j + a8j[indext - 1];
						AC.x = i + a8i[indext];
						AC.y = j + a8j[indext];
						flagA = true;
						break;
					}
					else{
						indext += 1;
					}
				}
			}
			for (indext = 7; indext > 0; indext--)
			{
				if (abs(CV - g_calImg.at<float>(i + a8i[indext], j + a8j[indext])) > TH1)
				{
					int ii = indext - 1;
					if (ii<0)  ii += 8;
					if (abs(CV - g_calImg.at<float>(i + a8i[ii], j + a8j[ii])) > TH1){
						B.x = i + a8i[indext + 1];
						B.y = j + a8j[indext + 1];
						BC.x = i + a8i[indext];
						BC.y = j + a8j[indext];
						if (AC == BC)
						{
							return false;
						}
						flagB = true;
						break;
					}
					else{
						indext -= 1;
					}
				}
			}
		}
		else{//如果第一个点和中心点不相同，则找连续两个和中点相同的点
			for (indext = 1; indext < 8; indext++)
			{
				if (abs(CV - g_calImg.at<float>(i + a8i[indext], j + a8j[indext])) <= TH1)
				{
					if (abs(CV - g_calImg.at<float>(i + a8i[indext + 1], j + a8j[indext + 1])) <= TH1)
					{
						A.x = i + a8i[indext];
						A.y = j + a8j[indext];
						AC.x = i + a8i[indext - 1];
						AC.y = j + a8j[indext - 1];
						flagA = true;

						break;
					}
					else
					{
						indext += 1;
					}

				}
			}
			for (indext = 7; indext > 0; indext--)
			{
				if (abs(CV - g_calImg.at<float>(i + a8i[indext], j + a8j[indext])) <= TH1)
				{
					int ii = indext - 1;
					if (ii < 0)  ii += 8;
					if (abs(CV - g_calImg.at<float>(i + a8i[ii], j + a8j[ii])) <= TH1)
					{
						B.x = i + a8i[indext];
						B.y = j + a8j[indext];
						BC.x = i + a8i[indext + 1];
						BC.y = j + a8j[indext + 1];
						if (AC == BC)
						{
							return false;
						}
						flagB = true;
						break;
					}
					else
					{
						indext -= 1;
					}

				}
			}
		}

	}

	//计算Direction,若如下叉积小于0，则从半平面的顺时针开始查找
	direction = (A.x - centerPt.x)*(AC.y - centerPt.y) - (A.y - centerPt.y)*(AC.x - centerPt.x);
	dx = A.x - centerPt.x;
	dy = A.y - centerPt.y;

	//得到了A、B后，计算出边缘类型
	//把A、B点转为相对下标来求得连续长度作为边缘类型
	if (outOfrange(A.x, A.y) || outOfrange(B.x, B.y) || outOfrange(AC.x, AC.y) || outOfrange(BC.x, BC.y))
	{
		return false;//防止越界
	}
	if (g_imgFlg.at<int>(A.x, A.y) || g_imgFlg.at<int>(B.x, B.y) || g_imgFlg.at<int>(AC.x, AC.y) || g_imgFlg.at<int>(BC.x, BC.y))
		return false;//已经处理过


	dA = getIangle(centerPt, A);
	dB = getIangle(centerPt, B);
	nType = lengthAB(dA, dB, direction);
	bool isStartPt = startPoint(centerPt.x, centerPt.y);

	if (flagA&&flagB&&nType > 2 && nType < 7 && isStartPt)//nSection(centerPt)  && isStartPt&&g_maxGrad.at<float>(i, j) >= 120
	{
		//判断两个区域是否为纯净区
		//输入：centerPt,A,B AC,BC 
		//输出：true or false
		//A-B区域是否<TH，AC—BC是否>TH
		//*/没有扩展到彩色图像中
		int dA = getIangle(centerPt, A);
		int dB = getIangle(centerPt, B);
		int dAC = getIangle(centerPt, AC);
		int dBC = getIangle(centerPt, BC);
		int tmpV;
		int tmpC;
		tmpC = g_calImg.at<float>(centerPt.x, centerPt.y);
		//平滑噪音点
		if (direction > 0)
		{
			while ((dA) != dB)
			{
				dA--;
				if (dA == -1)
				{
					dA += 8;
				}
				tmpV = g_calImg.at<float>(centerPt.x + a8i[dA], centerPt.y + a8j[dA]);
				if (abs(tmpV - tmpC) > TH1)
				{
					//g_calImg.at<float>(centerPt.x + a8i[dA], centerPt.y + a8j[dA]) = tmpC;
					return false;
				}
				if (dA - 1 != dB)
				{
					//去除和D点相邻的点，避免标记了点，后续不能粥延伸：参见论文/每天任务/今天问题
					//g_imgFlg.at<float>(centerPt.x + a8i[dA], centerPt.y + a8j[dA]) = 22;
				}
			}
			while ((dAC) != dBC)
			{
				dAC++;
				if (dAC == 8)
				{
					dAC -= 8;
				}
				tmpV = g_calImg.at<float>(centerPt.x + a8i[dAC], centerPt.y + a8j[dAC]);
				if (abs(tmpV - tmpC) <= TH1)
				{
					//g_calImg.at<float>(centerPt.x + a8i[dAC], centerPt.y + a8j[dAC]) = tmpC;
					return false;
				}
			}

		}
		else
		{
			while ((dA) != dB)
			{
				dA++;
				if (dA == 8)
				{
					dA -= 8;
				}
				tmpV = g_calImg.at<float>(centerPt.x + a8i[dA], centerPt.y + a8j[dA]);
				if (abs(tmpV - tmpC) > TH1)
				{
					//g_calImg.at<float>(centerPt.x + a8i[dA], centerPt.y + a8j[dA]) = tmpC;
					return false;
				}
				if (dA + 1 != dB)
				{
					//去除和D点相邻的点，避免标记了点，后续不能粥延伸：参见论文/每天任务/今天问题
					//g_imgFlg.at<float>(centerPt.x + a8i[dA], centerPt.y + a8j[dA]) = 22;
				}
			}
			while ((dAC) != dBC)
			{
				dAC--;
				if (dAC == -1)
				{
					dAC += 8;
				}
				tmpV = g_calImg.at<float>(centerPt.x + a8i[dAC], centerPt.y + a8j[dAC]);
				if (abs(tmpV - tmpC) <= TH1)
				{
					//g_calImg.at<float>(centerPt.x + a8i[dAC], centerPt.y + a8j[dAC]) = tmpC;
					return false;
				}
			}
		}
		//*/


		if (!g_imgFlg.at<int>(centerPt.x, centerPt.y))
		{

			if (green)
			{
				g_edge[0].at<float>(centerPt.x, centerPt.y) = 0;
				g_edge[1].at<float>(centerPt.x, centerPt.y) = 255;
				g_edge[2].at<float>(centerPt.x, centerPt.y) = 0;
				g_imgFlg.at<int>(centerPt.x, centerPt.y) = 5;
			}


			//补上起点的AC-BC
			//kflagMissed = GetMissedPoint(centerPt, AC, BC, direction, missedPt);

			//dAC = getIangle(centerPt, AC);
			//dBC = getIangle(centerPt, BC);
			//与中心点不相同点比中心点小的标记为红色

			float comR;//比较大小
			if (rgb)
			{
				comR = (srcBGR[0].at<float>(centerPt.x, centerPt.y) - srcBGR[0].at<float>(AC.x, AC.y)) +
					(srcBGR[1].at<float>(centerPt.x, centerPt.y) - srcBGR[1].at<float>(AC.x, AC.y)) +
					(srcBGR[2].at<float>(centerPt.x, centerPt.y) - srcBGR[2].at<float>(AC.x, AC.y));
			}
			else
			{
				g_avgACBC = (g_calImg.at<float>(AC.x, AC.y) + g_calImg.at<float>(BC.x, BC.y)) / 2;
				comR = g_calImg.at<float>(centerPt.x, centerPt.y) - g_avgACBC;
				g_calImg.at<float>(AC.x, AC.y) = g_avgACBC;
				g_calImg.at<float>(BC.x, BC.y) = g_avgACBC;
			}
			if (comR > 0){
				if (virtual_edge)//大边
				{
					g_edge[0].at<float>(A.x, A.y) = 0;
					g_edge[1].at<float>(A.x, A.y) = 0;
					g_edge[2].at<float>(A.x, A.y) = 100;

					g_edge[0].at<float>(B.x, B.y) = 0;
					g_edge[1].at<float>(B.x, B.y) = 0;
					g_edge[2].at<float>(B.x, B.y) = 100;
					if (g_imgFlg.at<int>(centerPt.x, centerPt.y) == 0)
					{
						g_edge[0].at<float>(centerPt.x, centerPt.y) = 0;
						g_edge[1].at<float>(centerPt.x, centerPt.y) = 0;
						g_edge[2].at<float>(centerPt.x, centerPt.y) = 100;
					}


				}

				g_imgFlg.at<int>(A.x, A.y) = 2;
				g_imgFlg.at<int>(centerPt.x, centerPt.y) = 2;
				g_imgFlg.at<int>(B.x, B.y) = 2;

			}
			else{//与中心点不相同点且比中心点大的标记为黄色

				//*
				if (real_edge)//小边
				{
					bluePt++;
					g_edge[0].at<float>(A.x, A.y) = 0;
					g_edge[1].at<float>(A.x, A.y) = 255;
					g_edge[2].at<float>(A.x, A.y) = 255;
					if (g_imgFlg.at<int>(centerPt.x, centerPt.y) == 0)
					{
						g_edge[0].at<float>(centerPt.x, centerPt.y) = 0;
						g_edge[1].at<float>(centerPt.x, centerPt.y) = 255;
						g_edge[2].at<float>(centerPt.x, centerPt.y) = 255;
					}
					g_edge[0].at<float>(B.x, B.y) = 0;
					g_edge[1].at<float>(B.x, B.y) = 255;
					g_edge[2].at<float>(B.x, B.y) = 255;
				}

				g_imgFlg.at<int>(A.x, A.y) = 1;
				g_imgFlg.at<int>(centerPt.x, centerPt.y) = 1;
				g_imgFlg.at<int>(B.x, B.y) = 1;

			}

		}//*/


		g_thresold.at<float>(centerPt.x, centerPt.y) = getDth(A, centerPt, AC, B, BC, -direction);
		TH_D = g_thresold.at<float>(centerPt.x, centerPt.y);
		//传递的方向和getth的方向相反。geth用的是延伸方向，而此时是起点方向。

		//if (g_thresold.at<float>(centerPt.x, centerPt.y) > 0)
		//{
		//	cout << "ll=" << g_thresold.at<float>(centerPt.x, centerPt.y) << endl;
		//	return true;//如果找到A、B两个边缘点则开始调用延伸追踪函数TraceEdgeTwoBoundary
		//}
		//else
		//	return false;

		return true;//如果找到A、B两个边缘点则开始调用延伸追踪函数TraceEdgeTwoBoundary
	}
	else
		return false;


}

#endif
bool findStartPt3(Point2i centerPt, Point2i &A, Point2i &B, Point2i &AC, Point2i &BC, int &direction, bool& min_max){
	//find a start edge without noice,which include at leat 3 same type edgepoints
	//1 find a 5_type point C and its adjacent points:A && B
	bool  flagA, flagB;//标记是否找到A/B点
	flagA = flagB = false;
	int nType;
	Point2i stPt, E;
	int i, j;
	int bdA, bdB, bdTmp;
	int dx, dy;
	i = centerPt.x;
	j = centerPt.y;
	//与中心边缘点相邻的两个边缘点A，B，
	int indext;
	bool kflagMissed = false;
	Point2i missedPt;
	int dA, dB, dAC, dBC;
	float CV;
	int flagkk = g_imgFlg.at<int>(centerPt.x, centerPt.y);
	if (flagkk)
		return false;
	int typeA, typeB;
	//To centerPoint C: find A,B,AC,BC
	if (rgb)//to color image
	{
		//如果第一个点和中心点相同，则找连续两个和中点不相同的点
		if (absAB_BGR(centerPt, i, j + 1) <= TH2){
			for (indext = 1; indext<8; indext++)
			{
				if (absAB_BGR(centerPt, i + a8i[indext], j + a8j[indext])>TH2)
				{
					if (absAB_BGR(centerPt, i + a8i[indext + 1], j + a8j[indext + 1]) > TH2){
						A.x = i + a8i[indext - 1];
						A.y = j + a8j[indext - 1];
						AC.x = i + a8i[indext];
						AC.y = j + a8j[indext];
						flagA = true;
						break;
					}
					else{
						indext += 1;
					}
				}
			}
			for (indext = 7; indext > 0; indext--)
			{
				if (absAB_BGR(centerPt, i + a8i[indext], j + a8j[indext]) > TH2)
				{
					int ii = indext - 1;
					if (ii<0)  ii += 8;
					if (absAB_BGR(centerPt, i + a8i[ii], j + a8j[ii]) > TH2){
						B.x = i + a8i[indext + 1];
						B.y = j + a8j[indext + 1];
						BC.x = i + a8i[indext];
						BC.y = j + a8j[indext];
						if (AC == BC)
						{
							return false;
						}
						flagB = true;
						break;
					}
					else{
						indext -= 1;
					}
				}
			}
		}
		else{//如果第一个点和中心点不相同，则找连续两个和中点相同的点
			for (indext = 1; indext < 8; indext++)
			{
				if (absAB_BGR(centerPt, i + a8i[indext], j + a8j[indext]) <= TH2)
				{
					if (absAB_BGR(centerPt, i + a8i[indext + 1], j + a8j[indext + 1]) <= TH2)
					{
						A.x = i + a8i[indext];
						A.y = j + a8j[indext];
						AC.x = i + a8i[indext - 1];
						AC.y = j + a8j[indext - 1];
						flagA = true;

						break;
					}
					else
					{
						indext += 1;
					}

				}
			}
			for (indext = 7; indext > 0; indext--)
			{
				if (absAB_BGR(centerPt, i + a8i[indext], j + a8j[indext]) <= TH2)
				{
					int ii = indext - 1;
					if (ii < 0)  ii += 8;
					if (absAB_BGR(centerPt, i + a8i[ii], j + a8j[ii]) <= TH2)
					{
						B.x = i + a8i[indext];
						B.y = j + a8j[indext];
						BC.x = i + a8i[indext + 1];
						BC.y = j + a8j[indext + 1];
						if (AC == BC)
						{
							return false;
						}
						flagB = true;
						break;
					}
					else
					{
						indext -= 1;
					}

				}
			}
		}

	}

	//to gray image:
	else
	{
		//如果第一个点和中心点相同，则找连续两个和中点不相同的点
		CV = g_calImg.at<float>(centerPt.x, centerPt.y);
		if (abs(CV - g_calImg.at<float>(i, j + 1)) <= TH1){
			for (indext = 1; indext<8; indext++)
			{
				if (abs(CV - g_calImg.at<float>(i + a8i[indext], j + a8j[indext]))>TH1)
				{
					A.x = i + a8i[indext - 1];
					A.y = j + a8j[indext - 1];
					AC.x = i + a8i[indext];
					AC.y = j + a8j[indext];
					flagA = true;
					break;
				}
			}
			for (indext = 7; indext > 0; indext--)
			{
				if (abs(CV - g_calImg.at<float>(i + a8i[indext], j + a8j[indext])) > TH1)
				{
					B.x = i + a8i[indext + 1];
					B.y = j + a8j[indext + 1];
					BC.x = i + a8i[indext];
					BC.y = j + a8j[indext];
					flagB = true;
					break;

				}
			}
		}
		else{//如果第一个点和中心点不相同，则找连续两个和中点相同的点
			for (indext = 1; indext < 8; indext++)
			{
				if (abs(CV - g_calImg.at<float>(i + a8i[indext], j + a8j[indext])) <= TH1)
				{
					A.x = i + a8i[indext];
					A.y = j + a8j[indext];
					AC.x = i + a8i[indext - 1];
					AC.y = j + a8j[indext - 1];
					flagA = true;

					break;
				}
			}
			for (indext = 7; indext > 0; indext--)
			{
				if (abs(CV - g_calImg.at<float>(i + a8i[indext], j + a8j[indext])) <= TH1)
				{

					B.x = i + a8i[indext];
					B.y = j + a8j[indext];
					BC.x = i + a8i[indext + 1];
					BC.y = j + a8j[indext + 1];
					flagB = true;
					break;

				}
			}
		}

	}

	//计算Direction,若如下叉积小于0，则从半平面的顺时针开始查找
	direction = (A.x - centerPt.x)*(AC.y - centerPt.y) - (A.y - centerPt.y)*(AC.x - centerPt.x);
	dx = A.x - centerPt.x;
	dy = A.y - centerPt.y;

	//得到了A、B后，计算出边缘类型
	//把A、B点转为相对下标来求得连续长度作为边缘类型
	if (outOfrange(A.x, A.y) || outOfrange(B.x, B.y) || outOfrange(AC.x, AC.y) || outOfrange(BC.x, BC.y))
	{
		return false;//防止越界
	}
	if (g_imgFlg.at<int>(A.x, A.y) || g_imgFlg.at<int>(B.x, B.y) || g_imgFlg.at<int>(AC.x, AC.y) || g_imgFlg.at<int>(BC.x, BC.y))
		return false;//已经处理过


	dA = getIangle(centerPt, A);
	dB = getIangle(centerPt, B);
	nType = lengthAB(dA, dB, direction);


	bool isStartPt = startPoint(centerPt.x, centerPt.y);

	if (flagA && flagB  && isStartPt && (nType == 3 || nType == 7)&&g_maxGrad.at<float>(centerPt.x,centerPt.y)>180)//nSection(centerPt)  && isStartPt&&g_maxGrad.at<float>(i, j) >= 120   
	{
		//判断两个区域是否为纯净区
		//输入：centerPt,A,B AC,BC 
		//输出：true or false
		//A-B区域是否<TH，AC—BC是否>TH
		//*/没有扩展到彩色图像中
		int dA = getIangle(centerPt, A);
		int dB = getIangle(centerPt, B);
		int dAC = getIangle(centerPt, AC);
		int dBC = getIangle(centerPt, BC);
		int tmpV;
		int tmpC;
		tmpC = g_calImg.at<float>(centerPt.x, centerPt.y);
		//is pure:else smooth noice point
		if (direction > 0)
		{
			while ((dA) != dB)
			{
				dA--;
				if (dA == -1)
				{
					dA += 8;
				}
				tmpV = g_calImg.at<float>(centerPt.x + a8i[dA], centerPt.y + a8j[dA]);
				if (abs(tmpV - tmpC) > TH1)
				{
					//g_calImg.at<float>(centerPt.x + a8i[dA], centerPt.y + a8j[dA]) = tmpC;
					return false;
				}
				if (dA - 1 != dB)
				{
					//去除和D点相邻的点，避免标记了点，后续不能粥延伸：参见论文/每天任务/今天问题
					//g_imgFlg.at<float>(centerPt.x + a8i[dA], centerPt.y + a8j[dA]) = 22;
				}
			}
			while ((dAC) != dBC)
			{
				dAC++;
				if (dAC == 8)
				{
					dAC -= 8;
				}
				tmpV = g_calImg.at<float>(centerPt.x + a8i[dAC], centerPt.y + a8j[dAC]);
				if (abs(tmpV - tmpC) <= TH1)
				{
					//g_calImg.at<float>(centerPt.x + a8i[dAC], centerPt.y + a8j[dAC]) = tmpC;
					return false;
				}
			}

		}
		else
		{
			while ((dA) != dB)
			{
				dA++;
				if (dA == 8)
				{
					dA -= 8;
				}
				tmpV = g_calImg.at<float>(centerPt.x + a8i[dA], centerPt.y + a8j[dA]);
				if (abs(tmpV - tmpC) > TH1)
				{
					//g_calImg.at<float>(centerPt.x + a8i[dA], centerPt.y + a8j[dA]) = tmpC;
					return false;
				}
				if (dA + 1 != dB)
				{
					//去除和D点相邻的点，避免标记了点，后续不能粥延伸：参见论文/每天任务/今天问题
					//g_imgFlg.at<float>(centerPt.x + a8i[dA], centerPt.y + a8j[dA]) = 22;
				}
			}
			while ((dAC) != dBC)
			{
				dAC--;
				if (dAC == -1)
				{
					dAC += 8;
				}
				tmpV = g_calImg.at<float>(centerPt.x + a8i[dAC], centerPt.y + a8j[dAC]);
				if (abs(tmpV - tmpC) <= TH1)
				{
					//g_calImg.at<float>(centerPt.x + a8i[dAC], centerPt.y + a8j[dAC]) = tmpC;
					return false;
				}
			}
		}
		//*/


		//calculate typeA,typeB
		int dCA = getIangle(A, centerPt);//A is centpt
		int dCB = getIangle(B, centerPt);
		int xA, xB, yA, yB;
		bool ptA5 = false, ptB5 = false;
		vector<float> ptA, ptB;
		if (direction > 0){
			for (int k = 3; k <= 5; k++)
			{
				xA = A.x + a8i[(dCA + k) % 8];
				yA = A.y + a8j[(dCA + k) % 8];
				//assert(dCB-k>0);
				xB = B.x + a8i[(dCB - k + 8) % 8];

				yB = B.y + a8j[(dCB - k + 8) % 8];
				ptA.push_back(g_calImg.at<float>(xA, yA));
				ptB.push_back(g_calImg.at<float>(xB, yB));
			}

		}
		else{
			for (int k = 3; k <= 5; k++)
			{
				xA = A.x + a8i[(dCA - k + 8) % 8];
				yA = A.y + a8j[(dCA - k + 8) % 8];
				xB = B.x + a8i[(dCB + k) % 8];
				yB = B.y + a8j[(dCB + k) % 8];
				ptA.push_back(abs(g_calImg.at<float>(xA, yA) - g_calImg.at<float>(centerPt.x, centerPt.y)));
				ptB.push_back(abs(g_calImg.at<float>(xB, yB) - g_calImg.at<float>(centerPt.x, centerPt.y)));
			}

		}

		//ptA,ptB's type are 5.complete...after...add noice
		if (ptA.at(0) <= TH1&&ptA.at(1) <= TH1&&ptA.at(2) > TH1)
		{
			ptA5 = true;
		}
		if (ptB.at(0) <= TH1&& ptB.at(1) <= TH1&& ptB.at(2) > TH1)
		{
			ptB5 = true;
		}

		if (!g_imgFlg.at<int>(centerPt.x, centerPt.y))//&& ptA5 && ptB5
		{

			if (green)
			{
				if (nType == 3)
				{
					g_edge[0].at<float>(centerPt.x, centerPt.y) = 0;
					g_edge[1].at<float>(centerPt.x, centerPt.y) = 255;
					g_edge[2].at<float>(centerPt.x, centerPt.y) = 0;
				}else if (nType==7)
				{
					g_edge[0].at<float>(centerPt.x, centerPt.y) = 0;
					g_edge[1].at<float>(centerPt.x, centerPt.y) = 0;
					g_edge[2].at<float>(centerPt.x, centerPt.y) = 255;
				}
				
				g_imgFlg.at<int>(centerPt.x, centerPt.y) = 5;
			}

			float comR;//比较大小
			if (rgb)
			{
				comR = (srcBGR[0].at<float>(centerPt.x, centerPt.y) - srcBGR[0].at<float>(AC.x, AC.y)) +
					(srcBGR[1].at<float>(centerPt.x, centerPt.y) - srcBGR[1].at<float>(AC.x, AC.y)) +
					(srcBGR[2].at<float>(centerPt.x, centerPt.y) - srcBGR[2].at<float>(AC.x, AC.y));
			}
			else
			{
				g_avgACBC = (g_calImg.at<float>(AC.x, AC.y) + g_calImg.at<float>(BC.x, BC.y)) / 2;
				comR = g_calImg.at<float>(centerPt.x, centerPt.y) - g_avgACBC;

			}
			if (comR > 0){
				min_max = true;
				if (virtual_edge)//大边
				{
					
					g_edge[0].at<float>(A.x, A.y) = 0;
					g_edge[1].at<float>(A.x, A.y) = 0;
					g_edge[2].at<float>(A.x, A.y) = 100;

					g_edge[0].at<float>(B.x, B.y) = 0;
					g_edge[1].at<float>(B.x, B.y) = 0;
					g_edge[2].at<float>(B.x, B.y) = 100;
					if (g_imgFlg.at<int>(centerPt.x, centerPt.y) == 0)
					{
						g_edge[0].at<float>(centerPt.x, centerPt.y) = 0;
						g_edge[1].at<float>(centerPt.x, centerPt.y) = 0;
						g_edge[2].at<float>(centerPt.x, centerPt.y) = 100;
					}


				}
				//tag start point's 8 neibor
				for (int k = 0; k < 8; k++)
				{
					if (g_imgFlg.at<int>(centerPt.x + a8i[k], centerPt.y + a8j[k]) == 0)
					{
						g_imgFlg.at<int>(centerPt.x + a8i[k], centerPt.y + a8j[k]) = 2;
					}
				}
				g_imgFlg.at<int>(A.x, A.y) = 2;
				g_imgFlg.at<int>(centerPt.x, centerPt.y) = 2;
				g_imgFlg.at<int>(B.x, B.y) = 2;

			}
			else{//与中心点不相同点且比中心点大的标记为黄色
				min_max = false;
				//*
				if (real_edge)//小边
				{
					bluePt++;
					g_edge[0].at<float>(A.x, A.y) = 0;
					g_edge[1].at<float>(A.x, A.y) = 100;
					g_edge[2].at<float>(A.x, A.y) = 100;
					if (g_imgFlg.at<int>(centerPt.x, centerPt.y) == 0)
					{
						g_edge[0].at<float>(centerPt.x, centerPt.y) = 0;
						g_edge[1].at<float>(centerPt.x, centerPt.y) = 255;
						g_edge[2].at<float>(centerPt.x, centerPt.y) = 255;
					}
					g_edge[0].at<float>(B.x, B.y) = 0;
					g_edge[1].at<float>(B.x, B.y) = 100;
					g_edge[2].at<float>(B.x, B.y) = 100;
				}

				g_imgFlg.at<int>(A.x, A.y) = 1;
				g_imgFlg.at<int>(centerPt.x, centerPt.y) = 1;
				g_imgFlg.at<int>(B.x, B.y) = 1;

			}

		}
		else
			return false;


		//g_thresold.at<float>(centerPt.x, centerPt.y) = getDth(A, centerPt, AC, B, BC, -direction);
		//TH_D = g_thresold.at<float>(centerPt.x, centerPt.y);
		//传递的方向和getth的方向相反。geth用的是延伸方向，而此时是起点方向。

		//if (g_thresold.at<float>(centerPt.x, centerPt.y) > 0)
		//{
		//	cout << "ll=" << g_thresold.at<float>(centerPt.x, centerPt.y) << endl;
		//	return true;//如果找到A、B两个边缘点则开始调用延伸追踪函数TraceEdgeTwoBoundary
		//}
		//else
		//	return false;

		return true;//如果找到A、B两个边缘点则开始调用延伸追踪函数TraceEdgeTwoBoundary
	}
	else
		return false;

	//2 judge whether A&&B is type 5

}


#if (START==2)
bool findStartPt2(Point2i centerPt, Point2i &A, Point2i &B, Point2i &AC, Point2i &BC, int &direction, int& min_max)
{
	bool  flagA, flagB;//标记是否找到A/B点
	flagA = flagB = false;
	int nType;
	Point2i stPt, E;
	int i, j;
	int bdA, bdB, bdTmp;
	int dx, dy;
	i = centerPt.x;
	j = centerPt.y;
	//与中心边缘点相邻的两个边缘点A，B，
	int indext;
	bool kflagMissed = false;
	Point2i missedPt;
	int dA, dB, dAC, dBC;
	float CV;
	int comR;
	int flagkk = g_imgFlg.at<int>(centerPt.x, centerPt.y);
	if (flagkk)
		return false;

	//下面函数用来找到中心点的相同的起止点A、B及不同的起止点AC、BC
	if (rgb)//彩色图像
	{
		//如果第一个点和中心点相同，则找连续两个和中点不相同的点
		if (absAB_BGR(centerPt, i, j + 1) <= TH2){
			for (indext = 1; indext<8; indext++)
			{
				if (absAB_BGR(centerPt, i + a8i[indext], j + a8j[indext])>TH2)
				{
					if (absAB_BGR(centerPt, i + a8i[indext + 1], j + a8j[indext + 1]) > TH2){
						A.x = i + a8i[indext - 1];
						A.y = j + a8j[indext - 1];
						AC.x = i + a8i[indext];
						AC.y = j + a8j[indext];
						flagA = true;
						break;
					}
					else{
						indext += 1;
					}
				}
			}
			for (indext = 7; indext > 0; indext--)
			{
				if (absAB_BGR(centerPt, i + a8i[indext], j + a8j[indext]) > TH2)
				{
					int ii = indext - 1;
					if (ii<0)  ii += 8;
					if (absAB_BGR(centerPt, i + a8i[ii], j + a8j[ii]) > TH2){
						B.x = i + a8i[indext + 1];
						B.y = j + a8j[indext + 1];
						BC.x = i + a8i[indext];
						BC.y = j + a8j[indext];
						if (AC == BC)
						{
							return false;
						}
						flagB = true;
						break;
					}
					else{
						indext -= 1;
					}
				}
			}
		}
		else{//如果第一个点和中心点不相同，则找连续两个和中点相同的点
			for (indext = 1; indext < 8; indext++)
			{
				if (absAB_BGR(centerPt, i + a8i[indext], j + a8j[indext]) <= TH2)
				{
					if (absAB_BGR(centerPt, i + a8i[indext + 1], j + a8j[indext + 1]) <= TH2)
					{
						A.x = i + a8i[indext];
						A.y = j + a8j[indext];
						AC.x = i + a8i[indext - 1];
						AC.y = j + a8j[indext - 1];
						flagA = true;

						break;
					}
					else
					{
						indext += 1;
					}

				}
			}
			for (indext = 7; indext > 0; indext--)
			{
				if (absAB_BGR(centerPt, i + a8i[indext], j + a8j[indext]) <= TH2)
				{
					int ii = indext - 1;
					if (ii < 0)  ii += 8;
					if (absAB_BGR(centerPt, i + a8i[ii], j + a8j[ii]) <= TH2)
					{
						B.x = i + a8i[indext];
						B.y = j + a8j[indext];
						BC.x = i + a8i[indext + 1];
						BC.y = j + a8j[indext + 1];
						if (AC == BC)
						{
							return false;
						}
						flagB = true;
						break;
					}
					else
					{
						indext -= 1;
					}

				}
			}
		}

	}

	//灰度图像
	else
	{
		//如果第一个点和中心点相同，则找连续两个和中点不相同的点
		CV = g_calImg.at<float>(centerPt.x, centerPt.y);
		if (abs(CV - g_calImg.at<float>(i, j + 1)) <= TH1){
			for (indext = 1; indext<8; indext++)
			{
				if (abs(CV - g_calImg.at<float>(i + a8i[indext], j + a8j[indext]))>TH1)
				{
					if (abs(CV - g_calImg.at<float>(i + a8i[indext + 1], j + a8j[indext + 1])) > TH1){
						A.x = i + a8i[indext - 1];
						A.y = j + a8j[indext - 1];
						AC.x = i + a8i[indext];
						AC.y = j + a8j[indext];
						flagA = true;
						break;
					}
					else{
						indext += 1;
					}
				}
			}
			for (indext = 7; indext > 0; indext--)
			{
				if (abs(CV - g_calImg.at<float>(i + a8i[indext], j + a8j[indext])) > TH1)
				{
					int ii = indext - 1;
					if (ii<0)  ii += 8;
					if (abs(CV - g_calImg.at<float>(i + a8i[ii], j + a8j[ii])) > TH1){
						B.x = i + a8i[indext + 1];
						B.y = j + a8j[indext + 1];
						BC.x = i + a8i[indext];
						BC.y = j + a8j[indext];
						if (AC == BC)
						{
							return false;
						}
						flagB = true;
						break;
					}
					else{
						indext -= 1;
					}
				}
			}
		}
		else{//如果第一个点和中心点不相同，则找连续两个和中点相同的点
			for (indext = 1; indext < 8; indext++)
			{
				if (abs(CV - g_calImg.at<float>(i + a8i[indext], j + a8j[indext])) <= TH1)
				{
					if (abs(CV - g_calImg.at<float>(i + a8i[indext + 1], j + a8j[indext + 1])) <= TH1)
					{
						A.x = i + a8i[indext];
						A.y = j + a8j[indext];
						AC.x = i + a8i[indext - 1];
						AC.y = j + a8j[indext - 1];
						flagA = true;

						break;
					}
					else
					{
						indext += 1;
					}

				}
			}
			for (indext = 7; indext > 0; indext--)
			{
				if (abs(CV - g_calImg.at<float>(i + a8i[indext], j + a8j[indext])) <= TH1)
				{
					int ii = indext - 1;
					if (ii < 0)  ii += 8;
					if (abs(CV - g_calImg.at<float>(i + a8i[ii], j + a8j[ii])) <= TH1)
					{
						B.x = i + a8i[indext];
						B.y = j + a8j[indext];
						BC.x = i + a8i[indext + 1];
						BC.y = j + a8j[indext + 1];
						if (AC == BC)
						{
							return false;
						}
						flagB = true;
						break;
					}
					else
					{
						indext -= 1;
					}

				}
			}
		}

	}

	//计算Direction,若如下叉积小于0，则从半平面的顺时针开始查找
	direction = (A.x - centerPt.x)*(AC.y - centerPt.y) - (A.y - centerPt.y)*(AC.x - centerPt.x);
	dx = A.x - centerPt.x;
	dy = A.y - centerPt.y;

	//得到了A、B后，计算出边缘类型
	//把A、B点转为相对下标来求得连续长度作为边缘类型
	if (outOfrange(A.x, A.y) || outOfrange(B.x, B.y) || outOfrange(AC.x, AC.y) || outOfrange(BC.x, BC.y))
	{
		return false;//防止越界
	}
	if (g_imgFlg.at<int>(A.x, A.y) || g_imgFlg.at<int>(B.x, B.y) || g_imgFlg.at<int>(AC.x, AC.y) || g_imgFlg.at<int>(BC.x, BC.y))
		return false;//已经处理过


	dA = getIangle(centerPt, A);
	dB = getIangle(centerPt, B);
	nType = lengthAB(dA, dB, direction);
	bool isStartPt = startPoint(centerPt.x, centerPt.y);
	g_avgACBC = (g_calImg.at<float>(AC.x, AC.y) + g_calImg.at<float>(BC.x, BC.y)) / 2;
	min_max = g_calImg.at<float>(centerPt.x, centerPt.y) - g_avgACBC;
	//comR = min_max;
	if (flagA&&flagB&&nType > 2 && nType < 7 && isStartPt)//nSection(centerPt) &&
	{
		//判断中心点是否为最大最小
		MyStruct mm;
		mm = searchMValue(centerPt, min_max);
		if (mm.m_num == -1)
		{
			return false;
		}
		//判断两个区域是否为纯净区
		//输入：centerPt,A,B AC,BC 
		//输出：true or false
		//A-B区域是否<TH，AC—BC是否>TH
		//*/没有扩展到彩色图像中
		int dA = getIangle(centerPt, A);
		int dB = getIangle(centerPt, B);
		int dAC = getIangle(centerPt, AC);
		int dBC = getIangle(centerPt, BC);
		int tmpV;
		int tmpC;
		tmpC = g_calImg.at<float>(centerPt.x, centerPt.y);
		//平滑噪音点
		if (direction > 0)
		{
			while ((dA) != dB)
			{
				dA--;
				if (dA == -1)
				{
					dA += 8;
				}
				tmpV = g_calImg.at<float>(centerPt.x + a8i[dA], centerPt.y + a8j[dA]);
				if (abs(tmpV - tmpC) > TH1)
				{
					//g_calImg.at<float>(centerPt.x + a8i[dA], centerPt.y + a8j[dA]) = tmpC;
					return false;
				}
				if (dA - 1 != dB)
				{
					//去除和D点相邻的点，避免标记了点，后续不能粥延伸：参见论文/每天任务/今天问题
					//g_imgFlg.at<float>(centerPt.x + a8i[dA], centerPt.y + a8j[dA]) = 22;
				}
			}
			while ((dAC) != dBC)
			{
				dAC++;
				if (dAC == 8)
				{
					dAC -= 8;
				}
				tmpV = g_calImg.at<float>(centerPt.x + a8i[dAC], centerPt.y + a8j[dAC]);
				if (abs(tmpV - tmpC) <= TH1)
				{
					//g_calImg.at<float>(centerPt.x + a8i[dAC], centerPt.y + a8j[dAC]) = tmpC;
					return false;
				}
			}

		}
		else
		{
			while ((dA) != dB)
			{
				dA++;
				if (dA == 8)
				{
					dA -= 8;
				}
				tmpV = g_calImg.at<float>(centerPt.x + a8i[dA], centerPt.y + a8j[dA]);
				if (abs(tmpV - tmpC) > TH1)
				{
					//g_calImg.at<float>(centerPt.x + a8i[dA], centerPt.y + a8j[dA]) = tmpC;
					return false;
				}
				if (dA + 1 != dB)
				{
					//去除和D点相邻的点，避免标记了点，后续不能粥延伸：参见论文/每天任务/今天问题
					//g_imgFlg.at<float>(centerPt.x + a8i[dA], centerPt.y + a8j[dA]) = 22;
				}
			}
			while ((dAC) != dBC)
			{
				dAC--;
				if (dAC == -1)
				{
					dAC += 8;
				}
				tmpV = g_calImg.at<float>(centerPt.x + a8i[dAC], centerPt.y + a8j[dAC]);
				if (abs(tmpV - tmpC) <= TH1)
				{
					//g_calImg.at<float>(centerPt.x + a8i[dAC], centerPt.y + a8j[dAC]) = tmpC;
					return false;
				}
			}
		}
		//*/


		if (!g_imgFlg.at<int>(centerPt.x, centerPt.y))
		{

			if (green)
			{
				g_edge[0].at<float>(centerPt.x, centerPt.y) = 0;
				g_edge[1].at<float>(centerPt.x, centerPt.y) = 255;
				g_edge[2].at<float>(centerPt.x, centerPt.y) = 0;
				g_imgFlg.at<int>(centerPt.x, centerPt.y) = 5;
			}


			//补上起点的AC-BC
			//kflagMissed = GetMissedPoint(centerPt, AC, BC, direction, missedPt);

			//dAC = getIangle(centerPt, AC);
			//dBC = getIangle(centerPt, BC);
			//与中心点不相同点比中心点小的标记为红色

			float comR;//比较大小
			if (rgb)
			{
				comR = (srcBGR[0].at<float>(centerPt.x, centerPt.y) - srcBGR[0].at<float>(AC.x, AC.y)) +
					(srcBGR[1].at<float>(centerPt.x, centerPt.y) - srcBGR[1].at<float>(AC.x, AC.y)) +
					(srcBGR[2].at<float>(centerPt.x, centerPt.y) - srcBGR[2].at<float>(AC.x, AC.y));
			}
			else
			{


				//g_calImg.at<float>(AC.x, AC.y) = g_avgACBC;
				//g_calImg.at<float>(BC.x, BC.y) = g_avgACBC;
			}
			if (min_max > 0){
				if (virtual_edge)//大边
				{
					g_edge[0].at<float>(A.x, A.y) = 0;
					g_edge[1].at<float>(A.x, A.y) = 0;
					g_edge[2].at<float>(A.x, A.y) = 100;

					g_edge[0].at<float>(B.x, B.y) = 0;
					g_edge[1].at<float>(B.x, B.y) = 0;
					g_edge[2].at<float>(B.x, B.y) = 100;
					if (g_imgFlg.at<int>(centerPt.x, centerPt.y) == 0)
					{
						g_edge[0].at<float>(centerPt.x, centerPt.y) = 0;
						g_edge[1].at<float>(centerPt.x, centerPt.y) = 0;
						g_edge[2].at<float>(centerPt.x, centerPt.y) = 100;
					}


				}

				g_imgFlg.at<int>(A.x, A.y) = 2;
				g_imgFlg.at<int>(centerPt.x, centerPt.y) = 2;
				g_imgFlg.at<int>(B.x, B.y) = 2;

			}
			else{//与中心点不相同点且比中心点大的标记为黄色

				//*
				if (real_edge)//小边
				{
					bluePt++;
					g_edge[0].at<float>(A.x, A.y) = 0;
					g_edge[1].at<float>(A.x, A.y) = 255;
					g_edge[2].at<float>(A.x, A.y) = 255;
					if (g_imgFlg.at<int>(centerPt.x, centerPt.y) == 0)
					{
						g_edge[0].at<float>(centerPt.x, centerPt.y) = 0;
						g_edge[1].at<float>(centerPt.x, centerPt.y) = 255;
						g_edge[2].at<float>(centerPt.x, centerPt.y) = 255;
					}
					g_edge[0].at<float>(B.x, B.y) = 0;
					g_edge[1].at<float>(B.x, B.y) = 255;
					g_edge[2].at<float>(B.x, B.y) = 255;
				}

				g_imgFlg.at<int>(A.x, A.y) = 1;
				g_imgFlg.at<int>(centerPt.x, centerPt.y) = 1;
				g_imgFlg.at<int>(B.x, B.y) = 1;

			}

		}//*/


		//g_thresold.at<float>(centerPt.x, centerPt.y) = getDth(A, centerPt, AC, B, BC, -direction);
		//TH_D = g_thresold.at<float>(centerPt.x, centerPt.y);
		//传递的方向和getth的方向相反。geth用的是延伸方向，而此时是起点方向。

		//if (g_thresold.at<float>(centerPt.x, centerPt.y) > 0)
		//{
		//	cout << "ll=" << g_thresold.at<float>(centerPt.x, centerPt.y) << endl;
		//	return true;//如果找到A、B两个边缘点则开始调用延伸追踪函数TraceEdgeTwoBoundary1
		//}
		//else
		//	return false;

		return true;//如果找到A、B两个边缘点则开始调用延伸追踪函数TraceEdgeTwoBoundary1
	}
	else
		return false;


}

#endif

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
#if (STRETCH==1)
//1延伸过程中找到半平面内的最小值及其位置并判断是否为反半平面内极小点
bool searchLocalExtremum1(Point2i centPt, Point2i * Pts, int min_max, int& index)
{

	int ptsVal[5], invptsVal[5];
	int d, tmpd;
	MyStruct mmi;

	int x, y;
	float pV, nV, CV;
	CV = g_calImg.at<float>(centPt.x, centPt.y);
	//1找到正半平面内的最值点pV
	for (int i = 0; i <5; i++)
	{
		ptsVal[4 - i] = g_calImg.at<float>(Pts[i].x, Pts[i].y);
	}//反向查找


	if (min_max>0)
	{
		mmi = find_Max(ptsVal, 5);
	}
	else
		mmi = find_Min(ptsVal, 5);

	index = 4 - mmi.m_index;

	try
	{
		pV = g_calImg.at<float>(Pts[index].x, Pts[index].y);//正半面的最值
	}
	catch (int)
	{
		cout << "下标越界\n" << endl;
	}


	d = getIangle(Pts[index], centPt);//中心点相对于最大或最小的方向，即中心点在最值点的位置

	//2判断最值点pV是否为反半平面内最值点
	//得到反半平面的值
	for (int i = 0; i < 5; i++)
	{
		tmpd = (d + a5[i] + 8) % 8;
		//排除已经标记过的点
		if (outOfrange(Pts[index].x + a8i[tmpd], Pts[index].y + a8j[tmpd]))
		{
			return false;
		}
		if (g_imgFlg.at<int>(Pts[index].x + a8i[tmpd], Pts[index].y + a8j[tmpd]))
		{
			if (min_max>0)
			{
				invptsVal[i] = -1;
			}
			else
				invptsVal[i] = 258;

		}
		else
		{
			invptsVal[i] = g_calImg.at<float>(Pts[index].x + a8i[tmpd], Pts[index].y + a8j[tmpd]);
		}

	}
	//判断正平面的最值是否为反半平面内最值点
	if (min_max > 0)
	{
		nV = findMax(invptsVal, 5);
		if (nV <= pV&&abs(pV - CV) <= TH1)//nV为反平面的最值
		{
			return true;
		}
		else
			return false;
	}
	else
	{

		nV = findMin(invptsVal, 5);
		if (nV >= pV&&abs(pV - CV) <= TH1)//反平面的最小值一定要大于等于正平面的最小值
		{
			return true;
		}
		else
			return false;
	}


}
#endif
#if (STRETCH==2)
//2延伸过程中只寻找前进半平面内的最值点
bool searchLocalExtremum2(Point2i centPt, Point2i * Pts, int min_max, int& index)
{

	int ptsVal[5], invptsVal[5];
	int d, tmpd;
	MyStruct mmi;

	int x, y;
	float pV, nV, CV;
	CV = g_calImg.at<float>(centPt.x, centPt.y);
	//1找到正半平面内的最值点pV
	for (int i = 0; i <5; i++)
	{
		ptsVal[4 - i] = g_calImg.at<float>(Pts[i].x, Pts[i].y);
	}//反向查找


	if (min_max>0)
	{
		mmi = find_Max(ptsVal, 5);
	}
	else
		mmi = find_Min(ptsVal, 5);

	index = 4 - mmi.m_index;

	try
	{
		pV = g_calImg.at<float>(Pts[index].x, Pts[index].y);//正半面的最值
	}
	catch (int)
	{
		cout << "下标越界\n" << endl;
	}

	return true;





}
#endif
#if (STRETCH==3)
//3延伸过程中寻找前进半平面内与中心点最为接近的点
bool searchLocalExtremum3(Point2i centPt, Point2i * Pts, int min_max, int& index)
{

	int ptsVal[5], invptsVal[5];
	int d, tmpd;
	MyStruct mmi;

	int x, y;
	float pV, nV, CV;
	CV = g_calImg.at<float>(centPt.x, centPt.y);

	for (int i = 0; i <5; i++)
	{
		ptsVal[4 - i] = abs(g_calImg.at<float>(Pts[i].x, Pts[i].y) - CV);
	}//反向查找


	//找和中心点最接近的点的位置及其值pV
	mmi = find_Min(ptsVal, 5);
	index = 4 - mmi.m_index;
	pV = g_calImg.at<float>(Pts[index].x, Pts[index].y);


	if (abs(pV - CV) <= TH1)//nV为反平面的最值
	{
		return true;
	}
	else
		return false;

}
#endif
//Trace two sides of the boundary
//direction; 0  denotes  counterclockwise
//输入：点A、B、C ,direction    A、B为相邻边缘点，B为中心点，C为与AB不同类的点,direction为方向
//输出：点D、E     D为B的半平面内最后一个和B点相近的点，E为半平面内第一个与B不相似的点
bool TraceEdgeTwoBoundary2(Point2i A, Point2i B, Point2i C, Point2i &D, Point2i &E, int  direction, int& min_max)
{
	int dx, dy, d1, d2;
	dx = B.x - A.x;
	dy = B.y - A.y;
	g_tracenum++;
	Point2i Pts[5];
	Point2i missedPt;
	bool flagD = false, flagE = false;
	bool collision = false;
	bool found = false;
	float comR;//比较大小
	int index;
	if (direction > 0){
		Pts[0].x = B.x + dy;
		Pts[0].y = B.y - dx;
		Pts[2].x = B.x + dx;
		Pts[2].y = B.y + dy;
		Pts[4].x = B.x - dy;
		Pts[4].y = B.y + dx;
		if (dx&dy){
			Pts[1].x = (Pts[0].x + Pts[2].x) / 2;
			Pts[1].y = (Pts[0].y + Pts[2].y) / 2;
			Pts[3].x = (Pts[4].x + Pts[2].x) / 2;
			Pts[3].y = (Pts[4].y + Pts[2].y) / 2;
		}
		else{
			Pts[1].x = (Pts[0].x + Pts[2].x - B.x);
			Pts[1].y = (Pts[0].y + Pts[2].y - B.y);
			Pts[3].x = (Pts[4].x + Pts[2].x - B.x);
			Pts[3].y = (Pts[4].y + Pts[2].y - B.y);
		}
	}
	else{
		Pts[4].x = B.x + dy;
		Pts[4].y = B.y - dx;
		Pts[2].x = B.x + dx;
		Pts[2].y = B.y + dy;
		Pts[0].x = B.x - dy;
		Pts[0].y = B.y + dx;
		if (dx&dy){
			Pts[3].x = (Pts[4].x + Pts[2].x) / 2;
			Pts[3].y = (Pts[4].y + Pts[2].y) / 2;
			Pts[1].x = (Pts[0].x + Pts[2].x) / 2;
			Pts[1].y = (Pts[0].y + Pts[2].y) / 2;
		}
		else{
			Pts[3].x = (Pts[4].x + Pts[2].x - B.x);
			Pts[3].y = (Pts[4].y + Pts[2].y - B.y);
			Pts[1].x = (Pts[0].x + Pts[2].x - B.x);
			Pts[1].y = (Pts[0].y + Pts[2].y - B.y);
		}
	}
	int i;



	if (rgb)
	{
		for (i = 0; i < 4; i++)
		{
			if (!outOfrange(Pts[i].x, Pts[i].y)){
				if (absAB_BGR(B, Pts[i].x, Pts[i].y) < absAB_BGR(C, Pts[i].x, Pts[i].y))//再添加阈值限制条件？&&改进为找到最后一个和B相同的点，前面的为噪音。
				{

					D.x = Pts[i].x;
					D.y = Pts[i].y;
					E.x = Pts[i + 1].x;
					E.y = Pts[i + 1].y;


					flagD = true;
					flagE = true;

				}
			}
		}

	}
	else//灰度图像
	{
		float  BV, CV, tempV, tempV1;
		BV = g_calImg.at<float>(B.x, B.y);
		CV = g_calImg.at<float>(C.x, C.y);
#if (STRETCH==1)
		if (searchLocalExtremum1(B, Pts, min_max, index))
#endif
#if (STRETCH==2)
		if (searchLocalExtremum2(B, Pts, min_max, index))
#endif
#if (STRETCH==3)
		if (searchLocalExtremum3(B, Pts, min_max, index))
#endif

		{

			D.x = Pts[index].x;
			D.y = Pts[index].y;
			d1 = D.x - A.x;
			d2 = D.y - A.y;
			if (abs(d1) < 2 && abs(d2) < 2)
			{
				return false;
			}

			if (index + 1 == 5)
			{
				E.x = C.x;
				E.y = C.y;
			}
			else
			{
				E.x = Pts[index + 1].x;
				E.y = Pts[index + 1].y;
			}
			flagD = true;
			flagE = true;

		}
		else
			return false;
	}


	//Pts[0-4] set the value of color
	//*
	int indext;
	bool kflagMissed = false;

	if (flagD&&flagE)
	{
		if (outOfrange(D.x, D.y) || g_imgFlg.at<int>(D.x, D.y))//|| outOfrange(E.x, E.y) || g_imgFlg.at<int>(E.x, E.y)
		{
			D.x = 1;
			D.y = 1;
			return false;

		}


		//g_thresold.at<float>(B.x, B.y) = getDth(A, B, C, D, E, direction);
		//TH_D = g_thresold.at<float>(B.x, B.y);

		int  directC, directE, deltat;
		directC = getIangle(B, C);
		directE = getIangle(B, E);

		int dA, dD, dC, dE;

		/*	float BVB = srcBGR[0].at<float>(B.x, B.y);
			float BVG = srcBGR[1].at<float>(B.x, B.y);
			float BVR = srcBGR[2].at<float>(B.x, B.y);*/


		float BV = g_calImg.at<float>(B.x, B.y);
		float CV = g_calImg.at<float>(C.x, C.y);
		float tmpV;

		dA = getIangle(B, A);
		dD = getIangle(B, D);
		dC = getIangle(B, C);
		dE = getIangle(B, E);
		//-------------------------------------------------------------------
		if (direction > 0)
		{
			//cout << "sss" << endl;
			while (dA != dD)
			{
				dA++;
				if (dA == 8)
				{
					dA -= 8;
				}
				if (rgb)
				{
					/*srcBGR[0].at<float>(B.x + a8i[dA], B.y + a8j[dA]) = BVB;
					srcBGR[1].at<float>(B.x + a8i[dA], B.y + a8j[dA]) = BVG;
					srcBGR[2].at<float>(B.x + a8i[dA], B.y + a8j[dA]) = BVR;*/
				}
				else
				{
					tmpV = g_calImg.at<float>(B.x + a8i[dA], B.y + a8j[dA]);
					if (abs(tmpV - BV) > TH1)
					{
						//标记噪音位置及颜色
						/*g_edge[0].at<float>(B.x + a8i[dA], B.y + a8j[dA]) = 255;
						g_edge[1].at<float>(B.x + a8i[dA], B.y + a8j[dA]) = 255;
						g_edge[2].at<float>(B.x + a8i[dA], B.y + a8j[dA]) = 0;*/
					}
					//g_calImg.at<float>(B.x + a8i[dA], B.y + a8j[dA]) = BV;

				}
				if (dA + 1 != dD)
				{
					//去除和D点相邻的点，避免标记了点，后续不能粥延伸：参见论文/每天任务/今天问题
					//g_imgFlg.at<float>(B.x + a8i[dA], B.y + a8j[dA]) = 22;
				}

			}
			//while (dC != dE)
			//{
			//	dC--;
			//	if (dC == -1)
			//	{
			//		dC += 8;
			//	}
			//	if (rgb)
			//	{
			//		/*srcBGR[0].at<float>(B.x + a8i[dC], B.y + a8j[dC]) = BVB;
			//		srcBGR[1].at<float>(B.x + a8i[dC], B.y + a8j[dC]) = BVG;
			//		srcBGR[2].at<float>(B.x + a8i[dC], B.y + a8j[dC]) = BVR;*/
			//	}
			//	else
			//	{
			//		//g_calImg.at<float>(B.x + a8i[dC], B.y + a8j[dC]) = CV;
			//	}
			//	if (dC-1!=dE)
			//	{
			//		//这里需要添加
			//		
			//	}
			//	g_imgFlg.at<float>(B.x + a8i[dC], B.y + a8j[dC]) = 22;
			//}
		}
		else
		{
			while (dA != dD)
			{
				dA--;
				if (dA == -1)
				{
					dA += 8;
				}
				if (rgb)
				{
					/*srcBGR[0].at<float>(B.x + a8i[dA], B.y + a8j[dA]) = BVB;
					srcBGR[1].at<float>(B.x + a8i[dA], B.y + a8j[dA]) = BVG;
					srcBGR[2].at<float>(B.x + a8i[dA], B.y + a8j[dA]) = BVR;*/
				}
				else
				{
					tmpV = g_calImg.at<float>(B.x + a8i[dA], B.y + a8j[dA]);
					if (abs(tmpV - BV) > TH1)
					{
						//标记噪音位置及颜色
						/*g_edge[0].at<float>(B.x + a8i[dA], B.y + a8j[dA]) = 255;
						g_edge[1].at<float>(B.x + a8i[dA], B.y + a8j[dA]) = 255;
						g_edge[2].at<float>(B.x + a8i[dA], B.y + a8j[dA]) = 0;*/
					}
					//g_calImg.at<float>(B.x + a8i[dA], B.y + a8j[dA]) = BV;

				}
				if (dA - 1 != dD)
				{
					//去除和D点相邻的点，避免标记了点，后续不能粥延伸：参见论文/每天任务/今天问题
					//g_imgFlg.at<float>(B.x + a8i[dA], B.y + a8j[dA]) = 22;
				}

			}
			//while (dC != dE)
			//{
			//	dC++;
			//	if (dC == 8)
			//	{
			//		dC -= 8;
			//	}
			//	if (rgb)
			//	{
			//		/*srcBGR[0].at<float>(B.x + a8i[dC], B.y + a8j[dC]) = BVB;
			//		srcBGR[1].at<float>(B.x + a8i[dC], B.y + a8j[dC]) = BVG;
			//		srcBGR[2].at<float>(B.x + a8i[dC], B.y + a8j[dC]) = BVR;*/
			//	}
			//	else
			//	{
			//		g_calImg.at<float>(B.x + a8i[dC], B.y + a8j[dC]) = CV;
			//	}

			//	g_imgFlg.at<float>(B.x + a8i[dC], B.y + a8j[dC]) = 22;
			//}
		}

		//-----------------------------------------------------------------------------

		if (min_max > 0)
		{
			if (virtual_edge)//大边
			{
				g_edge[0].at<float>(D.x, D.y) = 0;
				g_edge[1].at<float>(D.x, D.y) = 0;
				g_edge[2].at<float>(D.x, D.y) = 255;
				g_imgFlg.at<int>(D.x, D.y) = 2;
			}


			if (real_edge)//小边
			{
				bluePt++;

				if (g_imgFlg.at<int>(E.x, E.y) == g_imgFlg.at<int>(D.x, D.y))
				{
					g_edge[0].at<float>(D.x, D.y) = 0;
					g_edge[1].at<float>(D.x, D.y) = 0;
					g_edge[2].at<float>(D.x, D.y) = 255;
					collision = true;
					//return false;//去除双重同边
				}
				if (!g_imgFlg.at<int>(E.x, E.y))
				{

					g_imgFlg.at<int>(E.x, E.y) = 1;
				}

			}

		}
		else{
			if (real_edge)//小边
			{
				bluePt++;
				g_edge[0].at<float>(D.x, D.y) = 0;
				g_edge[1].at<float>(D.x, D.y) = 255;
				g_edge[2].at<float>(D.x, D.y) = 255;
				g_imgFlg.at<int>(D.x, D.y) = 1;
			}

			if (virtual_edge)//大边
			{
				//E已经被标记
				if (g_imgFlg.at<int>(E.x, E.y) == g_imgFlg.at<int>(D.x, D.y))
				{
					g_edge[0].at<float>(D.x, D.y) = 0;
					g_edge[1].at<float>(D.x, D.y) = 255;
					g_edge[2].at<float>(D.x, D.y) = 255;
					collision = true;
					//return false;
				}
				//E未被标记
				if (!g_imgFlg.at<int>(E.x, E.y))
				{
					g_imgFlg.at<int>(E.x, E.y) = 2;
				}

			}
		}

		//检测是否碰触到另一边
		if (collision)
		{
			return false;
		}
		else
			return true;


	}

}
bool TraceEdgeTwoBoundary1(Point2i A, Point2i B, Point2i C, Point2i &D, Point2i &E, int  direction)
{
	int dx, dy;
	dx = B.x - A.x;
	dy = B.y - A.y;
	g_tracenum++;
	Point2i Pts[5];
	Point2i missedPt;
	bool flagD = false, flagE = false;
	if (direction > 0){
		Pts[0].x = B.x + dy;
		Pts[0].y = B.y - dx;
		Pts[2].x = B.x + dx;
		Pts[2].y = B.y + dy;
		Pts[4].x = B.x - dy;
		Pts[4].y = B.y + dx;
		if (dx&&dy){
			Pts[1].x = (Pts[0].x + Pts[2].x) / 2;
			Pts[1].y = (Pts[0].y + Pts[2].y) / 2;
			Pts[3].x = (Pts[4].x + Pts[2].x) / 2;
			Pts[3].y = (Pts[4].y + Pts[2].y) / 2;
		}
		else{
			Pts[1].x = (Pts[0].x + Pts[2].x - B.x);
			Pts[1].y = (Pts[0].y + Pts[2].y - B.y);
			Pts[3].x = (Pts[4].x + Pts[2].x - B.x);
			Pts[3].y = (Pts[4].y + Pts[2].y - B.y);
		}
	}
	else{
		Pts[4].x = B.x + dy;
		Pts[4].y = B.y - dx;
		Pts[2].x = B.x + dx;
		Pts[2].y = B.y + dy;
		Pts[0].x = B.x - dy;
		Pts[0].y = B.y + dx;
		if (dx&&dy){
			Pts[3].x = (Pts[4].x + Pts[2].x) / 2;
			Pts[3].y = (Pts[4].y + Pts[2].y) / 2;
			Pts[1].x = (Pts[0].x + Pts[2].x) / 2;
			Pts[1].y = (Pts[0].y + Pts[2].y) / 2;
		}
		else{
			Pts[3].x = (Pts[4].x + Pts[2].x - B.x);
			Pts[3].y = (Pts[4].y + Pts[2].y - B.y);
			Pts[1].x = (Pts[0].x + Pts[2].x - B.x);
			Pts[1].y = (Pts[0].y + Pts[2].y - B.y);
		}
	}
	int i;

	if (rgb)
	{
		for (i = 0; i < 4; i++)
		{
			if (!outOfrange(Pts[i].x, Pts[i].y)){
				if (absAB_BGR(B, Pts[i].x, Pts[i].y) < absAB_BGR(C, Pts[i].x, Pts[i].y))//再添加阈值限制条件？&&改进为找到最后一个和B相同的点，前面的为噪音。
				{

					D.x = Pts[i].x;
					D.y = Pts[i].y;
					E.x = Pts[i + 1].x;
					E.y = Pts[i + 1].y;


					flagD = true;
					flagE = true;

				}
			}
		}

	}
	else//灰度图像
	{
		float  BV, CV, tempV, tempV1;
		BV = g_calImg.at<float>(B.x, B.y);
		CV = g_calImg.at<float>(C.x, C.y);

		for (i = 4; i >= 0; i--)//i==4考虑为延伸到了另一个区域
		{
			/*if (g_imgFlg.at<int>(Pts[i].x, Pts[i].y) > 0)
			{
			return false;
			}*/
			if (!outOfrange(Pts[i].x, Pts[i].y))//i + 1可能越数组下标&& && !g_imgFlg.at<int>(Pts[i].x, Pts[i].y)
			{
				tempV = g_calImg.at<float>(Pts[i].x, Pts[i].y);
				//tempV1 = g_calImg.at<float>(Pts[i + 1].x, Pts[i + 1].y);
				//find the last point[LP] which similar to centerPoint[B]
				if (abs(tempV - BV) <= abs(tempV - CV))//&&abs(tempV - BV)<TH1 &&abs(tempV-BV)<TH1可以去除一些非边缘点 && abs(tempV - BV) <TH_D&& abs(tempV - CV) >TH_D && abs(tempV - BV) <TH1&& abs(tempV - CV) >TH1 && abs(tempV1 - BV) > abs(tempV1 - CV)再添加阈值限制条件？&&改进为找到最后一个和B相同的点，前面的为噪音。 && abs(tempV1 - BV) > abs(tempV1 - CV)
				{
					if (i == 4)
					{
						if (abs(dx) == abs(dy))
						{
							D.x = Pts[i].x;
							D.y = Pts[i].y;
							E.x = C.x;
							E.y = C.y;

							flagD = true;
							flagE = true;
							break;

						}
						else
							return false;//水平方向，全部相同

					}
					else if (i == 0)
					{
						return false;//折回点的处理
					}
					else{
						D.x = Pts[i].x;
						D.y = Pts[i].y;
						E.x = Pts[i + 1].x;
						E.y = Pts[i + 1].y;

					}

					/*if (E.x==C.x&&E.y==C.y)
					{
					return false;
					}*/


					flagD = true;
					flagE = true;
					break;


				}
				//Ptrs[0]-LP之间用中心点的值去填充。				

			}
			else
				return false;

		}



	}


	//Pts[0-4] set the value of color
	//*
	int indext;
	bool kflagMissed = false;

	if (flagD&&flagE)
	{
		//增加判断D的前进半平面内是否有边，有则退出。



		if (outOfrange(D.x, D.y) || g_imgFlg.at<int>(D.x, D.y))//|| outOfrange(E.x, E.y) || g_imgFlg.at<int>(E.x, E.y)
		{
			D.x = 1;
			D.y = 1;
			return false;

		}


		//g_thresold.at<float>(B.x, B.y) = getDth(A, B, C, D, E, direction);
		//TH_D = g_thresold.at<float>(B.x, B.y);

		int  directC, directE, deltat;
		directC = getIangle(B, C);
		directE = getIangle(B, E);

		int dA, dD, dC, dE;

		/*	float BVB = srcBGR[0].at<float>(B.x, B.y);
		float BVG = srcBGR[1].at<float>(B.x, B.y);
		float BVR = srcBGR[2].at<float>(B.x, B.y);*/


		float BV = g_calImg.at<float>(B.x, B.y);
		float CV = g_calImg.at<float>(C.x, C.y);
		float tmpV;

		dA = getIangle(B, A);
		dD = getIangle(B, D);
		dC = getIangle(B, C);
		dE = getIangle(B, E);
		//-------------------------------------------------------------------
		if (direction > 0)
		{
			//cout << "sss" << endl;
			while (dA != dD)
			{
				dA++;
				if (dA == 8)
				{
					dA -= 8;
				}
				if (rgb)
				{
					/*srcBGR[0].at<float>(B.x + a8i[dA], B.y + a8j[dA]) = BVB;
					srcBGR[1].at<float>(B.x + a8i[dA], B.y + a8j[dA]) = BVG;
					srcBGR[2].at<float>(B.x + a8i[dA], B.y + a8j[dA]) = BVR;*/
				}
				else
				{
					tmpV = g_calImg.at<float>(B.x + a8i[dA], B.y + a8j[dA]);
					if (abs(tmpV - BV) > TH1)
					{
						//标记噪音位置及颜色
						/*g_edge[0].at<float>(B.x + a8i[dA], B.y + a8j[dA]) = 255;
						g_edge[1].at<float>(B.x + a8i[dA], B.y + a8j[dA]) = 255;
						g_edge[2].at<float>(B.x + a8i[dA], B.y + a8j[dA]) = 0;*/
					}
					//g_calImg.at<float>(B.x + a8i[dA], B.y + a8j[dA]) = BV;

				}
				if (dA + 1 != dD)
				{
					//去除和D点相邻的点，避免标记了点，后续不能粥延伸：参见论文/每天任务/今天问题
					//g_imgFlg.at<float>(B.x + a8i[dA], B.y + a8j[dA]) = 22;
				}

			}
			//while (dC != dE)
			//{
			//	dC--;
			//	if (dC == -1)
			//	{
			//		dC += 8;
			//	}
			//	if (rgb)
			//	{
			//		/*srcBGR[0].at<float>(B.x + a8i[dC], B.y + a8j[dC]) = BVB;
			//		srcBGR[1].at<float>(B.x + a8i[dC], B.y + a8j[dC]) = BVG;
			//		srcBGR[2].at<float>(B.x + a8i[dC], B.y + a8j[dC]) = BVR;*/
			//	}
			//	else
			//	{
			//		//g_calImg.at<float>(B.x + a8i[dC], B.y + a8j[dC]) = CV;
			//	}
			//	if (dC-1!=dE)
			//	{
			//		//这里需要添加
			//		
			//	}
			//	g_imgFlg.at<float>(B.x + a8i[dC], B.y + a8j[dC]) = 22;
			//}
		}
		else
		{
			while (dA != dD)
			{
				dA--;
				if (dA == -1)
				{
					dA += 8;
				}
				if (rgb)
				{
					/*srcBGR[0].at<float>(B.x + a8i[dA], B.y + a8j[dA]) = BVB;
					srcBGR[1].at<float>(B.x + a8i[dA], B.y + a8j[dA]) = BVG;
					srcBGR[2].at<float>(B.x + a8i[dA], B.y + a8j[dA]) = BVR;*/
				}
				else
				{
					tmpV = g_calImg.at<float>(B.x + a8i[dA], B.y + a8j[dA]);
					if (abs(tmpV - BV) > TH1)
					{
						//标记噪音位置及颜色
						/*g_edge[0].at<float>(B.x + a8i[dA], B.y + a8j[dA]) = 255;
						g_edge[1].at<float>(B.x + a8i[dA], B.y + a8j[dA]) = 255;
						g_edge[2].at<float>(B.x + a8i[dA], B.y + a8j[dA]) = 0;*/
					}
					//g_calImg.at<float>(B.x + a8i[dA], B.y + a8j[dA]) = BV;

				}
				if (dA - 1 != dD)
				{
					//去除和D点相邻的点，避免标记了点，后续不能粥延伸：参见论文/每天任务/今天问题
					//g_imgFlg.at<float>(B.x + a8i[dA], B.y + a8j[dA]) = 22;
				}

			}
			//while (dC != dE)
			//{
			//	dC++;
			//	if (dC == 8)
			//	{
			//		dC -= 8;
			//	}
			//	if (rgb)
			//	{
			//		/*srcBGR[0].at<float>(B.x + a8i[dC], B.y + a8j[dC]) = BVB;
			//		srcBGR[1].at<float>(B.x + a8i[dC], B.y + a8j[dC]) = BVG;
			//		srcBGR[2].at<float>(B.x + a8i[dC], B.y + a8j[dC]) = BVR;*/
			//	}
			//	else
			//	{
			//		g_calImg.at<float>(B.x + a8i[dC], B.y + a8j[dC]) = CV;
			//	}

			//	g_imgFlg.at<float>(B.x + a8i[dC], B.y + a8j[dC]) = 22;
			//}
		}

		//-----------------------------------------------------------------------------
		//kflagMissed = GetMissedPoint(B, C, E, -direction, missedPt);
		bool collision = false;
		float comR;//比较大小
		if (rgb)
		{
			comR = (srcBGR[0].at<float>(A.x, A.y) - srcBGR[0].at<float>(C.x, C.y)) +
				(srcBGR[1].at<float>(A.x, A.y) - srcBGR[1].at<float>(C.x, C.y)) +
				(srcBGR[2].at<float>(A.x, A.y) - srcBGR[2].at<float>(C.x, C.y));
		}
		else
		{
			comR = g_calImg.at<float>(A.x, A.y) - g_calImg.at<float>(C.x, C.y);
		}

		if (comR > 0)
		{
			if (virtual_edge)//大边
			{
				g_edge[0].at<float>(D.x, D.y) = 0;
				g_edge[1].at<float>(D.x, D.y) = 0;
				g_edge[2].at<float>(D.x, D.y) = 255;
				g_imgFlg.at<int>(D.x, D.y) = 2;
			}


			if (real_edge)//小边
			{
				bluePt++;

				if (g_imgFlg.at<int>(E.x, E.y) == g_imgFlg.at<int>(D.x, D.y))
				{
					g_edge[0].at<float>(D.x, D.y) = 0;
					g_edge[1].at<float>(D.x, D.y) = 0;
					g_edge[2].at<float>(D.x, D.y) = 255;
					collision = true;
					//return false;//去除双重同边
				}
				if (!g_imgFlg.at<int>(E.x, E.y))
				{

					g_imgFlg.at<int>(E.x, E.y) = 1;
				}

			}

		}
		else{
			if (real_edge)//小边
			{
				bluePt++;
				g_edge[0].at<float>(D.x, D.y) = 0;
				g_edge[1].at<float>(D.x, D.y) = 255;
				g_edge[2].at<float>(D.x, D.y) = 255;
				g_imgFlg.at<int>(D.x, D.y) = 1;
			}

			if (virtual_edge)//大边
			{
				//E已经被标记
				if (g_imgFlg.at<int>(E.x, E.y) == g_imgFlg.at<int>(D.x, D.y))
				{
					g_edge[0].at<float>(D.x, D.y) = 0;
					g_edge[1].at<float>(D.x, D.y) = 255;
					g_edge[2].at<float>(D.x, D.y) = 255;
					collision = true;
					//return false;
				}
				//E未被标记
				if (!g_imgFlg.at<int>(E.x, E.y))
				{
					g_imgFlg.at<int>(E.x, E.y) = 2;
				}

			}
		}

		//检测是否碰触到另一边
		if (collision)
		{
			return false;
		}
		else
			return true;


	}

}
//input:A,B...
//outPut:6 points +,6 points -
bool getSixPoints(Point2i A, Point2i B, float* pt_same, float* pt_diff, vector<Point2i>& pt, int direction){
	//find the 5th point from the same side
	int dBA = getIangle(B, A);
	Point2i  Pt3, Pt4, Pt5, Pt6, Pt7;
	if (direction > 0){
		for (int i = 0; i <= 6; i++)
		{
			if (!outOfrange(B.x + a8i[(dBA + i) % 8], B.y + a8j[(dBA + i) % 8]) || !outOfrange(B.x + a8i[(dBA - i + 8) % 8], B.y + a8j[(dBA - i + 8) % 8]))
			{
				pt_same[i] = g_calImg.at<float>(B.x + a8i[(dBA + i) % 8], B.y + a8j[(dBA + i) % 8]);
				pt_diff[i] = g_calImg.at<float>(B.x + a8i[(dBA - i + 7) % 8], B.y + a8j[(dBA - i + 7) % 8]);//(dBA - 5+8)%8				
			}
			else
				return false;

		}
		Pt3.x = B.x + a8i[(dBA + 2) % 8];
		Pt3.y = B.y + a8j[(dBA + 2) % 8];
		Pt4.x = B.x + a8i[(dBA + 3) % 8];
		Pt4.y = B.y + a8j[(dBA + 3) % 8];
		Pt5.x = B.x + a8i[(dBA + 4) % 8];
		Pt5.y = B.y + a8j[(dBA + 4) % 8];
		Pt6.x = B.x + a8i[(dBA + 5) % 8];
		Pt6.y = B.y + a8j[(dBA + 5) % 8];
		Pt7.x = B.x + a8i[(dBA + 6) % 8];
		Pt7.y = B.y + a8j[(dBA + 6) % 8];
		pt.push_back(Pt3); pt.push_back(Pt4); pt.push_back(Pt5); pt.push_back(Pt6); pt.push_back(Pt7);
	}
	else{
		for (int i = 0; i <= 6; i++)
		{
			if (!outOfrange(B.x + a8i[(dBA + i) % 8], B.y + a8j[(dBA + i) % 8]) || !outOfrange(B.x + a8i[(dBA - i + 8) % 8], B.y + a8j[(dBA - i + 8) % 8]))
			{
				pt_same[i] = g_calImg.at<float>(B.x + a8i[(dBA - i + 8) % 8], B.y + a8j[(dBA - i + 8) % 8]);
				pt_diff[i] = g_calImg.at<float>(B.x + a8i[(dBA + i + 1) % 8], B.y + a8j[(dBA + i + 1) % 8]);//(dBA - 5+8)%8
			}
			else
				return false;
		}
		Pt3.x = B.x + a8i[(dBA - 2 + 8) % 8];
		Pt3.y = B.y + a8j[(dBA - 2 + 8) % 8];
		Pt4.x = B.x + a8i[(dBA - 3 + 8) % 8];
		Pt4.y = B.y + a8j[(dBA - 3 + 8) % 8];
		Pt5.x = B.x + a8i[(dBA - 4 + 8) % 8];
		Pt5.y = B.y + a8j[(dBA - 4 + 8) % 8];
		Pt6.x = B.x + a8i[(dBA - 5 + 8) % 8];
		Pt6.y = B.y + a8j[(dBA - 5 + 8) % 8];
		Pt7.x = B.x + a8i[(dBA - 6 + 8) % 8];
		Pt7.y = B.y + a8j[(dBA - 6 + 8) % 8];
		pt.push_back(Pt3); pt.push_back(Pt4); pt.push_back(Pt5); pt.push_back(Pt6); pt.push_back(Pt7);
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
bool TraceEdgeTwoBoundary3(Point2i A, Point2i B, Point2i &D, int  direction, bool min_max){

	int flagD = false;
	float BV = g_calImg.at<float>(B.x, B.y);
	//the 4,5,6th's relative location
	float pt_same[9] = { 0, 0, 0, 0, 0, 0 };
	float pt_diff[9] = { 0, 0, 0, 0, 0, 0 };
	float pt_same_next4[9], pt_same_next5[9], pt_same_next6[9];
	float pt_diff_next4[9] = { 0, 0, 0, 0, 0, 0 };
	float pt_diff_next5[9] = { 0, 0, 0, 0, 0, 0 };
	float pt_diff_next6[9] = { 0, 0, 0, 0, 0, 0 };
	vector<Point2i> pt, pt_next4, pt_next5, pt_next6;
	int dBA = getIangle(B, A);
	int type = 0;
	//find the 5th point from the same side
	if (!getSixPoints(A, B, pt_same, pt_diff, pt, direction))
	{
		return false;
	}

	//get the difference:same side set as maxValue and different side set as minValue
	float maxValue[4], minValue[4];
	float same4[4] = { pt_same[0], pt_same[1], pt_same[2], BV };
	float diff4[4] = { pt_diff[0], pt_diff[1], pt_diff[2], pt_diff[3] };
	float same5[5] = { pt_same[0], pt_same[1], pt_same[2], pt_same[3], BV };
	float diff5[3] = { pt_diff[0], pt_diff[1], pt_diff[2] };
	float same6[6] = { pt_same[0], pt_same[1], pt_same[2], pt_same[3], pt_same[4], BV };
	float diff6[2] = { pt_diff[0], pt_diff[1] };


	//4th
	float same4_abs[4] = { abs(pt_same[3] - pt_same[0]), abs(pt_same[3] - pt_same[1]), abs(pt_same[3] - pt_same[2]), abs(pt_same[3] - BV) };
	float diff4_abs[4] = { abs(pt_same[3] - pt_diff[0]), abs(pt_same[3] - pt_diff[1]), abs(pt_same[3] - pt_diff[2]), abs(pt_same[3] - pt_diff[3]) };
	//maxValue[0] = findMin(same4_abs, 4);
	//minValue[0] = findMin(diff4_abs, 4);
	maxValue[0] = min(min(abs(pt_same[3] - pt_same[1]), abs(pt_same[3] - pt_same[2])), abs(pt_same[3] - BV));
	minValue[0] = min(abs(pt_same[3] - pt_diff[0]), abs(pt_same[3] - pt_diff[1]));
	//5th
	float same5_abs[5] = { abs(pt_same[4] - pt_same[0]), abs(pt_same[4] - pt_same[1]), abs(pt_same[4] - pt_same[2]), abs(pt_same[4] - pt_same[3]), abs(pt_same[4] - BV) };
	float diff5_abs[3] = { abs(pt_same[4] - pt_diff[0]), abs(pt_same[4] - pt_diff[1]), abs(pt_same[4] - pt_diff[2]) };
	//maxValue[1] = findMin(same5_abs, 5);
	//minValue[1] = findMin(diff5_abs, 3);
	maxValue[1] = min(min(abs(pt_same[4] - pt_same[1]), abs(pt_same[4] - pt_same[2])), abs(pt_same[4] - BV));
	minValue[1] = min(abs(pt_same[4] - pt_diff[0]), abs(pt_same[4] - pt_diff[1]));
	//6th
	float same6_abs[6] = { abs(pt_same[5] - pt_same[0]), abs(pt_same[5] - pt_same[1]), abs(pt_same[5] - pt_same[2]), abs(pt_same[5] - pt_same[3]), abs(pt_same[5] - pt_same[4]), abs(pt_same[5] - BV) };
	float diff6_abs[2] = { abs(pt_same[5] - pt_diff[0]), abs(pt_same[5] - pt_diff[1]) };
	//maxValue[2] = findMin(same6_abs, 6);
	//minValue[2] = findMin(diff6_abs, 2);
	maxValue[2] = min(min(abs(pt_same[5] - pt_same[1]), abs(pt_same[5] - pt_same[2])), abs(pt_same[5] - BV));
	minValue[2] = min(abs(pt_same[5] - pt_diff[0]), abs(pt_same[5] - pt_diff[1]));

	//7th
	maxValue[3] = min(min(abs(pt_same[6] - pt_same[1]), abs(pt_same[6] - pt_same[2])), abs(pt_same[6] - BV));
	minValue[3] = min(abs(pt_same[6] - pt_diff[0]), abs(pt_same[6] - pt_diff[1]));

	//if (!getSixPoints(B, pt.at(0), pt_same_next4, pt_diff_next4, pt_next4, direction))
	//{
	//	return false;
	//}
	//if (!getSixPoints(B, pt.at(1), pt_same_next5, pt_diff_next5, pt_next5, direction))
	//{
	//	return false;
	//}
	//if (!getSixPoints(B, pt.at(2), pt_same_next6, pt_diff_next6, pt_next6, direction))
	//{
	//	return false;
	//}
	float tmpCV3 = g_calImg.at<float>(pt.at(0).x, pt.at(0).y);
	float tmpCV4 = g_calImg.at<float>(pt.at(1).x, pt.at(1).y);
	float tmpCV5 = g_calImg.at<float>(pt.at(2).x, pt.at(2).y);
	float tmpCV6 = g_calImg.at<float>(pt.at(3).x, pt.at(3).y);
	float tmpCV7 = g_calImg.at<float>(pt.at(4).x, pt.at(4).y);
	/*float result4 = getSmaller(tmpCV4, pt_diff_next4, min_max);
	float result5 = getSmaller(tmpCV5, pt_diff_next5, min_max);
	float result6 = getSmaller(tmpCV6, pt_diff_next6, min_max);*/

	float sameA = g_calImg.at<float>(A.x, A.y);
	float diffAC = pt_diff[1];
	int typeD = 0;
	Point2i cornerPt;
	//Whether pt5 satisfies the condition[pt5?pt6:pt4],//get smaller than pt5, pt_diff_next
	//result>big value in same area
	if (maxValue[1] <= minValue[1]) {//Take pt5 as center point,from different side find point:pt22-pt55,judge whether pt5 statisfies the bellow condition
		//if (result5 >= maxValue[1])
		{
			if (maxValue[2] <= minValue[2])//&& minValue[1]>TH1
			{
				//whether pt6 is type 6
				if (!getSixPoints(B, pt.at(3), pt_same_next6, pt_diff_next6, pt_next6, direction))
				{
					return false;
				}
				for (int i = 0; i < 5; i++)
				{
					if (abs(pt_diff_next6[i + 1] - sameA) > abs(pt_diff_next6[i + 1] - diffAC))
					{
						typeD++;
					}
					else
					{
						break;
					}
				}

				//if (typeD == 5)//tag pt6 and choose pt5 as next D
				//{
				//	/*cornerPt = pt.at(3);
				//	if (g_imgFlg.at<int>(cornerPt.x, cornerPt.y) == 0)
				//	{
				//	g_edge[0].at<float>(cornerPt.x, cornerPt.y) = 255;
				//	g_edge[1].at<float>(cornerPt.x, cornerPt.y) = 0;
				//	g_edge[2].at<float>(cornerPt.x, cornerPt.y) = 255;
				//	g_imgFlg.at<int>(cornerPt.x, cornerPt.y) = 2;
				//	}*/
				//	//D = pt.at(2);
				//	D = pt.at(3);
				//	flagD = true;
				//}
				//else//pt6
				//{

				//	D = pt.at(3);
				//	flagD = true;

				if (maxValue[3] <= minValue[3])
				{
					D = pt.at(4);
					flagD = true;
					//type = 7;
				}
				else
				{
					D = pt.at(3);
					flagD = true;
					type = 6;
				}
			
				
			}
			else//pt5
			{
				D = pt.at(2);
				flagD = true;
				type = 5;
			}
		}
	}
	else if (maxValue[0] <= minValue[0]) {//pt4     && result4 >= maxValue[0] slow the condition  && minValue[0]>TH1

		D = pt.at(1);
		flagD = true;
		type = 4;

	}
	else if (3)//pt3   abs(tmpCV3-BV)<TH1
	{
		D = pt.at(0);
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


	float same4_min, same4_max, diff4_min, diff4_max;
	float same5_min, same5_max, diff5_min, diff5_max;
	float same6_min, same6_max, diff6_min, diff6_max;

	//other constrain
	if (MMC)
	{
		if (min_max)
		{
			switch (type)
			{
			case 0:
			case 3:
				break;
			case 4:
				same4_min = findMin(same4, 4);
				diff4_max = findMax(diff4, 4);
				if (same4_min <= diff4_max)
				{
					return false;
				}
				break;
			case 5:
				same5_min = findMin(same5, 5);
				diff5_max = findMax(diff5, 3);
				if (same5_min <= diff5_max)
				{
					return false;
				}
				break;
			case 6:
				same6_min = findMin(same6, 6);
				diff6_max = findMax(diff6, 2);
				if (same6_min <= diff6_max)
				{
					return false;
				}
				break;
			default:
				cout << "this point's neighbour have nocie\n";
			}
		}
		else
		{
			switch (type)
			{
			case 0:
			case 3:
				break;
			case 4:
				same4_max = findMax(same4, 4);
				diff4_min = findMin(diff4, 4);
				if (same4_max >= diff4_min)
				{
					return false;
				}
				break;
			case 5:
				same5_max = findMax(same5, 5);
				diff5_min = findMin(diff5, 3);
				if (same5_max >= diff5_min)
				{
					return false;
				}
				break;
			case 6:
				same6_max = findMax(same6, 6);
				diff6_min = findMin(diff6, 2);
				if (same6_max >= diff6_min)
				{
					return false;
				}
				break;
			default:
				cout << "this point's neighbour have nocie\n";
			}
		}
	}


	if (g_imgFlg.at<int>(D.x, D.y) == 0)//no tag
	{
		if (g_maxGrad.at<float>(D.x, D.y) >= 8 || g_minGrad.at<float>(D.x, D.y) <= -8)
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

			g_imgFlg.at<int>(D.x, D.y) = 2;


			//tage both size along the edge
			g_imgFlg.at<int>(B.x + a8i[(dBA + 1) % 8], B.y + a8j[(dBA + 1) % 8]) = 2;
			g_imgFlg.at<int>(B.x + a8i[(dBA + 2) % 8], B.y + a8j[(dBA + 2) % 8]) = 2;
			g_imgFlg.at<int>(B.x + a8i[(dBA - 1 + 8) % 8], B.y + a8j[(dBA - 1 + 8) % 8]) = 2;
			g_imgFlg.at<int>(B.x + a8i[(dBA - 2 + 8) % 8], B.y + a8j[(dBA - 2 + 8) % 8]) = 2;
		}
		else
		{
			g_edge[0].at<float>(B.x, B.y) = 255;
			g_edge[1].at<float>(B.x, B.y) = 255;
			g_edge[2].at<float>(B.x, B.y) = 0;
			return false;
		}


	}
	else//collision,tag contact point
	{

		g_edge[0].at<float>(D.x, D.y) = 255;
		g_edge[1].at<float>(D.x, D.y) = 0;
		g_edge[2].at<float>(D.x, D.y) = 0;

		return false;
	}


	pt.clear(), pt_next4.clear(), pt_next5.clear(), pt_next6.clear();
	return true;
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

void Edge::edgeDetection1()
{
	int dx, dy, direction;//边缘的方向相对坐标
	Point2i A, B, AC, BC, D, E, centPt, stPt;

	g_medImg.convertTo(g_medImg, CV_32F);
	int num = 0;
	bool keyi = true;
	int i, j;
	bool min_max;
	vector<Point2i> stretchD1, stretchE1, stretchD2, stretchE2, startP;
	//找到前两个边缘点作为起始延伸点
	for (i = 1; i < gray.rows - 1; i++)
	{
		for (j = 1; j < gray.cols - 1; j++)
		{
			//起点位置的寻找限制条件，起点及其8邻阈不得有已标记点
			int flagij = g_imgFlg.at<int>(i, j);


			if (!outOfrange(i, j) && flagij == 0)//TH可修改为动态阈值
			{
				centPt.x = i;
				centPt.y = j;
				//1根椐findStartPt1得到起始点边缘类型
				bool find = pfindStart(centPt, A, B, AC, BC, direction, min_max);
#if (START==1)
				if (find)
#endif

#if (START==2)
				if (findStartPt2(centPt, A, B, AC, BC, direction, min_max))
#endif		////nSection(centPt)==2&&			
				{
					g_numPixel++;
					startPt++;
					startP.push_back(A);
					startP.push_back(B);
					//startP.push_back(AC);
					//startP.push_back(BC);
					startP.push_back(centPt);

					//3分别向两个方向进行延伸	
					keyi = false;
					//keyi = true;
					stPt = centPt;
					while (keyi){

#if (STRETCH==2||STRETCH==3)
						keyi = TraceEdgeTwoBoundary2(stPt, A, AC, D, E, direction, min_max);
#endif
#if (STRETCH==1)
						//keyi = TraceEdgeTwoBoundary1(stPt, A, AC, D, E, direction);
						keyi = TraceEdgeTwoBoundary3(stPt, A, D, direction, min_max);
#endif
						g_numPixel++;
						stretchD1.push_back(D);
						//stretchE1.push_back(E);


						stPt = A;
						A = D;
						//AC = E;
					}



					stPt = centPt;
					keyi = false;
					//keyi = true;
					while (keyi){
#if (STRETCH==2||STRETCH==3)
						keyi = TraceEdgeTwoBoundary2(stPt, B, BC, D, E, -direction, min_max);
#endif
#if (STRETCH==1)
						//keyi = TraceEdgeTwoBoundary1(stPt, B, BC, D, E, -direction);
						keyi = TraceEdgeTwoBoundary3(stPt, B, D, -direction, min_max);
#endif
						g_numPixel++;
						stretchD2.push_back(D);
						//stretchE2.push_back(E);

						stPt = B;
						B = D;
						//BC = E;
					}

					if (stretchD1.size() <= LTH&&stretchD2.size() <= LTH)
					{
						//清除
						//clearShort(stretchD1);
						//clearShort(stretchD2);
						//clearShort(stretchE1);
						//clearShort(stretchE2);
						//clearShort(startP);
						//clearShort(g_missedPt);

					}

					stretchD1.clear();
					stretchD2.clear();
					stretchE1.clear();
					stretchE2.clear();
					startP.clear();
					g_missedPt.clear();
					g_circle = Mat::zeros(gray.rows, gray.cols, CV_32S); // ready for next start point
				}




			}

		}

	}


	//输出混合图像【边缘+原图】
	getMix();

	//得到混合的彩色边缘点云图
	g_edge[0].convertTo(g_edge[0], CV_8UC1);
	g_edge[1].convertTo(g_edge[1], CV_8UC1);
	g_edge[2].convertTo(g_edge[2], CV_8UC1);
	merge(g_edge, g_mergImg);

	imshow("g_mergImg", g_mergImg); //imshow("listk1", listK1); imshow("listk1&&listk2", mergImg);


	imwrite("./output/median.bmp", g_mergImg);
	//outxls(g_calImg, "./output/change.xls");

	//g_calImg.convertTo(g_calImg, CV_8UC1);
	//imwrite("./output/change.bmp", g_calImg);

	merge(g_srcImageChannels, g_srcImageBGR);
	//imshow("g_srcImageBGR", g_srcImageBGR);
	imwrite("./output/tagEdge.bmp", g_srcImageBGR);
	//(g_edge[1].clone()).convertTo(g_outEdge, CV_32F);//最终边缘图

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
		str = "output/" + num2str(0) + num2str(nTH) + ".png";
		//str1 = "output/" + num2str(0) + num2str(nTH) + ".bmp";
	}
	else
		str = "output/" + num2str(nTH) + ".png";

	//imwrite(str1, g_mergImg);
	imwrite(str, bimap);
	//imshow("边缘图", bimap);

	//得到不同星型的混合边缘图
	//getTypeEdge(false);

	//添加噪音
	//if (noice)
	//{
	//	salt(g_srcImage);
	//	//imwrite("./output/noice.bmp", g_srcImage);
	//}

	

	//以后的循环初始化
	nextInitial();


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

			if (MIXGAT == 1)//get gt+gray && gt+type
			{
				if (g_gtImg.at<float>(i, j) > 0)
				{
					g_mixGS.at<float>(i, j) *= 100; //get gt + gray
					g_type.at<int>(i, j) += 100;//gt + type
					cntPixel++;
				}
			}

		}

	}

	cout << "GT_PIXELS=" << cntPixel << endl;
	//得到不同星型的混合边缘图
	getTypeEdge();
}
void Edge::edgeDetection3(){
	//1 find a start edge that include three same pointypes

	//2 stretch the start edge from two direction
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

		//outxls(g_srcImageB, "./output/b.xls");
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
	//g_thresold = Mat::zeros(gray.rows, gray.cols, CV_32F);
	g_angle = Mat::ones(gray.rows, gray.cols, CV_32F)*(-1);
	g_intAngle = Mat::zeros(gray.rows, gray.cols, CV_32S);
	g_nTypeFlag = Mat::zeros(gray.rows, gray.cols, CV_32S);
	g_dstImage = Mat::zeros(gray.rows, gray.cols, CV_32F);
	//g_edge[1] = Mat::zeros(gray.rows, gray.cols, CV_32F);//初始化边缘图
	g_edge[0] = Mat::zeros(gray.rows, gray.cols, CV_32F);
	g_edge[1] = Mat::zeros(gray.rows, gray.cols, CV_32F);
	g_edge[2] = Mat::zeros(gray.rows, gray.cols, CV_32F);
	g_mergImg = Mat::zeros(gray.rows, gray.cols, CV_8UC3);
	g_type = Mat::zeros(gray.rows, gray.cols, CV_32S);
	g_srcImageBGR = Mat::zeros(gray.rows, gray.cols, CV_8UC3);

	//Mat g_imgFlg(gray.rows, gray.cols, CV_32S,Scalar::all(0));
	nTypeEdge[0] = Mat::zeros(gray.rows, gray.cols, CV_32S);
	nTypeEdge[1] = Mat::zeros(gray.rows, gray.cols, CV_32S);
	nTypeEdge[2] = Mat::zeros(gray.rows, gray.cols, CV_32S);
	g_imgFlg = Mat::zeros(gray.rows, gray.cols, CV_32S);//图像特征标记表，32位有符号整数::::::::::::::::::::::::::::::::::为何用8位无符号整形或有符号整形就会出错
}


//判断是否下标越界
bool outOfrange(int i, int j)
{
	if (i > 0 && i < gray.rows - 1 && j>0 && j < gray.cols - 1)
		return false;
	else
	{
		//cout << "超出边界范围！\n";
		return true;
	}

}


int  lengthAB(int A, int B, int direction)
{
	if (direction < 0)
	{
		/*if ((A+1)%8==B)
		{
		return 2;
		}*/
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
			return 0;
	}
	else//逆时针,-
	{
		/*if( (A+2)%8==B)
		{
		return 2;
		}*/
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
			return 0;
	}

}


//边缘初始化
void Edge::Init()
{
	g_tracenum = 0;
	g_thresold = Mat::ones(gray.rows, gray.cols, CV_32F)*(-1);
	g_type = Mat::zeros(gray.rows, gray.cols, CV_32S);
	g_angle = Mat::ones(gray.rows, gray.cols, CV_32F)*(-1);//*11可以统一改变初始值
	g_intAngle = Mat::zeros(gray.rows, gray.cols, CV_32S);
	//g_mixGS = Mat::zeros(gray.rows, gray.cols, CV_32F);
	g_dstImage = Mat::zeros(gray.rows, gray.cols, CV_32F);
	//g_edge[1] = Mat::zeros(gray.rows, gray.cols, CV_32F);//初始化边缘图
	g_edge[0] = Mat::zeros(gray.rows, gray.cols, CV_32F);
	g_edge[1] = Mat::zeros(gray.rows, gray.cols, CV_32F);
	g_edge[2] = Mat::zeros(gray.rows, gray.cols, CV_32F);
	g_mergImg = Mat::zeros(gray.rows, gray.cols, CV_8UC3);
	g_circle = Mat::zeros(gray.rows, gray.cols, CV_32S);
	g_srcImageBGR = Mat::zeros(gray.rows, gray.cols, CV_8UC3);

	//Mat g_imgFlg(gray.rows, gray.cols, CV_32S,Scalar::all(0));
	g_imgFlg = Mat::zeros(gray.rows, gray.cols, CV_32S);//图像特征标记表，32位有符号整数::::::::::::::::::::::::::::::::::为何用8位无符号整形或有符号整形就会出错
	g_nTypeFlag = Mat::zeros(gray.rows, gray.cols, CV_32S);
	g_nSectionFlg = Mat::zeros(gray.rows, gray.cols, CV_32S);//

	nTypeEdge[0] = Mat::zeros(gray.rows, gray.cols, CV_32S);
	nTypeEdge[1] = Mat::zeros(gray.rows, gray.cols, CV_32S);
	nTypeEdge[2] = Mat::zeros(gray.rows, gray.cols, CV_32S);


	g_nType = Mat::ones(gray.rows, gray.cols, CV_32S) * 8;//
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

	maxNum = Mat::zeros(gray.rows, gray.cols, CV_32S);
	maxTH = Mat::zeros(gray.rows, gray.cols, CV_32F);

	//【中值滤波】
	medianBlur(gray, g_medImg, 3);

	gray.copyTo(g_calImg);
	gray.copyTo(g_mixGS);
	//g_srcImage.copyTo(g_calImg);
	if (rgb)
	{
		srcBGR[0].convertTo(srcBGR[0], CV_32F);
		srcBGR[1].convertTo(srcBGR[1], CV_32F);
		srcBGR[2].convertTo(srcBGR[2], CV_32F);

	}
	else
	{
		g_calImg.convertTo(g_calImg, CV_32F);//转换类型，便于加减运算
		g_mixGS.convertTo(g_mixGS, CV_32F);
	}


	//namedWindow("【中值滤波】");
	//imshow("【中值滤波】", g_medImg);
	float tempG[8] = { 0 };
	float maxG = 0, minG = 0;
	int x, y;
	gray.convertTo(gray, CV_32F);
	g_maxGrad = Mat::zeros(gray.rows, gray.cols, CV_32F);

	float min, max, mint, maxt, maxVal;
	int m, n;
	float point_9[9];
	

	//计算图像梯度，第一行最后一行第一列最后一列先不考虑梯度，设定为0
	for (int i = 1; i < gray.rows - 1; i++)
	{
		for (int j = 1; j < gray.cols - 1; j++)
		{
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

			//point[8] is centerpoint
			point_9[8] = g_calImg.at<float>(i, j);
			for (int k = 0; k < 8; ++k)
			{
				m = a8i[k];
				n = a8j[k];
				point_9[k] = g_calImg.at<float>(i + m, j + n);
			}

			maxNum.at<int>(i, j) = HandlePoint(point_9, point_9[8], maxVal);  //保存中心点所在部分长度
			maxTH.at<float>(i, j) = maxVal;  //记录最大差值 ，作为阈值
			/*for (int k = 0; k < 8; ++k)
			{
			m = a8i[k];
			n = a8j[k];

			mint = g_calImg.at<float>(i + m, j + n) - g_calImg.at<float>(i, j);
			maxt = g_calImg.at<float>(i + m, j + n) - g_calImg.at<float>(i, j);
			if (mint < min)
			{
			min = mint;
			}
			if (maxt>max)
			{
			max = maxt;
			}
			}

			if (i >= 0 && i<minValue.rows && j >= 0 && j< minValue.cols)
			minValue.at<float>(i, j) = min;
			if (i >= 0 && i<maxValue.rows && j >= 0 && j< maxValue.cols)
			maxValue.at<float>(i, j) = max;*/

		}
	}

	outXls("./output/maxNum.xls", maxNum, "int");
	outXls("./output/maxTH.xls", maxTH, "float");
	maxTH.convertTo(maxTH, CV_8UC1);
	namedWindow("maxTH1");
	imshow("maxTH1", maxTH);
	//showHistogram(g_minGrad);
	showHistogram(maxTH, "maxTH");
	showHistogram(g_maxGrad, "maxGrad");
	//showHistogram(g_minGrad, "negative");

	//get th from higram


	//梯度图5*5>8进行个数统计
	//filter2D(g_biGrad, g_dstBiGrad, -1, g_kernel, Point(-1, -1), 0.0, BORDER_DEFAULT);



}

//Canny检测函数
void Edge::on_Canny(int, void*)
{
	//cvtColor(g_srcImage, g_srcGrayImage, CV_BGR2GRAY);
	// 先使用 3x3内核来降噪
	g_srcGrayImage.convertTo(g_cannyDetectedEdges, CV_8UC1);
	//blur(g_srcGrayImage, g_cannyDetectedEdges, Size(3, 3));

	// 运行我们的Canny算子  
	Canny(g_cannyDetectedEdges, g_cannyDetectedEdges, g_cannyLowThreshold, g_cannyLowThreshold * 3, 3);

	////先将g_dstImage内的所有元素设置为0  
	//g_dstImage = Scalar::all(0);

	////使用Canny算子输出的边缘图g_cannyDetectedEdges作为掩码，来将原图g_srcImage拷到目标图g_dstImage中  
	//g_srcImage.copyTo(g_dstImage, g_cannyDetectedEdges);

	//显示效果图  
	//namedWindow("Canny", CV_WINDOW_NORMAL);
	imshow("canny", g_cannyDetectedEdges);
	imwrite("./canny.png", g_cannyDetectedEdges);
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
	//system("color 2f");//设置控制台背影颜色
	//---------------------------------------------------------------

	//接口控制函数，1表示是，0表示否
	controlFuction(10,//黄色边
		10,//是否减边，红色
		0,//是否实边填充,1为实边填充，0为隐边填充
		0,//是否填充邻边
		10,//是否显示初始绿点
		1,//是否导出数据
		0,//是否显示叠加色
		0);//是否添加噪音
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
void controlFuction(bool n1, bool n2, bool n3, bool n4, bool n5, bool n6, bool n7, bool n8)
{
	real_edge = n1;
	virtual_edge = n2;
	real_fill = n3;
	bufill = n4;
	green = n5;
	xls = n6;
	init_edge = n7;
	noice = n8;
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

void deNoice(int row, int col)
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
	imwrite("./output/noicePicture.bmp", image);
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
//填充邻边

float sum(Mat mat, int n)
{
	float sum = 0.0;
	for (int i = 0; i <= n; i++)
	{
		sum += mat.at<float>(i, 0);
	}
	return sum;
}



