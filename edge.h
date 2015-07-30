#pragma  once
#ifndef EDGE_H
#define EDGE_H

#include <opencv2/highgui/highgui.hpp>  
#include <opencv2/imgproc/imgproc.hpp>  
#include "highgui.h"  
#include "structdef.h"
#include <cv.h>  
#include <cvaux.h>
#include <iostream>
#include <fstream>
#include <windows.h>
#include <string.h>
#include <numeric>

using namespace std;
using namespace cv;
//-----------------------------------【宏定义部分】--------------------------------------------
//	描述：定义一些辅助宏
//------------------------------------------------------------------------------------------------
#define LONG 6
#define STEP 10//最大填充长度
#define BLUE_FLAG 0  //非0表示显示一类缓冲区域
#define  LTH  2
#define  NPT 81
#define  MMC 0  //MIN MAX CONSTRAIN
#define START  1//1找起止 2找起止且局部是最值
#define STRETCH 1//1正反都最小 2正最小 3正最近 
//一、11 12 
//二、22 
//三、23 
#define MIXGAT 0
//用于保存最大最小值及其对应的下标结构体


//-----------------------------------【全局变量声明部分】--------------------------------------  
//      描述：全局变量声明  
//-----------------------------------------------------------------------------------------------  
//原图，原图的灰度版，目标图  

//global Struct declaration is more complicated than normal type
extern MyStruct mmi, mmik, man;


extern Mat g_srcImage, g_srcGrayImage, g_dstImage;

//梯度图
extern Mat g_srcGrad, g_outEdge;
extern Mat g_medImg;//中值滤波
extern Mat g_mixGS;
extern Mat g_PTh, g_PType, g_PBuffer, g_PBigSmall;
extern Mat g_type;
extern Mat g_thresold;
extern int g_dg, g_di;//Trackbar位置参数
extern bool rgb;
extern float g_avgACBC;
//Canny边缘检测相关变量  
extern Mat g_cannyDetectedEdges;
extern int g_cannyLowThreshold;//TrackBar位置参数  
extern Mat gray;
//YC边缘检测相关变量
extern Mat g_listKS;//存放此方法边缘图
extern Mat g_imgFlg;//图像标志位
extern Mat g_nSectionFlg;//区域划分处理标记
extern Mat g_calImg;//转换了成了浮点数据类型的灰度值
extern Mat g_calImg_I;
extern Mat g_angle;//法向量
extern Mat g_intAngle;//把法向量统一到8个方向上
extern Mat g_gradBmp;//梯度图像
extern Mat g_nType;//用来保存边缘点是几类型的（星型）
extern Mat g_typeEdge;//保存星型边缘图
extern Mat g_biGrad, g_dstBiGrad;
extern int g_numPixel;
extern Mat g_kernel;//核函数

//Mat g_kernel = (Mat_<float>(7, 7) << 1, 1, 1, 1, 1, 1, 1,
//	1, 1, 1, 1, 1, 1, 1,
//	1, 1, 1, 1, 1, 1, 1,
//	1, 1, 1, 1, 1, 1, 1,
//	1, 1, 1, 1, 1, 1, 1);//核函数

extern vector<Point2i> g_missedPt;
extern vector<Mat> g_edge;
extern vector<Mat> g_srcImageChannels, g_srcImageChannels_canny;//保存原图的通道
extern vector<Mat> srcBGR;
extern Mat g_mergImg;//合并彩色边缘
extern Mat g_srcImageBGR,g_srcImageBGR_canny;//混合边缘通道后的图像
extern Mat g_srcImageB, g_srcImageG, g_srcImageR;
extern Mat g_gtImg;
extern Mat g_nTypeFlag;
extern Mat g_circle;
extern vector<Mat> nTypeEdge;
//Mat ltedge(g_listk.rows, g_listk.cols, CV_8SC2, Scalar(0, 0));
extern Mat_ <Vec3b> img;//Mat_ 为Mat的模板类型
extern float temp;
extern vector<float> tempB_t, tempB, tempS;//_t表示临时存放变量
extern int tempM[8];//存入循环遍历每层中的最大连续相同的个数
extern int md_t, md;
extern int module;//模板大小设置
extern float dsm, grd;//dsm为中值与原图中心点差值，grd梯度值
extern float tempGrad[8];
extern vector<Point2i> arrP;
extern Mat g_minGrad, g_maxGrad;
extern Mat g_grad0, g_grad1, g_grad2, g_grad3, g_grad4, g_grad5, g_grad6, g_grad7, g_grad8;
extern vector<startPT> S37,S5;

extern bool real_edge, virtual_edge, real_fill, bufill, green, xls, init_edge, noice;

extern bool(*pfindStart)(Point2i centerPt, Point2i &A, Point2i &B, Point2i &AC, Point2i &BC, int &direction, bool& min_max);//declare a function point
extern int calTime;
extern bool find7;

//图像8领域下标变化索引
extern int a8i[];
extern int a8j[];
//领域索引，备用
extern int a5[] ;
extern int TH1;
extern int TH , TH2, TH_D;
extern int n1, n2;
extern int cnt;
extern int gTH , nGrad;//gTH为梯度阈值，nGrad 为5*5模板中》gTH的个数
extern int bluePt;
extern int startPt;
extern int nTH;
extern int g_temth[8];
extern Mat bigThanTh;
extern Mat maxminD, opmaxminD;

//自定义边缘类
class Edge
{
public:
	Edge()
	{
		//cout << "默认构造函数执行：\n " << endl;
	}
	Edge(Mat in);//构造函数
	~Edge();
	void Init();
	void showImg();
	static void edgeDetection1();//核心边缘检测方法函数
	static void edgeDetection2();
	void edgeDetection3();
	static void on_Canny(int, void*);//Canny检测
private:
	Mat src;
	Mat dst;
	//Mat listK1, listK2;
};

extern int N1, N2;
//YC边缘检测相关函数
//template<class T>
bool  GetMissedPoint(Point2i centPtB, Point2i C, Point2i  E, int direction, Point2i& missedPt);
float getDth(Point2i A, Point2i B, Point2i C, Point2i D, Point2i E, int direction);
float getAngle(int i, int seq_num);//计算法向量
int getIangle(Point2i centPt, Point2i A);//
float getMedLine(int bdA, int bdB, int direction);
void TraceEdge(int x, int y, float grd);
bool clockDirection(int b, int d);
void calType(int i, int j);
void strEdge(int i, int j, int bd, int clk);
bool isSeqAngle(float nd1, float nd2);//判断角度是否连续
//float cosAngle(float nd1, float nd2);//判断角度是否连续,有错误？
int bordDirection(int i, int j);//求边缘走向
//int drt1;
bool outOfrange(int i, int j);//越界检测
void fillPixel(int i, int j, float nd, int bd);
void fillAdjacent(int i, int j);
//template<class T>
Point2f drtToVector(float nd);//把角度转换为向量坐标
Point2f drtToVector(int bd);
void fillRect(Point2i A, Point2i B, Point2i C, Point2i D);//填充四边形
int getThresold(Point2i A, Point2i B, Point2i C);//由连续的三个点产生一个动态阈值
Point2i normalToXY(float nv);//把法向量转为坐标形式

//添加椒盐噪音函数、
void salt(cv::Mat&);
int  lengthAB(int A, int B, int direction);
void getMix();
void nextInitial();
void getTypeEdge();
bool startPoint(int i, int j);
bool semiPlanePt(int i, int j);
bool TraceEdgeTwoBoundary1(Point2i A, Point2i B, Point2i C, Point2i &D, Point2i &E, int  direction);
bool TraceEdgeTwoBoundary2(Point2i A, Point2i B, Point2i C, Point2i &D, Point2i &E, int  direction,bool& min_max);
bool TraceEdgeTwoBoundary3(Point2i A, Point2i B, Point2i &D, int  direction, bool min_max,Point2i AA,int type,int tag,vector<Point2i> &stretchD);

bool findStartPt1(Point2i centerPt, Point2i &A, Point2i &B, Point2i &AC, Point2i &BC, int &direction);
bool findStartPt2(Point2i centerPt, Point2i &A, Point2i &B, Point2i &AC, Point2i &BC,int &direction,bool &min_max);
bool findStartPt3(Point2i centerPt, Point2i &A, Point2i &B, Point2i &AC, Point2i &BC, int &direction, bool &min_max);

void controlFuction(bool n1, bool n2, bool n3, bool n4, bool n5, bool n6, bool n7, bool n8);//控制函数
void initMain();//载入图像前的初始设置
int nSection(Point2i centPt);
float absAB_BGR(Point2i A1, Point2i A2);
void deNoice(int row , int col);
double deviation(vector<float> v);
bool isInsec(vector <int> &ind);
bool isTag(Point2i centPt);
void clearShort(vector <Point2i> &Pt);
int maxSeqence(vector <int> &ind);
MyStruct searchMValue(Point2i centPt, int min_max);
bool searchLocalExtremum1(Point2i centPt, Point2i * Pts, int min_max, int& index);
bool searchLocalExtremum2(Point2i centPt, Point2i * Pts, int min_max, int& index);
bool searchLocalExtremum3(Point2i centPt, Point2i * Pts, int min_max, int& index);
//-----------------------------------【类声明部分】--------------------------------------  
//      描述：Edge边缘检测类的声明  
//-----------------------------------------------------------------------------------------------  
static void ShowHelpText(Mat Image);//系统提示信息
//static void on_Canny(int, void*);//Canny边缘检测窗口滚动条的回调函数  
void histGram(Mat hist);//计算图像直方图
//template <class T>
MyStruct find_Max(int *arr, int N);//得到数组的最大值，并保存其下标
MyStruct find_Min(int *arr, int N);//得到数组的最小值，并保存其下标
template <class T>
T findMax(T *arr, int N);
template <class T>
T findMin(T *arr, int N);
bool findSeqNumSE(Point2i A);
template <class T>
string num2str(T num);
LPCWSTR stringToLPCWSTR(std::string orig);
int nSection1(Point2i centPt);
void tagTypeColor(int b, int g, int r, int i, int j);
float sum(Mat mat, int n);
//bool getSixPoints(Point2i A, Point2i B, vector<Point2i>& pt_same, vector<Point2i>& pt_diff, Vector<Point2i>& pt, int direction);
float getSmaller(float centPt, float* pt_diff_next, bool min_max);
void calcPixelAttribute(Point2i pt);
int calcStartPixelType(Point2i centerPt);
void getN1();
void InsertSort(int* array, int* index, int length);
void SemiCirleEdge(vector<Point2i>&stretchD, Point2i B, Point2i D, int tag);
#endif
