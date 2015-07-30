// ---------------------------------- - 【程序说明】----------------------------------------------
//      程序名称:OpenCV边缘检测：新方法实现边缘特征提取并能定位噪音
//      开发所用IDE版本：Visual Studio 2013  
//      <span style="white-space:pre">  </span>开发所用OpenCV版本：    2.4.9  
//      2014年11月19日 Create by 杨超  
//    
//----------------------------------------------------------------------------------------------  
#include "edge.h"  
#include"stdlib.h"
#include "chistogram.h"
#include "outXls.h"
#include "structdef.h"

using namespace std;
using namespace cv;

//-----------------------------------【全局变量声明部分】--------------------------------------  
//      描述：全局变量声明  
//-----------------------------------------------------------------------------------------------  
//原图，原图的灰度版，目标图  

Mat g_srcImage, g_srcGrayImage, g_dstImage;


MyStruct mmi, mmik, man;

//梯度图
Mat g_srcGrad, g_outEdge;
Mat g_medImg;//中值滤波
Mat g_mixGS;
Mat g_PTh, g_PType, g_PBuffer, g_PBigSmall;
Mat g_type;
Mat g_thresold;
int g_dg = 8, g_di;//Trackbar位置参数
bool rgb = false;
float g_avgACBC;
//Canny边缘检测相关变量  
Mat g_cannyDetectedEdges;
int g_cannyLowThreshold = 10;//TrackBar位置参数  
Mat gray;
//YC边缘检测相关变量
Mat g_listKS;//存放此方法边缘图
Mat g_imgFlg;//图像标志位
Mat g_nSectionFlg;//区域划分处理标记
Mat g_calImg;//转换了成了浮点数据类型的灰度值
Mat g_calImg_I;
Mat g_angle;//法向量
Mat g_intAngle;//把法向量统一到8个方向上
Mat g_gradBmp;//梯度图像
Mat g_nType;//用来保存边缘点是几类型的（星型）
Mat g_typeEdge;//保存星型边缘图
Mat g_biGrad, g_dstBiGrad;
int g_numPixel;
Mat g_circle;
Mat g_kernel = (Mat_<float>(5, 5) << 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1,
	1, 1, 1, 1, 1,
	1, 1, 1, 1, 1,
	1, 1, 1, 1, 1);//核函数

//Mat g_kernel = (Mat_<float>(7, 7) << 1, 1, 1, 1, 1, 1, 1,
//	1, 1, 1, 1, 1, 1, 1,
//	1, 1, 1, 1, 1, 1, 1,
//	1, 1, 1, 1, 1, 1, 1,
//	1, 1, 1, 1, 1, 1, 1);//核函数

vector<Point2i> g_missedPt;
vector<Mat> g_edge(3);
vector<Mat> g_srcImageChannels, g_srcImageChannels_canny;//保存原图的通道
vector<Mat> srcBGR(3);
Mat g_mergImg;//合并彩色边缘
Mat g_srcImageBGR, g_srcImageBGR_canny;//混合边缘通道后的图像
Mat g_srcImageB, g_srcImageG, g_srcImageR;
Mat g_gtImg;
Mat g_nTypeFlag;
Mat g_dedgeX, g_dedgeY;//隐边缘点位置
Mat g_inedgeX, g_inedgeY;//显示边缘点位置
vector<Mat> nTypeEdge(3);

//Mat ltedge(g_listk.rows, g_listk.cols, CV_8SC2, Scalar(0, 0));
Mat_ <Vec3b> img(240, 320, Vec3b(0, 255, 0));//Mat_ 为Mat的模板类型
float temp = 0;
vector<float> tempB_t, tempB, tempS;//_t表示临时存放变量
int tempM[8] = { 0 };//存入循环遍历每层中的最大连续相同的个数
int md_t = 0, md;
int module;//模板大小设置
float dsm, grd;//dsm为中值与原图中心点差值，grd梯度值
float tempGrad[8] = { 0 };
vector<Point2i> arrP;
Mat g_minGrad, g_maxGrad;
Mat g_grad0, g_grad1, g_grad2, g_grad3, g_grad4, g_grad5, g_grad6, g_grad7, g_grad8;
vector<startPT> S37,S5;
int m1 = 0, m2 = 0;
bool real_edge = 0, virtual_edge = 0, real_fill = 0, bufill = 0, green = 0, xls = 0, init_edge = 0, noice = 0;

//图像8领域下标变化索引
int a8i[] = { 0, -1, -1, -1, 0, 1, 1, 1, 0 };
int a8j[] = { 1, 1, 0, -1, -1, -1, 0, 1, 1 };
//领域索引，备用
int a5[] = { 2, 1, 0, -1, -2 };
Mat bigThanTh;
int N1, N2;

int TH1=1;
int calTime;
bool find7;

int TH = 0, TH2 = 2 * TH1, TH_D;
int n1 = 0, n2 = 0;
int cnt = 0;
int gTH = 50, nGrad = 8;//gTH为梯度阈值，nGrad 为5*5模板中》gTH的个数
int bluePt;
int startPt;
int nTH;
int g_temth[8] = { 0 };
Mat maxminD, opmaxminD;
bool(*pfindStart)(Point2i centerPt, Point2i &A, Point2i &B, Point2i &AC, Point2i &BC, int &direction, bool & min_max);//declare a function point

int main(int agrc, char** agrv)
{
	initMain();//first inital
	pfindStart = findStartPt3;
	//load srcImage
	g_srcImage = imread(".\\sourcePicture\\test/1.jpg"); //noicePicture 2018 poisson.jpg saltpepper.bmp edgenoice.bmp gaussian.bmp
	//load success or not
	if (!g_srcImage.data)
	{
		cout << "读取图片srcImage时出错！\n";
		return false;
	}

	//load GT image
	g_gtImg = imread(".\\sourcePicture\\gtimage\\1.png");
#if (MIXGAT==1)

	//imshow("1",g_gtImg);
	cvtColor(g_gtImg, g_gtImg, CV_BGR2GRAY);
	g_gtImg.convertTo(g_gtImg, CV_32F);
	if (!g_gtImg.data)
	{
		cout << "读取图片gtImage时出错！\n";
		return false;
	}

#endif

	//outxls(g_gtImg, "output/biedge.xls");
	//blur(g_srcImage, g_srcImage, Size(5, 5));
	//定义一个Mat类型并给其设定ROI区域  
	//g_srcImage = g_srcImage(Rect(0, 0, 50, 15));



	//一、默认把彩色图像转换为灰度图像----------------------------------
	//*/
	cvtColor(g_srcImage, g_srcGrayImage, CV_BGR2GRAY);
	//imshow("YUV",g_srcGrayImage);
	int chan=g_srcGrayImage.channels();
	Mat YUV_channel;
	//split(g_srcGrayImage, YUV_channel);
	//Mat Y_channel = YUV_channel.at(1);
	Edge edge(g_srcGrayImage);//灰色传值初始化
	//cout << "tes=" << typeid((int)g_srcGrayImage.at<uchar>(10,10)).name() << endl;
	//*/
	//二、取消注释下面语句可以实现彩色图像检测--------------------------
	//Edge edge(g_srcImage);//彩色传值初始化




	//outXls("./output/maxgrad.xls", g_maxGrad, "float");
	//outXls("./output/mm.xls", maxminD, "float");
	//outXls("./output/opmm.xls", opmaxminD, "float");


	//namedWindow("g_mergImg", CV_WINDOW_NORMAL);//, CV_WINDOW_NORMAL
	//createTrackbar("梯度：", "g_mergImg", &TH, 80, edge.edgeDection);	

	//createTrackbar("相似度：", "g_mergImg", &TH1, 240, edge.edgeDection);
	//createTrackbar("相似度：", "g_edge[1]", &g_di, 80, edge.listK);

	//deNoice(gray.rows, gray.cols);
    
	for (int m = 1; m < NPT; m++)
	{
		
		edge.Init();//深入初始化
		//showHistogram(g_minGrad);
		namedWindow("th");
		imwrite("./output/th.png", g_PTh);
		cout << "the " << m << " th:" << TH1 << endl;
		nTH = m;
		//edge.edgeDection();
		edge.edgeDetection1();

		TH1 = TH1 + 2;
		
	}
	outXls("./output/02 g_PType.xls", g_PType, "int");
	outXls("./output/bigThanTh.xls", bigThanTh, "int");

	cout << "g_numPixel=" << g_numPixel << endl;
	g_srcGrayImage.convertTo(g_srcGrayImage, CV_32F);


	//创建Canny检测的tracebar
	namedWindow("canny");
	//namedWindow("listk1"); namedWindow("listk1&&listk2");
	createTrackbar("参数值：", "canny", &g_cannyLowThreshold, 160, edge.on_Canny);

	outXls("./output/gray.xls", g_srcGrayImage, "float");
	
	//outXls("./output/ntype.xls", g_nType, "int");
	if (MIXGAT == 1)
	{
		outXls("./output/mixNtype.xls", g_type, "float");
	}

	g_maxGrad.convertTo(g_gradBmp, CV_8UC1);
	imshow("梯度图", g_gradBmp);
	imwrite("./output/grad.png", g_gradBmp);
	//showHistogram(g_maxGrad, "maxGrad");
	
	//showHistogram(g_PTh, "TH");
	//轮询获取按键信息，若按下Q，程序退出  
	while ((char(waitKey(1)) != 'q'))
	{
	}
	return 0;
}