// ---------------------------------- - ������˵����----------------------------------------------
//      ��������:OpenCV��Ե��⣺�·���ʵ�ֱ�Ե������ȡ���ܶ�λ����
//      ��������IDE�汾��Visual Studio 2013  
//      <span style="white-space:pre">  </span>��������OpenCV�汾��    2.4.9  
//      2014��11��19�� Create by �  
//    
//----------------------------------------------------------------------------------------------  

//-----------------------------------��ͷ�ļ��������֡�---------------------------------------  
//      ����������������������ͷ�ļ�  
//----------------------------------------------------------------------------------------------  
#include "edge.h"  
#include"stdlib.h"
Edge::Edge(Mat in)
{
	if (in.channels() == 3)
	{
		rgb = true;
		imshow("��ԭͼ��", in);
		split(in, srcBGR);
		cvtColor(in, gray, CV_BGR2GRAY);
	}
	else
	{
		namedWindow("ԭͼ");
		imshow("ԭͼ", in);
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

//��ɫͼ���������ֵ�����ڱȽ��Ƿ�����ͬһ������
float absAB_BGR(Point2i A, int i, int j)
{
	float tmp;
	tmp = abs(srcBGR[0].at<float>(A.x, A.y) - srcBGR[0].at<float>(i, j)) +
		abs(srcBGR[1].at<float>(A.x, A.y) - srcBGR[1].at<float>(i, j)) +
		abs(srcBGR[2].at<float>(A.x, A.y) - srcBGR[2].at<float>(i, j));
	return tmp;
}

//���룺һ����centerPt
//������жϴ˵��Ƿ�Ϊ��Ե�㣬�������������ڵ�������Ե��A,B,�����ĵ㲻ͬ�ĵ�AC��BC
bool findStartPt(Point2i centerPt, Point2i &A, Point2i &B, Point2i &AC, Point2i &BC, int &direction)
{
	bool  flagA, flagB;//����Ƿ��ҵ�A/B��
	flagA = flagB = false;
	int nType;
	Point2i stPt, E;
	int i, j;
	int bdA, bdB, bdTmp;
	int dx, dy;
	i = centerPt.x;
	j = centerPt.y;
	//�����ı�Ե�����ڵ�������Ե��A��B��
	int indext;
	bool kflagMissed = false;
	Point2i missedPt;
	int dA, dB, dAC, dBC;
	float CV;
	int flagkk = g_imgFlg.at<int>(centerPt.x, centerPt.y);
	if (flagkk)
		return false;

	//���溯�������ҵ����ĵ����ͬ����ֹ��A��B����ͬ����ֹ��AC��BC
	if (rgb)//��ɫͼ��
	{
		//�����һ��������ĵ���ͬ�����������������е㲻��ͬ�ĵ�
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
		else{//�����һ��������ĵ㲻��ͬ�����������������е���ͬ�ĵ�
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

	//�Ҷ�ͼ��
	else
	{
		//�����һ��������ĵ���ͬ�����������������е㲻��ͬ�ĵ�
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
		else{//�����һ��������ĵ㲻��ͬ�����������������е���ͬ�ĵ�
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

	//����Direction,�����²��С��0����Ӱ�ƽ���˳ʱ�뿪ʼ����
	direction = (A.x - centerPt.x)*(AC.y - centerPt.y) - (A.y - centerPt.y)*(AC.x - centerPt.x);
	dx = A.x - centerPt.x;
	dy = A.y - centerPt.y;

	//�õ���A��B�󣬼������Ե����
	//��A��B��תΪ����±����������������Ϊ��Ե����
	if (outOfrange(A.x, A.y) || outOfrange(B.x, B.y) || outOfrange(AC.x, AC.y) || outOfrange(BC.x, BC.y))
	{
		return false;//��ֹԽ��
	}
	if (g_imgFlg.at<int>(A.x, A.y) || g_imgFlg.at<int>(B.x, B.y) || g_imgFlg.at<int>(AC.x, AC.y) || g_imgFlg.at<int>(BC.x, BC.y))
		return false;//�Ѿ������


	dA = getIangle(centerPt, A);
	dB = getIangle(centerPt, B);
	nType = lengthAB(dA, dB, direction);
	bool isStartPt = startPoint(centerPt.x, centerPt.y);

	if (flagA&&flagB&&nType > 2 && nType < 7 && isStartPt)//nSection(centerPt) &&
	{
		//�ж����������Ƿ�Ϊ������
		//���룺centerPt,A,B AC,BC 
		//�����true or false
		//A-B�����Ƿ�<TH��AC��BC�Ƿ�>TH
		//*/û����չ����ɫͼ����
		int dA = getIangle(centerPt, A);
		int dB = getIangle(centerPt, B);
		int dAC = getIangle(centerPt, AC);
		int dBC = getIangle(centerPt, BC);
		int tmpV;
		int tmpC;
		tmpC = g_calImg.at<float>(centerPt.x, centerPt.y);
		//ƽ��������
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
					//ȥ����D�����ڵĵ㣬�������˵㣬�������������죺�μ�����/ÿ������/��������
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
					//ȥ����D�����ڵĵ㣬�������˵㣬�������������죺�μ�����/ÿ������/��������
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


			//��������AC-BC
			//kflagMissed = GetMissedPoint(centerPt, AC, BC, direction, missedPt);

			//dAC = getIangle(centerPt, AC);
			//dBC = getIangle(centerPt, BC);
			//�����ĵ㲻��ͬ������ĵ�С�ı��Ϊ��ɫ

			float comR;//�Ƚϴ�С
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
				if (virtual_edge)//���
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
			else{//�����ĵ㲻��ͬ���ұ����ĵ��ı��Ϊ��ɫ

				//*
				if (real_edge)//С��
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
		//���ݵķ����getth�ķ����෴��geth�õ������췽�򣬶���ʱ����㷽��

		//if (g_thresold.at<float>(centerPt.x, centerPt.y) > 0)
		//{
		//	cout << "ll=" << g_thresold.at<float>(centerPt.x, centerPt.y) << endl;
		//	return true;//����ҵ�A��B������Ե����ʼ��������׷�ٺ���TraceEdgeTwoBoundary
		//}
		//else
		//	return false;

		return true;//����ҵ�A��B������Ե����ʼ��������׷�ٺ���TraceEdgeTwoBoundary
	}
	else
		return false;


}

//Trace two sides of the boundary
//direction; 0  denotes  counterclockwise
//���룺��A��B��C ,direction    A��BΪ���ڱ�Ե�㣬BΪ���ĵ㣬CΪ��AB��ͬ��ĵ�,directionΪ����
//�������D��E     DΪB�İ�ƽ�������һ����B������ĵ㣬EΪ��ƽ���ڵ�һ����B�����Ƶĵ�
bool TraceEdgeTwoBoundary(Point2i A, Point2i B, Point2i C, Point2i &D, Point2i &E, int  direction)
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
				if (absAB_BGR(B, Pts[i].x, Pts[i].y) < absAB_BGR(C, Pts[i].x, Pts[i].y))//�������ֵ����������&&�Ľ�Ϊ�ҵ����һ����B��ͬ�ĵ㣬ǰ���Ϊ������
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
	else//�Ҷ�ͼ��
	{
		float  BV, CV, tempV, tempV1;
		BV = g_calImg.at<float>(B.x, B.y);
		CV = g_calImg.at<float>(C.x, C.y);

		for (i = 4; i >= 0; i--)//i==4����Ϊ���쵽����һ������
		{
			/*if (g_imgFlg.at<int>(Pts[i].x, Pts[i].y) > 0)
			{
			return false;
			}*/
			if (!outOfrange(Pts[i].x, Pts[i].y))//i + 1����Խ�����±�&& 
			{
				tempV = g_calImg.at<float>(Pts[i].x, Pts[i].y);
				//tempV1 = g_calImg.at<float>(Pts[i + 1].x, Pts[i + 1].y);
				//find the last point[LP] which similar to centerPoint[B]
				if (abs(tempV - BV) <= abs(tempV - CV))// ����ȥ��һЩ�Ǳ�Ե�� && abs(tempV - BV) <TH_D&& abs(tempV - CV) >TH_D && abs(tempV - BV) <TH1&& abs(tempV - CV) >TH1 && abs(tempV1 - BV) > abs(tempV1 - CV)�������ֵ����������&&�Ľ�Ϊ�ҵ����һ����B��ͬ�ĵ㣬ǰ���Ϊ������ && abs(tempV1 - BV) > abs(tempV1 - CV)
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
							return false;//ˮƽ����ȫ����ͬ

					}
					else if (i == 0)
					{
						return false;//�ۻص�Ĵ���
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
				//Ptrs[0]-LP֮�������ĵ��ֵȥ��䡣				

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
		if (outOfrange(D.x, D.y) || g_imgFlg.at<int>(D.x, D.y))//|| outOfrange(E.x, E.y) || g_imgFlg.at<int>(E.x, E.y)
		{
			D.x = 1;
			D.y = 1;
			return false;

		}


		g_thresold.at<float>(B.x, B.y) = getDth(A, B, C, D, E, direction);
		TH_D = g_thresold.at<float>(B.x, B.y);

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
						//�������λ�ü���ɫ
						/*g_edge[0].at<float>(B.x + a8i[dA], B.y + a8j[dA]) = 255;
						g_edge[1].at<float>(B.x + a8i[dA], B.y + a8j[dA]) = 255;
						g_edge[2].at<float>(B.x + a8i[dA], B.y + a8j[dA]) = 0;*/
					}
					//g_calImg.at<float>(B.x + a8i[dA], B.y + a8j[dA]) = BV;

				}
				if (dA + 1 != dD)
				{
					//ȥ����D�����ڵĵ㣬�������˵㣬�������������죺�μ�����/ÿ������/��������
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
			//		//������Ҫ���
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
						//�������λ�ü���ɫ
						/*g_edge[0].at<float>(B.x + a8i[dA], B.y + a8j[dA]) = 255;
						g_edge[1].at<float>(B.x + a8i[dA], B.y + a8j[dA]) = 255;
						g_edge[2].at<float>(B.x + a8i[dA], B.y + a8j[dA]) = 0;*/
					}
					//g_calImg.at<float>(B.x + a8i[dA], B.y + a8j[dA]) = BV;

				}
				if (dA - 1 != dD)
				{
					//ȥ����D�����ڵĵ㣬�������˵㣬�������������죺�μ�����/ÿ������/��������
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
		float comR;//�Ƚϴ�С
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
			if (virtual_edge)//���
			{
				g_edge[0].at<float>(D.x, D.y) = 0;
				g_edge[1].at<float>(D.x, D.y) = 0;
				g_edge[2].at<float>(D.x, D.y) = 255;
				g_imgFlg.at<int>(D.x, D.y) = 2;
			}


			if (real_edge)//С��
			{
				bluePt++;

				if (g_imgFlg.at<int>(E.x, E.y) == g_imgFlg.at<int>(D.x, D.y))
				{
					g_edge[0].at<float>(D.x, D.y) = 0;
					g_edge[1].at<float>(D.x, D.y) = 0;
					g_edge[2].at<float>(D.x, D.y) = 255;
					collision = true;
					//return false;//ȥ��˫��ͬ��
				}
				if (!g_imgFlg.at<int>(E.x, E.y))
				{

					g_imgFlg.at<int>(E.x, E.y) = 1;
				}

			}

		}
		else{
			if (real_edge)//С��
			{
				bluePt++;
				g_edge[0].at<float>(D.x, D.y) = 0;
				g_edge[1].at<float>(D.x, D.y) = 255;
				g_edge[2].at<float>(D.x, D.y) = 255;
				g_imgFlg.at<int>(D.x, D.y) = 1;
			}

			if (virtual_edge)//���
			{
				//E�Ѿ������
				if (g_imgFlg.at<int>(E.x, E.y) == g_imgFlg.at<int>(D.x, D.y))
				{
					g_edge[0].at<float>(D.x, D.y) = 0;
					g_edge[1].at<float>(D.x, D.y) = 255;
					g_edge[2].at<float>(D.x, D.y) = 255;
					collision = true;
					//return false;
				}
				//Eδ�����
				if (!g_imgFlg.at<int>(E.x, E.y))
				{
					g_imgFlg.at<int>(E.x, E.y) = 2;
				}

			}
		}

		//����Ƿ���������һ��
		if (collision)
		{
			return false;
		}
		else
			return true;


	}

}
//���룺A��B��C��D��E��direction--[BΪ����������ĵ㣬D��EΪ��һ��B��C]
//�������̬��ֵth g_thresold.at<float>(centerPt.x, centerPt.y) = getDth(A, centerPt, AC, B, BC, direction);
float getDth(Point2i A, Point2i B, Point2i C, Point2i D, Point2i E, int direction)//  ABD  CE
{
	float dt = 0;
	int dA, dD, dC, dE, dTmp;
	vector<Point2i> pt1, pt2, pt3;
	Point2i tmp;
	int tmp1[8] = { -1, -1, -1, -1, -1, -1, -1, -1 }, tmp2[2] = { 256 }, tmp3[8] = { 256, 256, 256, 256, 256, 256, 256, 256 };//��ʼ��������
	float max1, max2, max3, min;
	dA = getIangle(B, A);
	dD = getIangle(B, D);
	dC = getIangle(B, C);
	dE = getIangle(B, E);
	int BV = g_calImg.at<float>(B.x, B.y);
	if (direction > 0)
	{
		//1���Եͬ��ĵ�
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

		//2A,B,D��Ե��
		pt2.push_back(A);
		pt2.push_back(B);
		pt2.push_back(D);

		//3���Ե���
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
		//1���Եͬ��ĵ�
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

		//2A,B,D��Ե��
		pt2.push_back(A);
		pt2.push_back(B);
		pt2.push_back(D);

		//3���Ե���
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
			//pt3.push_back(E);//��E�Ž�������

		}

	}

	int tmpV;

	//��Ե����ͬ��ȡ���
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


	//����tmp2�����õ�
	tmp2[0] = abs(g_calImg.at<float>(pt2.at(0).x, pt2.at(0).y) - g_calImg.at<float>(pt3.at(0).x, pt3.at(0).y));//A - C
	int l3;
	l3 = pt3.size() - 1;
	tmp2[1] = abs(g_calImg.at<float>(pt2.at(2).x, pt2.at(2).y) - g_calImg.at<float>(pt3.at(l3).x, pt3.at(l3).y));//D - E
	//min = findMin(tmp2, 2);

	//���Ե�������ȡ��С
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

void Edge::edgeDection()
{
	int dx, dy, direction;//��Ե�ķ����������
	Point2i A, B, AC, BC, D, E, centPt, stPt;

	g_medImg.convertTo(g_medImg, CV_32F);
	int num = 0;
	bool keyi = true;
	int i, j;
	vector<Point2i> stretchD1, stretchE1, stretchD2, stretchE2, startP;
	//�ҵ�ǰ������Ե����Ϊ��ʼ�����
	for (i = 1; i < gray.rows - 1; i++)
	{
		for (j = 1; j < gray.cols - 1; j++)
		{
			//���λ�õ�Ѱ��������������㼰��8���в������ѱ�ǵ�
			int flagij = g_imgFlg.at<int>(i, j);
			if (!outOfrange(i, j) && flagij == 0)//TH���޸�Ϊ��̬��ֵ
			{
				centPt.x = i;
				centPt.y = j;
				//1���findStartPt�õ���ʼ���Ե����
				if (findStartPt(centPt, A, B, AC, BC, direction))//nSection(centPt)==2&&
				{
					startPt++;
					startP.push_back(A);
					startP.push_back(B);
					//startP.push_back(AC);
					//startP.push_back(BC);
					startP.push_back(centPt);

					//3�ֱ������������������	
					keyi = true;
					stPt = centPt;
					while (keyi){
						keyi = TraceEdgeTwoBoundary(stPt, A, AC, D, E, direction);

						stretchD1.push_back(D);
						stretchE1.push_back(E);


						stPt = A;
						A = D;
						AC = E;
					}



					stPt = centPt;
					keyi = true;

					while (keyi){
						keyi = TraceEdgeTwoBoundary(stPt, B, BC, D, E, -direction);

						stretchD2.push_back(D);
						stretchE2.push_back(E);

						stPt = B;
						B = D;
						BC = E;
					}

					if (stretchD1.size() <= LTH&&stretchD2.size() <= LTH)
					{
						//���
						clearShort(stretchD1);
						clearShort(stretchD2);
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

				}


			}

		}

	}

	if (xls)
	{
		outxls(g_srcGrad, ".\\output\\grad.xls");
		outxls(g_angle, "./output/angle.xls");
		outxlsInt(g_imgFlg, "./output/imgflag.xls");

	}


	//outxlsInt(g_dstBiGrad, "./output/dstBigrad.xls");

	//������ͼ�񡾱�Ե+ԭͼ��
	getMix();

	//�õ���ϵĲ�ɫ��Ե����ͼ
	g_edge[0].convertTo(g_edge[0], CV_8UC1);
	g_edge[1].convertTo(g_edge[1], CV_8UC1);
	g_edge[2].convertTo(g_edge[2], CV_8UC1);
	merge(g_edge, g_mergImg);
	imshow("g_mergImg", g_mergImg); //imshow("listk1", listK1); imshow("listk1&&listk2", mergImg);

	imwrite("./output/median.bmp", g_mergImg);
	outxls(g_calImg, "./output/change.xls");

	g_calImg.convertTo(g_calImg, CV_8UC1);
	imwrite("./output/change.bmp", g_calImg);




	merge(g_srcImageChannels, g_srcImageBGR);
	imshow("g_srcImageBGR", g_srcImageBGR);
	imwrite("./output/tagEdge.bmp", g_srcImageBGR);
	//(g_edge[1].clone()).convertTo(g_outEdge, CV_32F);//���ձ�Եͼ

	//���õ��Ĳ�ɫ��Եͼ��ֵ��
	Mat bimap;
	cvtColor(g_mergImg, bimap, CV_BGR2GRAY);
	threshold(bimap, bimap, 2, 255, THRESH_BINARY);
	imwrite("./output/edge.bmp", bimap);
	imshow("��Եͼ", bimap);

	//�õ���ͬ���͵Ļ�ϱ�Եͼ
	//	getTypeEdge();

	//�������
	if (noice)
	{
		salt(g_srcImage);
		//imwrite("./output/noice.bmp", g_srcImage);
	}
	//�Ժ��ѭ����ʼ��
	//nextInitial();


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
//����һ���㣬������Ƿ����������������Ϸ���true
bool startPoint(int i, int j)
{
	//�����ڴ�����һ���±�Խ���쳣�����ж�
	//�ж�8�������Ƿ��б�ǵ�
	for (int k = 0; k < 8; k++)
	{
		if (g_imgFlg.at<int>(i + a8i[k], j + a8j[k]) && !outOfrange(i + a8i[k], j + a8j[k]))
			return false;
	}
	return true;
}
bool isTag(Point2i centPt)
{
	//�ж�8�������Ƿ��б�ǵ�
	for (int k = 0; k < 8; k++)
	{
		if (!outOfrange(centPt.x + a8i[k], centPt.y + a8j[k]) && g_nSectionFlg.at<int>(centPt.x + a8i[k], centPt.y + a8j[k]) == 1)
			return true;
	}
	return false;
}
void getMix()
{

	if (g_srcImage.channels() == 3)//��������ͨ��
	{
		split(g_srcImage, g_srcImageChannels);//��ԭͼ��������ͨ��

		//imshow("�ڶ���ͨ��", g_srcImageChannels.at(1));
		g_srcImageChannels.at(0).convertTo(g_srcImageChannels.at(0), CV_32F);
		g_srcImageChannels.at(1).convertTo(g_srcImageChannels.at(1), CV_32F);
		g_srcImageChannels.at(2).convertTo(g_srcImageChannels.at(2), CV_32F);

		//outxls(g_srcImageB, "./output/b.xls");
	}
	//else if (g_srcImage.channels() == 1)//ת��Ϊ��ͨ��
	//{
	//	//����һ����ͨ��ͼ��
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

	g_dstImage = Mat::zeros(gray.rows, gray.cols, CV_32F);
	//g_edge[1] = Mat::zeros(gray.rows, gray.cols, CV_32F);//��ʼ����Եͼ
	g_edge[0] = Mat::zeros(gray.rows, gray.cols, CV_32F);
	g_edge[1] = Mat::zeros(gray.rows, gray.cols, CV_32F);
	g_edge[2] = Mat::zeros(gray.rows, gray.cols, CV_32F);
	g_mergImg = Mat::zeros(gray.rows, gray.cols, CV_8UC3);

	g_srcImageBGR = Mat::zeros(gray.rows, gray.cols, CV_8UC3);

	//Mat g_imgFlg(gray.rows, gray.cols, CV_32S,Scalar::all(0));

	//g_imgFlg = Mat::zeros(gray.rows, gray.cols, CV_32S);//ͼ��������Ǳ�32λ�з�������::::::::::::::::::::::::::::::::::Ϊ����8λ�޷������λ��з������ξͻ����
}


//�ж��Ƿ��±�Խ��
bool outOfrange(int i, int j)
{
	if (i > 0 && i < gray.rows - 1 && j>0 && j < gray.cols - 1)
		return false;
	else
	{
		//cout << "�����߽緶Χ��\n";
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
		else
			return 0;
	}
	else//��ʱ��
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
		else
		{
			return 0;
		}
	}

}


//��Ե��ʼ��
void Edge::Init()
{
	g_tracenum = 0;
	g_thresold = Mat::ones(gray.rows, gray.cols, CV_32F)*(-1);
	g_angle = Mat::ones(gray.rows, gray.cols, CV_32F)*(-1);//*11����ͳһ�ı��ʼֵ
	g_intAngle = Mat::zeros(gray.rows, gray.cols, CV_32S);

	g_dstImage = Mat::zeros(gray.rows, gray.cols, CV_32F);
	//g_edge[1] = Mat::zeros(gray.rows, gray.cols, CV_32F);//��ʼ����Եͼ
	g_edge[0] = Mat::zeros(gray.rows, gray.cols, CV_32F);
	g_edge[1] = Mat::zeros(gray.rows, gray.cols, CV_32F);
	g_edge[2] = Mat::zeros(gray.rows, gray.cols, CV_32F);
	g_mergImg = Mat::zeros(gray.rows, gray.cols, CV_8UC3);

	g_srcImageBGR = Mat::zeros(gray.rows, gray.cols, CV_8UC3);

	//Mat g_imgFlg(gray.rows, gray.cols, CV_32S,Scalar::all(0));
	g_imgFlg = Mat::zeros(gray.rows, gray.cols, CV_32S);//ͼ��������Ǳ�32λ�з�������::::::::::::::::::::::::::::::::::Ϊ����8λ�޷������λ��з������ξͻ����
	g_nSectionFlg = Mat::zeros(gray.rows, gray.cols, CV_32S);//


	g_nType = Mat::zeros(gray.rows, gray.cols, CV_32S);//

	g_biGrad = Mat::zeros(gray.rows, gray.cols, CV_32F);
	g_dstBiGrad = Mat::zeros(gray.rows, gray.cols, CV_32F);

	//����ֵ�˲���
	medianBlur(gray, g_medImg, 3);

	gray.copyTo(g_calImg);
	//g_srcImage.copyTo(g_calImg);
	if (rgb)
	{
		srcBGR[0].convertTo(srcBGR[0], CV_32F);
		srcBGR[1].convertTo(srcBGR[1], CV_32F);
		srcBGR[2].convertTo(srcBGR[2], CV_32F);

	}
	else
		g_calImg.convertTo(g_calImg, CV_32F);//ת�����ͣ����ڼӼ�����


	//namedWindow("����ֵ�˲���");
	//imshow("����ֵ�˲���", g_medImg);
	float tempG[8] = { 0 };
	float maxG = 0, minG = 0;
	gray.convertTo(gray, CV_32F);
	g_srcGrad = Mat::zeros(gray.rows, gray.cols, CV_32F);

	//����ͼ���ݶȣ���һ�����һ�е�һ�����һ���Ȳ������ݶȣ��趨Ϊ0
	for (int i = 1; i < gray.rows - 1; i++)
	{
		for (int j = 1; j < gray.cols - 1; j++)
		{
			for (int l = 0; l < 8; l++)
			{
				tempGrad[l] = abs(gray.at<float>(i + a8i[l], j + a8j[l]) - gray.at<float>(i, j));
			}
			g_srcGrad.at<float>(i, j) = findMax(tempGrad, 8);

			if (g_srcGrad.at<float>(i, j)>gTH)
			{
				g_biGrad.at<int>(i, j) = 1;
			}
		}
	}
	//�ݶ�ͼ5*5>8���и���ͳ��
	filter2D(g_biGrad, g_dstBiGrad, -1, g_kernel, Point(-1, -1), 0.0, BORDER_DEFAULT);



}
void Edge::outXls(Mat listK1, string filename)
{

	ofstream outEdge(filename);
	for (int i = 0; i < listK1.rows; i++)
	{
		for (int j = 0; j < listK1.cols; j++)
		{
			outEdge << listK1.at<float>(i, j) << "\t";
		}
		outEdge << "\n";
	}
	outEdge.close();
}
//�ⲿ��������������û�б�Ե���󴴽�ʱ���ݵĵ���
//template<class T>
void outxls(Mat listK1, string filename)
{
	ofstream outEdge(filename);
	for (int i = 0; i < listK1.rows; i++)
	{
		for (int j = 0; j < listK1.cols; j++)
		{
			outEdge << listK1.at<float>(i, j) << "\t";
		}
		outEdge << "\n";
	}
	outEdge.close();
}
void outxlsInt(Mat listK1, string filename)
{
	ofstream outEdge(filename);
	for (int i = 0; i < listK1.rows; i++)
	{
		for (int j = 0; j < listK1.cols; j++)
		{
			outEdge << listK1.at<int>(i, j) << "\t";
		}
		outEdge << "\n";
	}
	outEdge.close();
}
//Canny��⺯��
void Edge::on_Canny(int, void*)
{
	//cvtColor(g_srcImage, g_srcGrayImage, CV_BGR2GRAY);
	// ��ʹ�� 3x3�ں�������
	g_srcGrayImage.convertTo(g_cannyDetectedEdges, CV_8UC1);
	//blur(g_srcGrayImage, g_cannyDetectedEdges, Size(3, 3));

	// �������ǵ�Canny����  
	Canny(g_cannyDetectedEdges, g_cannyDetectedEdges, g_cannyLowThreshold, g_cannyLowThreshold * 3, 3);

	////�Ƚ�g_dstImage�ڵ�����Ԫ������Ϊ0  
	//g_dstImage = Scalar::all(0);

	////ʹ��Canny��������ı�Եͼg_cannyDetectedEdges��Ϊ���룬����ԭͼg_srcImage����Ŀ��ͼg_dstImage��  
	//g_srcImage.copyTo(g_dstImage, g_cannyDetectedEdges);

	//��ʾЧ��ͼ  
	//namedWindow("Canny", CV_WINDOW_NORMAL);
	imshow("canny", g_cannyDetectedEdges);
	imwrite("./canny.png", g_cannyDetectedEdges);
}

//���а���
static void ShowHelpText(Mat Image)
{
	cout << "ͼ��ĳߴ磺" << "��=" << Image.cols << "    " << "��=" << Image.rows << endl << endl;
	cout << "ͼ���ͨ������" << Image.channels() << endl;
}
//�������С�����Ӧ�±�
//template <class T>
MinMaxIndex find_Min(int *arr, int N)
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
MinMaxIndex find_Max(int *arr, int N)
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
	system("color 2f");//���ÿ���̨��Ӱ��ɫ
	//---------------------------------------------------------------

	//�ӿڿ��ƺ�����1��ʾ�ǣ�0��ʾ��
	controlFuction(10,//��ɫ��
		10,//�Ƿ���ߣ���ɫ
		0,//�Ƿ�ʵ�����,1Ϊʵ����䣬0Ϊ�������
		0,//�Ƿ�����ڱ�
		10,//�Ƿ���ʾ��ʼ�̵�
		1,//�Ƿ񵼳�����
		0,//�Ƿ���ʾ����ɫ
		0);//�Ƿ��������
	//---------------------------------------------------------------

}
double deviation(vector<float> v)
{
	double sum = std::accumulate(std::begin(v), std::end(v), 0.0);
	double mean = sum / v.size(); //��ֵ  

	double accum = 0.0;
	std::for_each(
		std::begin(v), std::end(v), [&](const double d)
	{
		accum += (d - mean)*(d - mean);
	}
	);

	double stdev;
	stdev = sqrt(accum / (v.size() - 1)); //���� deviation(ƫ��)

	return stdev;
}
bool isInsec(vector <int> &ind)//2-6
{
	vector <int> index = ind;
	int n = index.size();
	//����0��7������β���
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
	else//�������
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
	int len1, len2,len3;
	//Point2i pt;
	centVal = g_calImg.at<float>(centPt.x, centPt.y);
	low = centVal - TH1;
	high = centVal + TH1;

	//���򻮷�
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

	//һ�����������������������������߻�������
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
	//�������ڶ����������������������߱�Ե����
	else if ((n1 + n2) == 8)//n1,n2����������
	{
		if (!isTag(centPt) && n2 > 1 && n2 < 7)//&&!isTag(centPt) !isTag(centPt) &&
		{
			//�ж��Ƿ�Ϊ��������
			bool insec = false;
			if (ind1.size() < ind2.size())
			{
				insec = isInsec(ind1);
			}
			else
				insec = isInsec(ind2);

			//�ж���������ķ���

			//�������������������һ������Ϊ����
			if (0)//insec
			{
				//��n1,n2����ķ���ֵ
				//sort(sec1.begin(), sec1.end());
				//sort(sec2.begin(), sec2.end());
				std1 = deviation(sec1);
				std2 = deviation(sec2);
				len1 = maxSeqence(ind1);
				len2 = maxSeqence(ind2);
				//��ֵ�ϴ��Ϊ��������
				if (std1 < std2&&len1>len2)//sec2 Ϊ����
				{
					//�÷����������ֵ��ƽ��������
					sum = std::accumulate(sec1.begin(), sec1.end(), 0.0);
					ave = sum / sec1.size();
					int n = sec2.size();

					for (int k = 0; k < n; k++)
					{
						g_calImg.at<float>(centPt.x + a8i[ind2.at(k)], centPt.y + a8j[ind2.at(k)]) = ave;
						g_nSectionFlg.at<int>(centPt.x + a8i[ind2.at(k)], centPt.y + a8j[ind2.at(k)]) = 1;
					}
					//������ĵ�
					g_calImg.at<float>(centPt.x, centPt.y) = ave;
					g_nSectionFlg.at<int>(centPt.x, centPt.y) = 1;
				}
				else if (std1 > std2)//sec1 Ϊ����
				{
					//�÷����������ֵ��ƽ��������
					sum = std::accumulate(sec2.begin(), sec2.end(), 0.0);
					ave = sum / sec2.size();
					int n = sec1.size();
					for (int k = 0; k < n; k++)
					{
						g_calImg.at<float>(centPt.x + a8i[ind1.at(k)], centPt.y + a8j[ind1.at(k)]) = ave;
						g_nSectionFlg.at<int>(centPt.x + a8i[ind1.at(k)], centPt.y + a8j[ind1.at(k)]) = 1;
					}
					//������ĵ�
					g_calImg.at<float>(centPt.x, centPt.y) = ave;
					g_nSectionFlg.at<int>(centPt.x, centPt.y) = 1;
				}

			}
		}
		return 2;
	}
	else if ((n3 + n2) == 8)//n3,n2����������
	{
		if (!isTag(centPt) && n2 > 1 && n2 < 7)//&&!isTag(centPt) !isTag(centPt) &&
		{
			//�ж��Ƿ�Ϊ��������
			bool insec = false;
			if (ind3.size() < ind2.size())
			{
				insec = isInsec(ind3);
			}
			else
				insec = isInsec(ind2);

			//�������������������һ������Ϊ����
			if (0)//insec
			{
				//��n1,n2����ķ���ֵ
				//sort(sec1.begin(), sec1.end());
				//sort(sec2.begin(), sec2.end());
				std3 = deviation(sec3);
				std2 = deviation(sec2);
				len3 = maxSeqence(ind3);
				len2 = maxSeqence(ind2);
				//��ֵ�ϴ��Ϊ��������
				if (std3 < std2&&len3>len2)//sec2 Ϊ����
				{
					//�÷����������ֵ��ƽ��������
					sum = std::accumulate(sec3.begin(), sec3.end(), 0.0);
					ave = sum / sec3.size();
					int n = sec2.size();

					for (int k = 0; k < n; k++)
					{
						g_calImg.at<float>(centPt.x + a8i[ind2.at(k)], centPt.y + a8j[ind2.at(k)]) = ave;
						g_nSectionFlg.at<int>(centPt.x + a8i[ind2.at(k)], centPt.y + a8j[ind2.at(k)]) = 1;
					}
					//������ĵ�
					g_calImg.at<float>(centPt.x, centPt.y) = ave;
					g_nSectionFlg.at<int>(centPt.x, centPt.y) = 1;
				}
				else if (std3 > std2)//sec3 Ϊ����
				{
					//�÷����������ֵ��ƽ��������
					sum = std::accumulate(sec2.begin(), sec2.end(), 0.0);
					ave = sum / sec2.size();
					int n = sec3.size();
					for (int k = 0; k < n; k++)
					{
						g_calImg.at<float>(centPt.x + a8i[ind3.at(k)], centPt.y + a8j[ind3.at(k)]) = ave;
						g_nSectionFlg.at<int>(centPt.x + a8i[ind3.at(k)], centPt.y + a8j[ind3.at(k)]) = 1;
					}
					//������ĵ�
					g_calImg.at<float>(centPt.x, centPt.y) = ave;
					g_nSectionFlg.at<int>(centPt.x, centPt.y) = 1;
				}

			}
		}
		return 2;
	}
	else if (n1 + n3 == 8)//�������Ͷ���Ϊ���ĵ�������
	{
		return 1;
	}
	else//ȫ����ͬ��ͬ
	{

		return 1;
	}


	//����������
	//return true;

}
//Input��������������Ӧ�����±�2,3,4,7
//Output:�ҵ���������±꣬Ϊ������ֵΪ������ͬ��û��������ͬ�ķ�0;
int maxSeqence(vector <int> &ind)
{
	int n = ind.size();
	int x0, x1,x11;
	int maxLength=0;
	int max[6] = {0,0,0,0,0,0};

	for (int k = 0; k < n; k++)
	{
		x0 = ind.at(k);

		for (int i = 1; i < n - k; i++)//���ݵ�Ind����Ϊ2-6
		{
			x1 = ind.at(k+i);//
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
	return maxLength;//������������
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
//-----------------------------------��main( )������------------------------------------------------------------------------------  
//      ����������̨Ӧ�ó������ں��������ǵĳ�������￪ʼ  
//--------------------------------------------------------------------------------------------------------------------------------  
int main(int agrc, char** agrv)
{
	initMain();//��ʼ��

	//����ԭͼ  //��ȡ3_SͼƬʱ���ִ�
	g_srcImage = imread(".\\sourcePicture\\3.bmp"); //noicePicture 2018 poisson.jpg saltpepper.bmp
	//blur(g_srcImage, g_srcImage, Size(3, 3));
	//����һ��Mat���Ͳ������趨ROI����  
	//g_srcImage = g_srcImage(Rect(0, 0, 50, 15));
	//�ж��Ƿ��ȡ�ɹ�
	if (!g_srcImage.data)
	{
		cout << "��ȡͼƬsrcImageʱ����\n";
		return false;
	}

	//һ��Ĭ�ϰѲ�ɫͼ��ת��Ϊ�Ҷ�ͼ��
	//*/
	cvtColor(g_srcImage, g_srcGrayImage, CV_BGR2GRAY);
	Edge edge(g_srcGrayImage);//��ɫ��ֵ��ʼ��
	//*/
	//����ȡ��ע������������ʵ�ֲ�ɫͼ����
	//Edge edge(g_srcImage);//��ɫ��ֵ��ʼ��

	edge.Init();//�����ʼ��

	//����Canny����tracebar
	namedWindow("canny");
	//namedWindow("listk1"); namedWindow("listk1&&listk2");
	createTrackbar("����ֵ��", "canny", &g_cannyLowThreshold, 160, edge.on_Canny);
	namedWindow("��Եͼ");//, CV_WINDOW_NORMAL
	//createTrackbar("�ݶȣ�", "g_mergImg", &TH, 80, edge.edgeDection);	
	//createTrackbar("���ƶȣ�", "g_mergImg", &TH1, 40, edge.edgeDection);
	//createTrackbar("���ƶȣ�", "g_edge[1]", &g_di, 80, edge.listK);

	deNoice(gray.rows, gray.cols);
	edge.edgeDection();
	cout << "������=" << startPt << endl;
	cout << "��ɫ����=" << bluePt << endl;

	g_srcGrayImage.convertTo(g_srcGrayImage, CV_32F);
	if (xls)
	{
		edge.outXls(g_srcGrayImage, "./output/gray.xls");
		//outxlsInt(g_biGrad, "./output/bigrad.xls");
	}
	outxls(g_thresold, "./output/th.xls");
	g_srcGrad.convertTo(g_gradBmp, CV_8UC1);
	imshow("�ݶ�ͼ", g_gradBmp);
	imwrite("./output/grad.png", g_gradBmp);


	//��ѯ��ȡ������Ϣ��������Q�������˳�  
	while ((char(waitKey(1)) != 'q')) {}
	return 0;
}
void getTypeEdge()
{
	vector<Mat> nTypeEdge(3);
	nTypeEdge[0] = Mat::zeros(gray.rows, gray.cols, CV_32S);
	nTypeEdge[1] = Mat::zeros(gray.rows, gray.cols, CV_32S);
	nTypeEdge[2] = Mat::zeros(gray.rows, gray.cols, CV_32S);
	Mat display_typeEdge(gray.rows, gray.cols, CV_8UC3, Scalar::all(0));
	for (int i = 1; i < gray.rows - 1; i++)
	{
		for (int j = 1; j < gray.cols - 1; j++)
		{
			switch (g_nType.at<int>(i, j))
			{
			case 0:
				break;
			case 3:
				//blue
				nTypeEdge[0].at < int >(i, j) = 255;
				nTypeEdge[1].at < int >(i, j) = 0;
				nTypeEdge[2].at < int >(i, j) = 0;
				break;
			case 4:
				//green
				nTypeEdge[0].at < int >(i, j) = 0;
				nTypeEdge[1].at < int >(i, j) = 255;
				nTypeEdge[2].at < int >(i, j) = 0;
				break;
			case 5:
				//red
				nTypeEdge[0].at < int >(i, j) = 0;
				nTypeEdge[1].at < int >(i, j) = 0;
				nTypeEdge[2].at < int >(i, j) = 255;
				break;
			case 6:
				nTypeEdge[0].at < int >(i, j) = 255;
				nTypeEdge[1].at < int >(i, j) = 255;
				nTypeEdge[2].at < int >(i, j) = 0;
				break;
			case 7:
				//0,255,255Ϊ��ɫ
				nTypeEdge[0].at < int >(i, j) = 0;
				nTypeEdge[1].at < int >(i, j) = 255;
				nTypeEdge[2].at < int >(i, j) = 255;
				break;
			default:
				//cout << "nType error!\n";
				break;

			}
		}
	}
	nTypeEdge[0].convertTo(nTypeEdge[0], CV_8UC1);
	nTypeEdge[1].convertTo(nTypeEdge[1], CV_8UC1);
	nTypeEdge[2].convertTo(nTypeEdge[2], CV_8UC1);
	merge(nTypeEdge, display_typeEdge);
	imshow("��ͬ���͵ı�Եͼ", display_typeEdge);
	imwrite("./output/ntype.bmp", display_typeEdge);

	//display_typeEdge=Mat::zeros(gray.rows, gray.cols, CV_8UC3);
	g_nType = Mat::zeros(gray.rows, gray.cols, CV_32S);//�ٴγ�ʼ��
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
				else//�������ĵ㲻ͬ��ֵ��ӣ�7��8
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

			//�����ж�����
			centPt.x = i;
			centPt.y = j;
			nSection(centPt);
			num = 0;
			sum = 0;
		}
	}
}

void salt(cv::Mat& image){
	//unsigned int seed; /*������ʼ���������ӣ�ע����usigned int �͵�*//
	srand(time(0));
	int flgedge;
	for (int k = 0; k < 100000; k++)
	{
		int i = rand() % image.cols;
		int j = rand() % image.rows;

		flgedge = g_imgFlg.at<int>(j, i);

		//��ӱ�Ե����
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
		//	//��ӷǱ�Ե����

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


//���������򲢷�Ϊ8��1-8
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
//����ڱ�


