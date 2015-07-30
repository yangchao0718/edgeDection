// ---------------------------------- - 【程序说明】----------------------------------------------
//      程序名称:OpenCV边缘检测：新方法实现边缘特征提取并能定位噪音
//      开发所用IDE版本：Visual Studio 2013  
//      <span style="white-space:pre">  </span>开发所用OpenCV版本：    2.4.9  
//      2014年11月19日 Create by 杨超  
//    
//----------------------------------------------------------------------------------------------  

//-----------------------------------【头文件包含部分】---------------------------------------  
//      描述：包含程序所依赖的头文件  
//----------------------------------------------------------------------------------------------  
#include "edge.h"  
#include"stdlib.h"
Edge::Edge(Mat in)
{
	gray = in;
	g_listk = in;
	namedWindow("【灰度图】");
	imshow("【灰度图】", in);

}
Edge::~Edge()
{

}
//输入：一个点centerPt
//输出：判断此点是否为边缘点，如果是则输出相邻的两个边缘点A,B,与中心点不同的点AC、BC
bool findStartPt(Point2i centerPt, Point2i &A, Point2i &B, Point2i &AC, Point2i &BC, int &nType, int &direction)
{
	bool  flagA, flagB;//标记是否找到A/B点
	flagA = flagB = false;
	Point2i stPt, E;
	int i, j;
	int bdA, bdB, bdTmp;
	int dx, dy;
	i = centerPt.x;
	j = centerPt.y;
	//与中心边缘点相邻的两个边缘点A，B，
	int indext;

	if (g_imgFlg.at<int>(centerPt.x, centerPt.y))
		return false;

	float CV = g_calImg.at<float>(i, j);



	//下面函数用来找到中心点的相同的起止点A、B及不同的起止点AC、BC
	//如果第一个点和中心点相同，则找连续两个和中点不相同的点
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


	//计算Direction,若如下叉积小于0，则从半平面的顺时针开始查找
	direction = (A.x - centerPt.x)*(AC.y - centerPt.y) - (A.y - centerPt.y)*(AC.x - centerPt.x);
	dx = A.x - centerPt.x;
	dy = A.y - centerPt.y;

	//得到了A、B后，计算出边缘类型
	//把A、B点转为相对下标来求得连续长度作为边缘类型


	if (g_imgFlg.at<int>(A.x, A.y) || g_imgFlg.at<int>(B.x, B.y) || g_imgFlg.at<int>(AC.x, AC.y) || g_imgFlg.at<int>(BC.x, BC.y))
		return false;

	if (flagA&&flagB)
	{
		bdA = bordDirection(A.x - centerPt.x, A.y - centerPt.y);
		bdB = bordDirection(B.x - centerPt.x, B.y - centerPt.y);
		nType = lengthAB(bdA, bdB, direction);


		if (!g_imgFlg.at<int>(centerPt.x, centerPt.y))
		{
			//中点标记1,绿色
			g_edge[0].at<float>(centerPt.x, centerPt.y) = 0;
			g_edge[1].at<float>(centerPt.x, centerPt.y) = 255;
			g_edge[2].at<float>(centerPt.x, centerPt.y) = 0;
			g_imgFlg.at<int>(centerPt.x, centerPt.y) = 1;

			//与中心点不相同点比中心点小的标记为红色
			if (g_calImg.at<float>(centerPt.x, centerPt.y)>g_calImg.at<float>(AC.x,AC.y)){
				g_edge[0].at<float>(A.x, A.y) = 0;
				g_edge[1].at<float>(A.x, A.y) = 0;
				g_edge[2].at<float>(A.x, A.y) = 255;
				g_imgFlg.at<int>(A.x, A.y) = 2;

				g_edge[0].at<float>(B.x, B.y) = 0;
				g_edge[1].at<float>(B.x, B.y) = 0;
				g_edge[2].at<float>(B.x, B.y) = 255;
				g_imgFlg.at<int>(B.x, B.y) = 2;

				g_edge[0].at<float>(AC.x, AC.y) = 255;
				g_edge[1].at<float>(AC.x, AC.y) = 0;
				g_edge[2].at<float>(AC.x, AC.y) = 0;
				g_imgFlg.at<int>(AC.x, AC.y) = 3;

				g_edge[0].at<float>(BC.x, BC.y) = 255;
				g_edge[1].at<float>(BC.x, BC.y) = 0;
				g_edge[2].at<float>(BC.x, BC.y) = 0;
				g_imgFlg.at<int>(BC.x, BC.y) = 3;
			}
			else{//与中心点不相同点且比中心点大的标记为蓝色
				
				//*
				g_edge[0].at<float>(A.x, A.y) = 255;
				g_edge[1].at<float>(A.x, A.y) = 0;
				g_edge[2].at<float>(A.x, A.y) = 0;
				g_imgFlg.at<int>(A.x, A.y) = 3;

				g_edge[0].at<float>(B.x, B.y) = 255;
				g_edge[1].at<float>(B.x, B.y) = 0;
				g_edge[2].at<float>(B.x, B.y) = 0;
				g_imgFlg.at<int>(B.x, B.y) = 3;

				g_edge[0].at<float>(AC.x, AC.y) = 0;
				g_edge[1].at<float>(AC.x, AC.y) = 0;
				g_edge[2].at<float>(AC.x, AC.y) = 255;
				g_imgFlg.at<int>(AC.x, AC.y) = 2;

				g_edge[0].at<float>(BC.x, BC.y) = 0;
				g_edge[1].at<float>(BC.x, BC.y) = 0;
				g_edge[2].at<float>(BC.x, BC.y) = 255;
				g_imgFlg.at<int>(BC.x, BC.y) = 2;
				//*/
			}
		}

		return true;//如果找到A、B两个边缘点则开始调用延伸追踪函数TraceEdgeTwoBoundary
	}
	else
		return false;


}
 
//Trace two sides of the boundary
//direction; 0  denotes  counterclockwise
//输入：点A、B、C ,direction    A、B为相邻边缘点，C为与AB不同类的点,direction为方向
//输出：点D、E          D为B的半平面内最后一个和B点相近的点，E为半平面内第一个与B不相似的点
bool TraceEdgeTwoBoundary(Point2i A, Point2i B, Point2i C, Point2i &D, Point2i &E, int  direction)
{
	int dx, dy;
	dx = B.x - A.x;
	dy = B.y - A.y;
	g_tracenum++;
	Point2i Pts[5];
	Point2i missedPt;
	bool flagD = false, flagE = false;
	if (direction >0){
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
	float  BV, CV, tempV;
	BV = g_calImg.at<float>(B.x, B.y);
	CV = g_calImg.at<float>(C.x, C.y);

	for (i = 0; i <= 4; i++)
	{
		if (!outOfrange(Pts[i].x, Pts[i].y)){
			tempV = g_calImg.at<float>(Pts[i].x, Pts[i].y);
			if (abs(tempV - BV) > abs(tempV - CV))
			{
				int ii = i - 1;
				if (i == 0)
				{
					return false;
					/*D.x = Pts[i + 1].x;
					D.y = Pts[i + 1].y;
					E.x = Pts[i+2].x;
					E.y = Pts[i+2].y;*/
				}
				else
				{
					D.x = Pts[ii].x;
					D.y = Pts[ii].y;
					E.x = Pts[i].x;
					E.y = Pts[i].y;
				}

				

				flagD = true;
				flagE = true;
				break;
			}
		}
	}

	if (i == 5)
	{
		D.x = Pts[4].x;
		D.y = Pts[4].y;
		E.x = C.x;
		E.y = C.y;
		flagD = true;
		flagE = true;
	}




	//Pts[0-4] set the value of color
	//*
	int indext, kflagMissed=0;

	if (flagD&&flagE)
	{
		if (outOfrange(D.x, D.y) || g_imgFlg.at<int>(D.x, D.y))
			return false;

		int  directC, directE, deltat;
		directC = getIangle(B, C);
		directE = getIangle(B, E);
		
		
		if (direction<0){
			deltat = 1;  
			if (directE<directC)
				directE += 8;
		}
		else{
			deltat = -1;
			if (directC<directE)
				directC += 8;
		}
		for (i = directC + deltat; i != directE - 1; i += deltat){
			if (i % 2 == 0){
				kflagMissed = 1;
				missedPt.x = B.x + a8i[i];
				missedPt.y = B.y + a8j[i];
				break;
			}
		}
        
		if (g_calImg.at<float>(A.x, A.y) > g_calImg.at<float>(C.x, C.y))
		{
			g_edge[0].at<float>(D.x, D.y) = 0;
			g_edge[1].at<float>(D.x, D.y) = 0;
			g_edge[2].at<float>(D.x, D.y) = 255;
			g_imgFlg.at<int>(D.x, D.y) = 2;

			g_edge[0].at<float>(E.x, E.y) = 255;
			g_edge[1].at<float>(E.x, E.y) = 0;
			g_edge[2].at<float>(E.x, E.y) = 0;
			g_imgFlg.at<int>(E.x, E.y) = 3;
			if (kflagMissed){
				g_edge[0].at<float>(missedPt.x, missedPt.y) = 255;
				g_edge[1].at<float>(missedPt.x, missedPt.y) = 0;
				g_edge[2].at<float>(missedPt.x, missedPt.y) = 0;
				g_imgFlg.at<int>(missedPt.x, missedPt.y) = 3;
			}
		}
		else{
			g_edge[0].at<float>(D.x, D.y) = 255;
			g_edge[1].at<float>(D.x, D.y) = 0;
			g_edge[2].at<float>(D.x, D.y) = 0;
			g_imgFlg.at<int>(D.x, D.y) = 3;

			g_edge[0].at<float>(E.x, E.y) = 0;
			g_edge[1].at<float>(E.x, E.y) = 0;
			g_edge[2].at<float>(E.x, E.y) = 255;
			g_imgFlg.at<int>(E.x, E.y) = 2;
			if (kflagMissed){
				g_edge[0].at<float>(missedPt.x, missedPt.y) = 0;
				g_edge[1].at<float>(missedPt.x, missedPt.y) = 0;
				g_edge[2].at<float>(missedPt.x, missedPt.y) = 255;
				g_imgFlg.at<int>(missedPt.x, missedPt.y) = 2;
			}
		}
		return true;
	}
	

}
//输入：A、B、C、D、E，direction--[B为待处理的中心点，D、E为下一个B、C]
//输出：动态阈值th
int getDth(Point2i A, Point2i B, Point2i C, Point2i D, Point2i E,int direction)
{
	int dt;
	vector<Point2i> pts;

	if (direction>0)
	{
	}
	else
	{

	}


	return dt;
}

void Edge::edgeDection()
{

	int dx, dy, direction;//边缘的方向相对坐标
	int nType;
	Point2i A, B, AC, BC, D, E, centPt, stPt;
	g_medImg.convertTo(g_medImg, CV_32F);
	int num = 0;
	bool keyi = true;
	int i, j;

//	for (j = 0; j < g_listk.cols; j++)
//		cout << "i=117" << "j=" << j << ";flag=" << g_imgFlg.at<int>(117, j) << endl;

	//找到前两个边缘点作为起始延伸点
	for (i = 1; i < g_calImg.rows - 1; i++)
	{
//		if (i==117)
//			outxlsInt(g_imgFlg, "./output/imgflagTest.xls");

		for (j = 1; j < g_calImg.cols - 1; j++)
		{

			//起点位置的寻找限制条件，起点及其8邻阈不得有已标记点

			int flagij = g_imgFlg.at<int>(i, j);

			//cout << "i=" << i << "j=" << j << ";flag=" << flagij << endl;

			if (!outOfrange(i, j) && flagij == 0)//TH可修改为动态阈值
			{
				centPt.x = i;
				centPt.y = j;

				int colorx, colory, colorz, colorxA, coloryA, colorzA, flagxyA, flagxy;

				//1根椐findStartPt得到起始点边缘类型
				if (findStartPt(centPt, A, B, AC, BC, nType, direction))
				{
					/*if (g_calImg.at<float>(A.x, A.y) > g_calImg.at<float>(AC.x,AC.y))
					{
					colorxA = 0;
					coloryA = 0;
					colorzA = 255;
					flagxyA = 2;

					colorx = 255;
					colory = 0;
					colorz = 0;

					flagxy = 3;
					}
					else
					{
					colorxA = 255;
					coloryA = 0;
					colorzA = 0;
					flagxyA = 3;

					colorx = 0;
					colory = 0;
					colorz = 255;
					flagxy = 2;
					}*/
					//2边缘类型满足3-6的定义为延伸点
					if (nType > 2 && nType < 7)
//					if (1)
					{

						//3分别向两个方向进行延伸			
						stPt = centPt;
						while (keyi){
							keyi = TraceEdgeTwoBoundary(stPt, A, AC, D, E, direction);
							stPt = A;
							A = D;
							AC = E;
						}
						//*
						stPt = centPt;
						keyi = true;

						while (keyi){				
							keyi = TraceEdgeTwoBoundary(stPt, B, BC, D, E, -direction);
							stPt = B;
							B = D;
							BC = E;
						}
						//*/

					}
				}


			}

		}

	}

	if (xls)
	{
		outxls(g_srcGrad, ".\\output\\grad.xls");
		outxls(g_angle, "./output/angle.xls");
		outxlsInt(g_imgFlg, "./output/imgflag.xls");
		//outxlsInt(g_dedgeX, "./output/g_dedgeX.xls");
		//outxlsInt(g_dedgeY, "./output/g_dedgeY.xls");

	}


	//outxlsInt(g_dstBiGrad, "./output/dstBigrad.xls");

	//输出混合图像【边缘+原图】
	getMix();

	//得到混合的彩色边缘点云图
	g_edge[0].convertTo(g_edge[0], CV_8UC1);
	g_edge[1].convertTo(g_edge[1], CV_8UC1);
	g_edge[2].convertTo(g_edge[2], CV_8UC1);
	merge(g_edge, g_mergImg);
	imshow("g_mergImg", g_mergImg); //imshow("listk1", listK1); imshow("listk1&&listk2", mergImg);
	imwrite("./output/median.bmp", g_mergImg);

	merge(g_srcImageChannels, g_srcImageBGR);
	imshow("g_srcImageBGR", g_srcImageBGR);
	imwrite("./output/tagEdge.bmp", g_srcImageBGR);
	//(g_edge[1].clone()).convertTo(g_outEdge, CV_32F);//最终边缘图

	//得到不同星型的混合边缘图
	//	getTypeEdge();

	//以后的循环初始化
	nextInitial();



}

void Edge::edgeDection1()
{
	n1 = 0; n2 = 0;
	int bdA, bd, bdB, bdC;//边缘方向
	float ndA, nd, ndB;
	int x, y;
	int dx, dy;//边缘的方向相对坐标

	Point2i A, B, C, D, E;
	float sum3 = 0;
	//根据前两点得到第二个点的法向量和边缘方向,接下来去判断延伸点的方向，根据这个方向来找起止点
	Point2f X, XA, XB;



	g_medImg.convertTo(g_medImg, CV_32F);

	//找到前两个边缘点作为起始延伸点
	for (int i = 1; i < g_calImg.rows - 1; i++)
	{
		for (int j = 1; j < g_calImg.cols - 1; j++)
		{

			//起点位置的寻找限制条件，起点及其8邻阈不得有已标记点

			if (!outOfrange(i, j) && g_imgFlg.at<int>(i, j) == 0 && startPoint(i, j))//TH可修改为动态阈值
			{
				mmik = findSeqNum(i, j);//求满足边缘相同的元素的个数和起始下标
				seq_num = mmik.m_num;

				if (seq_num > 3 && seq_num < 7)//和中点连续相同个数为2时出现调头不考虑
				{
					/*cnt++;
					if (cnt>=1&&cnt<=4)
					{
					cout << endl;
					cout << "i=1" << i << " " << "j=" << j << endl;
					}*/
					if (110 == i && 216 == j)
					{
						cout << endl;
					}
					//求得中心点法向量
					g_angle.at<float>(i, j) = getAngle(mmik.m_index, seq_num);
					nd = getAngle(mmik.m_index, seq_num);


					//与中心边缘点相邻的两个边缘点A，B，
					//A点标记为绿色
					A.x = i + a8i[mmik.m_index];
					A.y = j + a8j[mmik.m_index];
					/*		g_edge[0].at<float>(A.x, A.y) = 255;
					g_edge[1].at<float>(A.x, A.y) = 0;
					g_edge[2].at<float>(A.x, A.y) = 0;*/
					//g_imgFlg.at<int>(A.x, A.y) = 1;
					//B点标记为黄色
					B.x = i + a8i[(mmik.m_index + seq_num - 1) % 8];//方向对应的坐标
					B.y = j + a8j[(mmik.m_index + seq_num - 1) % 8];
					/*g_edge[0].at<float>(B.x, B.y) = 155;
					g_edge[1].at<float>(B.x, B.y) = 100;
					g_edge[2].at<float>(B.x, B.y) = 100;*/
					//g_imgFlg.at<int>(B.x, B.y) = 1;
					//设置中心点和中心点的起点颜色
					calType(i, j);

					g_edge[0].at<float>(i, j) = 0;
					g_edge[1].at<float>(i, j) = 255;
					g_edge[2].at<float>(i, j) = 0;

					//g_imgFlg.at<int>(i, j) = 1;
					//fillAdjacent(i, j);

					//AB点在中心点的相对方向定义为中心点的边缘方向
					bdA = mmik.m_index;
					bdB = (mmik.m_index + seq_num - 1) % 8;//方向	
					//bdC = getMedLine(bdA, bdB);

					//drtToVector可以通过函数模板来实现
					//bd为中心点边缘方向，nd为边缘点的法向量方向
					float reslutA, reslutB;
					X = drtToVector(nd);
					XA = drtToVector(bdA);
					XB = drtToVector(bdB);

					reslutA = X.cross(XA);
					reslutB = X.cross(XB);


					//TraceEdgeTwoBoundary();
					strEdge(i, j, bdA, reslutA);
					strEdge(i, j, bdB, reslutB);

				}

			}

		}

	}

	if (xls)
	{
		outxls(g_srcGrad, ".\\output\\grad.xls");
		outxls(g_angle, "./output/angle.xls");
		outxlsInt(g_imgFlg, "./output/imgflag.xls");
		//outxlsInt(g_dedgeX, "./output/g_dedgeX.xls");
		//outxlsInt(g_dedgeY, "./output/g_dedgeY.xls");

	}


	//outxlsInt(g_dstBiGrad, "./output/dstBigrad.xls");

	//输出混合图像【边缘+原图】
	getMix();

	//得到混合的彩色边缘点云图
	g_edge[0].convertTo(g_edge[0], CV_8UC1);
	g_edge[1].convertTo(g_edge[1], CV_8UC1);
	g_edge[2].convertTo(g_edge[2], CV_8UC1);
	merge(g_edge, g_mergImg);
	imshow("g_mergImg", g_mergImg); //imshow("listk1", listK1); imshow("listk1&&listk2", mergImg);
	imwrite("./output/median.bmp", g_mergImg);

	merge(g_srcImageChannels, g_srcImageBGR);
	imshow("g_srcImageBGR", g_srcImageBGR);
	imwrite("./output/tagEdge.bmp", g_srcImageBGR);
	//(g_edge[1].clone()).convertTo(g_outEdge, CV_32F);//最终边缘图

	//得到不同星型的混合边缘图
	//	getTypeEdge();

	//以后的循环初始化
	nextInitial();



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
		else
			return true;
	}
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
	//	Mat imBGR(g_listk.rows, g_listk.cols, CV_8UC3, Scalar::all(0));
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
	g_thresold = Mat::zeros(g_listk.rows, g_listk.cols, CV_32F);
	g_angle = Mat::ones(g_listk.rows, g_listk.cols, CV_32F)*(-1);
	g_intAngle = Mat::zeros(g_listk.rows, g_listk.cols, CV_32S);

	g_dstImage = Mat::zeros(g_listk.rows, g_listk.cols, CV_32F);
	//g_edge[1] = Mat::zeros(g_listk.rows, g_listk.cols, CV_32F);//初始化边缘图
	g_edge[0] = Mat::zeros(g_listk.rows, g_listk.cols, CV_32F);
	g_edge[1] = Mat::zeros(g_listk.rows, g_listk.cols, CV_32F);
	g_edge[2] = Mat::zeros(g_listk.rows, g_listk.cols, CV_32F);
	g_mergImg = Mat::zeros(g_listk.rows, g_listk.cols, CV_8UC3);

	g_srcImageBGR = Mat::zeros(g_listk.rows, g_listk.cols, CV_8UC3);

	//Mat g_imgFlg(gray.rows, gray.cols, CV_32S,Scalar::all(0));
	g_imgFlg = Mat::zeros(g_listk.rows, g_listk.cols, CV_32S);//图像特征标记表，32位有符号整数::::::::::::::::::::::::::::::::::为何用8位无符号整形或有符号整形就会出错
}


//判断是否下标越界
bool outOfrange(int i, int j)
{
	if (i > 0 && i < g_listk.rows - 1 && j>0 && j < g_listk.cols - 1)
		return false;
	else
	{
		//cout << "超出边界范围！\n";
		return true;
	}

}

//计算边缘点类型，星形及增减型
void calType(int i, int j)
{
	//1---------------判断增减型------------------------
	Point2i inv_angle = { 0 };
	int sum3 = 0;
	inv_angle = normalToXY(g_angle.at<float>(i, j));

	//根据此边缘点两侧的平反法向量延长的3个点的均值与此边缘点作差来判断是123中的哪种：较以前改进了噪音及局部像素不稳定造成的景程
	if (!outOfrange(i - 3 * inv_angle.x, j - 3 * inv_angle.y))
	{
		for (int k = 0; k < 3; k++)
		{

			sum3 = sum3 + g_calImg.at<float>(i - k*inv_angle.x, j - k*inv_angle.y);// -号表示反向

		}
		sum3 = sum3 / 3;//求得均值
		if (sum3 - g_calImg.at<float>(i, j)>0)
		{
			if (real_edge)
			{
				//一、增边,用黄色表示
				g_edge[0].at<float>(i, j) = 0;
				g_edge[1].at<float>(i, j) = 255;
				g_edge[2].at<float>(i, j) = 255;

				//fillAdjacent(i,j);
			}
			else
			{
				g_edge[0].at<float>(i, j) = 0;
				g_edge[1].at<float>(i, j) = 0;
				g_edge[2].at<float>(i, j) = 0;
			}
			g_imgFlg.at<int>(i, j) = 1;

		}
		else if (sum3 - g_calImg.at<float>(i, j) < 0)
		{
			if (virtual_edge)
			{
				//二、减边，用红色表示
				g_edge[0].at<float>(i, j) = 255;
				g_edge[1].at<float>(i, j) = 0;
				g_edge[2].at<float>(i, j) = 255;
				g_imgFlg.at<int>(i, j) = 2;
				//fillAdjacent(i, j);
			}
			else
			{
				g_edge[0].at<float>(i, j) = 0;
				g_edge[1].at<float>(i, j) = 0;
				g_edge[2].at<float>(i, j) = 0;
			}
			g_imgFlg.at<int>(i, j) = 2;
		}
		else
		{
			if (real_edge || virtual_edge)
			{
				//三、未定边，用汪蓝色表示 
				g_edge[0].at<float>(i, j) = 0;
				g_edge[1].at<float>(i, j) = 0;
				g_edge[2].at<float>(i, j) = 255;
				//fillAdjacent(i, j);
			}
			else
			{
				g_edge[0].at<float>(i, j) = 0;
				g_edge[1].at<float>(i, j) = 0;
				g_edge[2].at<float>(i, j) = 0;
			}
			g_imgFlg.at<int>(i, j) = 3;

		}

		sum3 = 0;//再次初始化为0

	}

	//2---------------判断星型------------------------
	//g_nType.at<int>(i, j) = len;//保存星形;
}
bool semiPlanePt(int i, int j)
{
	return true;
}

//边缘生长函数。
void strEdge(int i, int j, int bd, int clk)
{

	//根据前两点得到第二个点的法向量和边缘方向,接下来去判断延伸点的方向，根据这个方向来找起止点
	int bd1 = -1;
	int bd_op = -1;
	int len = -1;
	Point2i inv_angle;
	float sum3 = 0;
	int semiP = 0;
	Point2i center = { 0 };
	center.x = i + a8i[bd];
	center.y = j + a8j[bd];
	if (!outOfrange(center.x, center.y))
	{
		//求center的法向量：1求传递进来的点相对于此时中心点的位置
		if (bd >= 0 && bd <= 3)
		{
			bd_op = bd + 4;
		}
		else
			bd_op = bd - 4;

		//在center点的半平面内开始按顺时针方向找下一个点,还需要进行已有边缘点检测
		if (clk <0)//<0表示半平面从+2、-2方向,顺时针
		{//求center的法向量：2求传递进来的点相对于此时中心点的位置

			//1越过半平面判断是否相似，相似则不为边缘点

			if (abs(g_calImg.at<float>(center.x, center.y) - g_calImg.at<float>(center.x + a8i[((bd + a5[4] - 1) + 8) % 8], center.y + a8j[((bd + a5[4] - 1) + 8) % 8])) > TH1)
			{
				//2判断半平面同是否存在了边缘点，是则退出
				for (int semi = 0; semi < 5; semi++)
				{
					/*	if (g_imgFlg.at<int>(center.x + a8i[((bd + a5[semi]) + 8) % 8], center.y + a8j[((bd + a5[semi]) + 8) % 8]))
					semiP++;*/
				}

				if (semiP < 2)
				{
					for (int ln = 0; ln < 5; ln++)
					{

						if (abs(g_calImg.at<float>(center.x, center.y) - g_calImg.at<float>(center.x + a8i[((bd + a5[ln]) + 8) % 8], center.y + a8j[((bd + a5[ln]) + 8) % 8])) <= TH1)
						{
							bd1 = ((bd + a5[ln]) + 8) % 8;
							/*center.x = center.x + a8i[bd1];
							center.y = center.y + a8j[bd1];*/
							//len = lengthAB(bd_op, bd1, true);
							g_angle.at<float>(center.x, center.y) = getMedLine(bd_op, bd1, 1);
						}

					}
				}

			}


		}
		else
		{//求center的法向量：2求传递进来的点相对于此时中心点的位置

			if (abs(g_calImg.at<float>(center.x, center.y) - g_calImg.at<float>(center.x + a8i[((bd + a5[0] + 1) + 8) % 8], center.y + a8j[((bd + a5[0] + 1) + 8) % 8])) > TH1)
			{
				//2判断半平面同是否存在了边缘点，是则退出
				for (int semi = 4; semi >= 0; semi--)
				{
					/*if (g_imgFlg.at<int>(center.x + a8i[((bd + a5[semi]) + 8) % 8], center.y + a8j[((bd + a5[semi]) + 8) % 8]))
					semiP++;*/
				}

				if (semiP < 2)
				{
					for (int ln = 4; ln >= 0; ln--)
					{
						if (abs(g_calImg.at<float>(center.x, center.y) - g_calImg.at<float>(center.x + a8i[((bd + a5[ln]) + 8) % 8], center.y + a8j[((bd + a5[ln]) + 8) % 8])) <= TH1)
						{
							bd1 = ((bd + a5[ln]) + 8) % 8;
							/*center.x = center.x + a8i[bd1];
							center.y = center.y + a8j[bd1];*/
							// len = lengthAB(bd_op, bd1, false);
							g_angle.at<float>(center.x, center.y) = getMedLine(bd_op, bd1, -1);
						}

					}
				}
			}


		}

		if (g_imgFlg.at<int>(center.x, center.y) == 0 && !outOfrange(center.x, center.y) && bd1 >= 0)//&& len> 2&&len<7   
		{

			//判断此点边缘类型1，2
			//cout << "length=" << len << endl;
			//求得法向量


			//inv_angle = normalToXY(g_angle.at<float>(center.x, center.y));


			calType(center.x, center.y);

			//g_edge[0].at<float>(center.x, center.y) = 0;
			//g_edge[1].at<float>(center.x, center.y) = 255;
			//g_edge[2].at<float>(center.x, center.y) = 255;

			//g_imgFlg.at<int>(center.x, center.y) = 2;

			////沿方向填充一段
			////fillPixel(i, j, g_angle.at<float>(i, j));
			//点亮此像素点
			//g_edge[0].at<float>(center.x, center.y) = 0;
			//g_edge[1].at<float>(center.x, center.y) = 0;
			//g_edge[2].at<float>(center.x, center.y) = 255;
			//标记此点
			//g_imgFlg.at<int>(center.x, center.y) = 5;
			//继续寻找下一点
			strEdge(center.x, center.y, bd1, clk);
		}

		//三、判断是否为边缘，如果是边缘得到边缘方向和法向并继续延伸，否则跳出延伸点，重新开始。

	}



}
int  lengthAB(int A, int B, int direction)
{
	if (direction < 0)
	{
		if ((A + 2) % 8 == B)
		{
			return 3;
		}
		if ((A + 3) % 8 == B)
		{
			return 4;
		}
		if ((A + 4) % 8 == B)
		{
			return 5;
		}
		if ((A + 5) % 8 == B)
		{
			return 6;
		}
	}
	else//逆时针
	{
		if ((A + 6) % 8 == B)
		{
			return 3;
		}
		if ((A + 5) % 8 == B)
		{
			return 4;
		}
		if ((A + 4) % 8 == B)
		{
			return 5;
		}
		if ((A + 3) % 8 == B)
		{
			return 6;
		}
	}

}
//判断两个连续边缘点的夹角是否连续
bool isSeqAngle(float nd1, float nd2)
{
	//cout << "nd1=" << nd1 << endl;
	//cout << "nd2=" << nd2 << endl;
	Point2f n1 = drtToVector(nd1);
	Point2f n2 = drtToVector(nd2);
	float cosA;
	cosA = (int)100 * (n1.x*n2.x + n1.y*n2.y) / (int)(100 * sqrt(n1.x*n1.x + n1.y*n1.y)*sqrt(n2.x*n2.x + n2.y*n2.y));
	//cout << "COSA=" << cosA << endl;
	if (cosA >= 0 && cosA <= 1)
	{
		return true;
	}
	else
		return false;

	return true;

}
//float cosAngle(float bd, float nd)
//{
//	PT n1 = drtToVector(bd);
//	PT n2 = drtToVector(nd);
//	float cosA;
//	cosA = (int)100 * (n1.x*n2.x + n1.y*n2.y) / (int)(100 * sqrt(n1.x*n1.x + n1.y*n1.y)*sqrt(n2.x*n2.x + n2.y*n2.y));
//	return cosA;
//}
//把8邻域位置转变为坐标方向,包含插值方向

Point2f drtToVector(float nd)
{
	//cout << "INDD=" << nd << endl;
	Point2f pt = { 0 };
	int IND = (int)(10 * nd);
	if (IND == 0)
	{
		//IND = 80;
	}
	//cout << "INDD=" << IND << endl;
	switch (IND)
	{
	case 0:
		pt.x = 1;
		pt.y = 0;
		break;
	case 5:
		pt.x = 1; pt.y = 0.5;
		break;
	case 10:
		pt.x = 1;
		pt.y = 1;
		break;
	case 15:
		pt.x = 0.5; pt.y = 1;
		break;
	case 20:
		pt.x = 0; pt.y = 1;
		break;
	case 25:
		pt.x = -0.5; pt.y = 1;
		break;
	case 30:
		pt.x = -1; pt.y = 1;
		break;
	case 35:
		pt.x = -1; pt.y = 0.5;
		break;
	case 40:
		pt.x = -1; pt.y = 0;
		break;
	case 45:
		pt.x = -1; pt.y = -0.5;
		break;
	case 50:
		pt.x = -1; pt.y = -1;
		break;
	case 55:
		pt.x = -0.5; pt.y = -1;
		break;
	case 60:
		pt.x = 0; pt.y = -1;
		break;
	case 65:
		pt.x = 0.5; pt.y = -1;
		break;
	case 70:
		pt.x = 1; pt.y = -1;
		break;
	case 75:
		pt.x = 1; pt.y = -0.5;
		break;
	default:
		cout << "角度转换失败！\n";
	}
	return pt;
}
Point2f drtToVector(int bd)
{
	Point2f pt = { 0 };
	switch (bd)
	{

	case 1:
		pt.x = 1;
		pt.y = 1;
		break;
	case 2:
		pt.x = 0; pt.y = 1;
		break;
	case 3:
		pt.x = -1; pt.y = 1;
		break;
	case 4:
		pt.x = -1; pt.y = 0;
		break;
	case 5:
		pt.x = -1; pt.y = -1;
		break;
	case 6:
		pt.x = 0; pt.y = -1;
		break;
	case 7:
		pt.x = 1; pt.y = -1;
		break;
	case 0:
		pt.x = 1;
		pt.y = 0;
		break;
	default:
		cout << "边缘方向转换失败！\n";
	}
	return pt;
}

//求取边缘走向，1-8共8个方向
int bordDirection(int i, int j)
{
	int bd;
	if (0 == i)
	{
		switch (j)
		{
		case 1:
			bd = 0;
			break;
		case -1:
			bd = 4;
			break;
		default:
			cout << "error\n" << endl;
			break;
		}
	}
	else if (1 == i)
	{
		switch (j)
		{
		case -1:
			bd = 5;
			break;
		case  0:
			bd = 6;
			break;
		case 1:
			bd = 7;
			break;
		default:
			cout << "error\n" << endl;
			break;
		}
	}
	else if (-1 == i)
	{
		switch (j)
		{
		case -1:
			bd = 3;
			break;
		case  0:
			bd = 2;
			break;
		case 1:
			bd = 1;
			break;
		default:
			cout << "error\n" << endl;
			break;
		}
	}

	return bd;
}

//边缘初始化
void Edge::Init()
{
	g_tracenum = 0;
	g_thresold = Mat::zeros(g_listk.rows, g_listk.cols, CV_32F);
	g_angle = Mat::ones(g_listk.rows, g_listk.cols, CV_32F)*(-1);//*11可以统一改变初始值
	g_intAngle = Mat::zeros(g_listk.rows, g_listk.cols, CV_32S);

	g_dstImage = Mat::zeros(g_listk.rows, g_listk.cols, CV_32F);
	//g_edge[1] = Mat::zeros(g_listk.rows, g_listk.cols, CV_32F);//初始化边缘图
	g_edge[0] = Mat::zeros(g_listk.rows, g_listk.cols, CV_32F);
	g_edge[1] = Mat::zeros(g_listk.rows, g_listk.cols, CV_32F);
	g_edge[2] = Mat::zeros(g_listk.rows, g_listk.cols, CV_32F);
	g_mergImg = Mat::zeros(g_listk.rows, g_listk.cols, CV_8UC3);

	g_srcImageBGR = Mat::zeros(g_listk.rows, g_listk.cols, CV_8UC3);

	//Mat g_imgFlg(gray.rows, gray.cols, CV_32S,Scalar::all(0));
	g_imgFlg = Mat::zeros(g_listk.rows, g_listk.cols, CV_32S);//图像特征标记表，32位有符号整数::::::::::::::::::::::::::::::::::为何用8位无符号整形或有符号整形就会出错

	

	g_nType = Mat::zeros(g_listk.rows, g_listk.cols, CV_32S);//

	g_biGrad = Mat::zeros(g_listk.rows, g_listk.cols, CV_32F);
	g_dstBiGrad = Mat::zeros(g_listk.rows, g_listk.cols, CV_32F);

	//【中值滤波】
	medianBlur(gray, g_medImg, 3);

	g_listk.copyTo(g_calImg);
	g_calImg.convertTo(g_calImg, CV_32F);//转换类型，便于加减运算

	//namedWindow("【中值滤波】");
	//imshow("【中值滤波】", g_medImg);
	float tempG[8] = { 0 };
	float maxG = 0, minG = 0;
	gray.convertTo(gray, CV_32F);
	g_srcGrad = Mat::zeros(gray.rows, gray.cols, CV_32F);

	//计算图像梯度，第一行最后一行第一列最后一列先不考虑梯度，设定为0
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
	//梯度图5*5>8进行个数统计
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
//外部导出函数，用于没有边缘对象创建时数据的导出
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
//Canny检测函数
void Edge::on_Canny(int, void*)
{
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
	imshow("Canny检测", g_cannyDetectedEdges);
}

//直方图函数
void histGram(Mat hist)
{
	//1直方图属性
	int histSize = 255;
	float range[] = { 0, 255 };
	const float* histRange = { range };

	//2计算直方图
	bool uniform = true, accumulate = false;
	Mat g_hist;
	calcHist(&hist, 1, 0, Mat(), g_hist, 1, &histSize, &histRange, uniform, accumulate);

	//--------------------了解Hist的内部结构？？？？？？？？？？？？？？？？？？？？？？？？
	/*namedWindow("g_hist",CV_WINDOW_NORMAL);
	imshow("g_hist", g_hist);*/

	//3创建画布
	int hist_w = 400;
	int hist_h = 400;
	int bin_w = cvRound((double)hist_w / histSize);
	Mat histImage(hist_w, hist_h, CV_8UC3, Scalar::all(0));

	//4归一化直方图到定义的窗口
	normalize(g_hist, g_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat());

	//5在直方图上画布上画出直方图
	for (int i = 1; i < histSize; i++)
	{
		line(histImage, Point(bin_w*(i - 1), hist_h - cvRound(g_hist.at<float>(i - 1))),
			Point(bin_w*(i), hist_h - cvRound(g_hist.at<float>(i))), Scalar(0, 0, 250), 2, 8, 0);
	}
	//6显示直方图
	namedWindow("calcHist");
	imshow("calcHist", histImage);
}
//运行帮助
static void ShowHelpText(Mat Image)
{
	cout << "图像的尺寸：" << "宽=" << Image.cols << "    " << "高=" << Image.rows << endl << endl;
	cout << "图像的通道数：" << g_srcImage.channels();
}
//求最大最小及其对应下标
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

//输入：中心点的起止点下标，时针方向
//输出：中心点的法向量
float getMedLine(int bdA, int bdB, int direction)
{
	float drt;
	if (direction > 0)//顺时针
	{
		if (bdA < bdB)
		{
			drt = ((bdA + bdB + 8) / 2) % 8;

		}
		else
		{
			drt = (bdA + bdB) / 2;
		}
	}
	else//逆时针
	{
		if (bdA < bdB)
		{
			drt = (bdA + bdB) / 2;
		}
		else
		{
			drt = ((bdA + bdB + 8) / 2) % 8;
		}

	}


	return drt;


}

//此函数用来寻找和中心点连续相同灰度的最大个数
MinMaxIndex findSeqNum(int i, int j) //????????
{
	//cout << "执行了此句！\n" << endl;
	int K = 0;
	//求法向
	for (int mm = 0; mm < 8; mm++)
	{
		for (int m = 0; m < 8; m++)
		{
			K = mm + m;
			if (K >= 8)
			{
				K = K % 8;
			}

			temp = abs(g_calImg.at<float>(i + a8i[K], j + a8j[K]) - g_calImg.at<float>(i, j));//8领域点和中点作差

			/*		if (g_dstBiGrad.at<int>(i, j)>18)
			{
			TH1 = 25;
			}
			else
			TH1 = 5;*/

			if (temp <= TH1)
			{
				md_t++;
			}
			else//清空数据
				break;
		}
		//最大连续个数为4个的至多有一个
		tempM[mm] = md_t;
		//恢复初始状态
		md_t = 0;
		//temp[8] = { 0 };
	}
	//把最长的连续数据的起始下标保存下来
	man = (find_Max(tempM, 8));
	tempM[8] = { 0 };
	return man;
}
//用于得到中心点的法向量、现用不到。
float getAngle(int start_index, int seq_num)//start_index为连续相同的起始位置，seq_num为连续相同元素的个数,此为逆时针进行查找。
{
	float drt;//此函数返回的角度方向direction
	float medInd;
	size_t count = seq_num;

	//float angle;
	//处理链表

	//

	if ((count + 1) % 2 == 1)//出现偶数时进行插值
	{
		medInd = (count + 1) / 2 + 0.5;//可能出现数据转换问题
	}
	else
		medInd = (count + 1) / 2;

	//得到插值后对应8领域中哪个下标，然后就得到这个中心点的角度
	drt = start_index + medInd - 1;
	if (drt >= 8)
		drt = drt - 8;
	return drt;

}
float getIangle(int s, int len)
{
	float drt;
	drt = ((s + 8) - len / 2) % 8;
	return drt;
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
	controlFuction(1,//是否显示增边，黄色
		1,//是否减边，粉红色
		0,//是否实边填充,1为实边填充，0为隐边填充
		0,//是否填充邻边
		0,//是否显示初始红绿点
		1,//是否导出数据
		10,//是否显示叠加色
		0);//是否添加噪音
	//---------------------------------------------------------------

}
void controlFuction(bool n1, bool n2, bool n3, bool n4, bool n5, bool n6, bool n7, bool n8)
{
	real_edge = n1;
	virtual_edge = n2;
	real_fill = n3;
	bufill = n4;
	red_blue = n5;
	xls = n6;
	init_edge = n7;
	noice = n8;
}
//-----------------------------------【main( )函数】------------------------------------------------------------------------------  
//      描述：控制台应用程序的入口函数，我们的程序从这里开始  
//--------------------------------------------------------------------------------------------------------------------------------  
int main(int agrc, char** agrv)
{


	initMain();//初始化

	//载入原图  
	g_srcImage = imread(".\\sourcePicture\\3.bmp");

	//添加噪音
	if (noice)
	{
		salt(g_srcImage);
		imwrite("./output/noice.bmp", g_gradBmp);
	}

	//定义一个Mat类型并给其设定ROI区域  
	//g_srcImage = g_srcImage(Rect(0, 0, 250, 250));

	//判断是否读取成功
	if (!g_srcImage.data)
	{
		cout << "读取图片srcImage时出错！\n";
		return false;
	}


	//将原图转换为灰度图  
	cvtColor(g_srcImage, g_srcGrayImage, CV_BGR2GRAY);

	ShowHelpText(g_srcGrayImage);

	//histGram(g_srcGrayImage);
	Edge edge1;
	Edge edge(g_srcGrayImage);//传值初始化
	edge.Init();//深入初始化
	//edge.edgeDection(1,NULL);

	//创建Canny检测的tracebar
	namedWindow("Canny检测");
	//namedWindow("listk1"); namedWindow("listk1&&listk2");
	createTrackbar("参数值：", "Canny检测", &g_cannyLowThreshold, 160, edge.on_Canny);
	namedWindow("g_mergImg");//, CV_WINDOW_NORMAL
	//createTrackbar("梯度：", "g_mergImg", &TH, 80, edge.edgeDection);	
	//createTrackbar("相似度：", "g_mergImg", &TH1, 40, edge.edgeDection);
	//createTrackbar("相似度：", "g_edge[1]", &g_di, 80, edge.listK);
	edge.edgeDection();
	//edge.edgeDection1();
	g_srcGrayImage.convertTo(g_srcGrayImage, CV_32F);

	if (xls)
	{
		edge.outXls(g_srcGrayImage, "./output/gray.xls");
		//outxlsInt(g_biGrad, "./output/bigrad.xls");
	}

	//cout << g_ltedge << endl;
	//将原图转换为灰度图
	//g_dstImage.convertTo(g_dstImage, CV_8UC1);
	//namedWindow("dst");
	//imshow("dst", g_dstImage);
	//显示梯度图片

	g_srcGrad.convertTo(g_gradBmp, CV_8UC1);
	imshow("梯度图", g_gradBmp);
	imwrite("./output/grad.bmp", g_gradBmp);





	//轮询获取按键信息，若按下Q，程序退出  
	while ((char(waitKey(1)) != 'q')) {}
	return 0;
}
void getTypeEdge()
{
	vector<Mat> nTypeEdge(3);
	nTypeEdge[0] = Mat::zeros(g_listk.rows, g_listk.cols, CV_32S);
	nTypeEdge[1] = Mat::zeros(g_listk.rows, g_listk.cols, CV_32S);
	nTypeEdge[2] = Mat::zeros(g_listk.rows, g_listk.cols, CV_32S);
	Mat display_typeEdge(g_listk.rows, g_listk.cols, CV_8UC3, Scalar::all(0));
	for (int i = 1; i < g_listk.rows - 1; i++)
	{
		for (int j = 1; j < g_listk.cols - 1; j++)
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
				//0,255,255为黄色
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
	imshow("不同星型的边缘图", display_typeEdge);
	imwrite("./output/ntype.bmp", display_typeEdge);

	//display_typeEdge=Mat::zeros(g_listk.rows, g_listk.cols, CV_8UC3);
	g_nType = Mat::zeros(g_listk.rows, g_listk.cols, CV_32S);//再次初始化
}
//边缘递归生长函数
void TraceEdge(int x, int y, float grd)
{
	//对8邻域像素进行查询  
	int  yy, xx, k;
	for (k = 0; k < 8; k++)
	{
		xx = x + a8i[k];
		yy = y + a8j[k];

		//以该点为中心再进行跟踪
		if (xx < g_listk.rows - 1 && yy < g_listk.cols - 1 && xx>0 && yy>0)
		{
			if (g_srcGrad.at<float>(xx, yy) > 9 && g_imgFlg.at<int>(xx, yy) == 0)
			{
				//该点设为边界点  
				g_edge[2].at<float>(xx, yy) = 255;
				g_imgFlg.at<int>(xx, yy) = 1;
				TraceEdge(xx, yy, 10);

				//g_srcGrad.at<float>(xx, yy)
			}
		}
		else
		{
			cout << "out of range!\n";

		}

	}

}


void salt(cv::Mat& image){
	for (int k = 0; k < 3000; k++)
	{
		int i = rand() % image.cols;
		int j = rand() % image.rows;

		if (image.channels() == 1)
		{
			image.at<uchar>(j, i) = 255;
		}
		else
		{
			image.at<cv::Vec3b>(j, i)[0] = 255;
			image.at<cv::Vec3b>(j, i)[1] = 255;
			image.at<cv::Vec3b>(j, i)[2] = 255;
		}
	}
}

int getThresold(Point2i A, Point2i B, Point2i C)
{
	int nA, nB, nC;
	Point2i ptA, ptB, ptC;
	int na[LONG + 1] = { 0 }, nb[LONG + 1] = { 0 }, nc[LONG + 1] = { 0 };
	int th;

	nA = g_angle.at<float>(A.x, A.y);
	//if (nA==0)
	//{
	//	cout << "nA=" << nA <<" "<<A.x<<" "<<A.y<< endl;
	//}
	ptA = normalToXY(nA);
	nB = g_angle.at<float>(B.x, B.y);
	//if (nB == 0)
	//{
	//	cout << "nB=" << nB << " " << B.x << " " << B.y << endl;
	//}
	ptB = normalToXY(nB);
	nC = g_angle.at<float>(C.x, C.y);
	//if (nC == 0)
	//{
	//	cout << "nC=" << nC << " " << C.x << " " << C.y << endl;
	//}
	ptC = normalToXY(nC);

	for (int i = 1; i <= LONG; i++)
	{
		if (i <= LONG / 2)
		{
			if (!outOfrange(A.x + i*ptA.x, A.y + i*ptA.y) && !outOfrange(B.x + i*ptB.x, B.y + i*ptB.y) && !outOfrange(C.x + i*ptC.x, A.y + i*ptC.y))

			{
				na[i] = g_calImg.at<float>(A.x + i*ptA.x, A.y + i*ptA.y);
				nb[i] = g_calImg.at<float>(B.x + i*ptB.x, B.y + i*ptB.y);
				nc[i] = g_calImg.at<float>(C.x + i*ptC.x, A.y + i*ptC.y);
			}
			else
			{
				return TH;
			}

		}
		else
		{
			if (!outOfrange(A.x + (3 - i)*ptA.x, A.y + (3 - i)*ptA.y) && !outOfrange(B.x + (3 - i)*ptB.x, B.y + (3 - i)*ptB.y) && !outOfrange(C.x + (3 - i)*ptC.x, A.y + (3 - i)*ptC.y))
			{
				na[i] = g_calImg.at<float>(A.x + (3 - i)*ptA.x, A.y + (3 - i)*ptA.y);
				nb[i] = g_calImg.at<float>(B.x + (3 - i)*ptB.x, B.y + (3 - i)*ptB.y);
				nc[i] = g_calImg.at<float>(C.x + (3 - i)*ptC.x, A.y + (3 - i)*ptC.y);
			}
			else
			{
				return TH;
			}

		}
	}
	int sumA, sumB, sumC, sumF, sumD, sumE;
	int sumABC, sumDEF;
	sumA = na[3] + na[1] + na[2];
	sumB = nb[3] + nb[1] + nb[2];
	sumC = nc[3] + nc[1] + nc[2];

	sumD = na[6] + na[4] + na[5];
	sumE = nb[6] + nb[4] + nb[5];
	sumF = nc[6] + nc[4] + nc[5];

	sumABC = sumA + sumB + sumC;
	sumDEF = sumD + sumE + sumF;

	th = abs(sumABC - sumDEF) / (3 * 3);

	//cout << "th=" << th << endl;
	//cout << "Ax=" << A.x << " " << "Ay=" << A.y << endl;
	//cout << "Bx=" << B.x << " " << "By=" << B.y << endl;
	//cout << "Cx=" << C.x << " " << "Cy=" << C.y << endl;
	//cout << "sumA=" << sumA << " " << "sumB=" << sumB << " " << "sumC=" << sumC << endl;
	//cout << "sumD=" << sumD << " " << "sumE=" << sumE << " " << "sumF=" << sumF << endl;
	//cout << "sumABC=" << sumABC << " " << "sumDEF=" << sumDEF << endl;
	//if (A.x = 394 && A.y == 250)
	//{
	//	cout << "394       250" << endl;
	//	cout << "anA=" << g_angle.at<float>(394, 250) << endl;

	//	cout << "anB=" << g_angle.at<float>(395, 250) << endl;
	//	cout << "anC=" << g_angle.at<float>(396, 250) << endl;
	//}
	//if (A.x = 396 && A.y == 259)
	//{
	//	cout << "396 259" << endl;
	//	cout << "anA=" << g_angle.at<float>(396, 259) << endl;

	//	cout << "anB=" << g_angle.at<float>(397, 259) << endl;
	//	cout << "anC=" << g_angle.at<float>(398, 259) << endl;
	//}

	return th;
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
void fillAdjacent(int i, int j)
{
	int ai, aj;
	ai = normalToXY(g_angle.at<float>(i, j)).x;
	aj = normalToXY(g_angle.at<float>(i, j)).y;

	if (!outOfrange(i + 2 * ai, j + 2 * aj))//
	{
		g_imgFlg.at<int>(i + ai, j + aj) = 11;
		//g_imgFlg.at<int>(i + 2*ai, j + 2*aj) = 21;

	}

	if (!outOfrange(i - 2 * ai, j - 2 * aj))//
	{
		g_imgFlg.at<int>(i - ai, j - aj) = 11;
		//g_imgFlg.at<int>(i - 2*ai, j - 2*aj) = 21;

	}
}

