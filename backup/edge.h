#include <opencv2/highgui/highgui.hpp>  
#include <opencv2/imgproc/imgproc.hpp>  
#include "highgui.h"  
#include <cv.h>  
#include <cvaux.h>
#include <iostream>
#include <fstream>
using namespace std;





//-----------------------------------【宏定义部分】--------------------------------------------
//	描述：定义一些辅助宏
//------------------------------------------------------------------------------------------------
//#define TH1 10//固定的边缘点几何定义定义阈值
#define LONG 6
#define STEP 10//最大填充长度
#define BLUE_FLAG 0  //非0表示显示一类缓冲区域




using namespace cv;


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
	static void edgeDection();//核心边缘检测方法函数
	static void edgeDection1();
	static void on_Canny(int, void*);//Canny检测
	void outXls(Mat outxls, string filename);//边缘类中的导出到Excel的函数
private:
	Mat src;
	Mat gray;
	Mat dst;
	//Mat listK1, listK2;
};

//用于保存最大最小值及其对应的下标结构体
typedef struct MyStruct
{
	int m_num;
	int m_index;
}
MinMaxIndex;

//备用结构体变量
typedef struct MyPoint
{
	float x;
	float y;

}
PT;

//-----------------------------------【全局变量声明部分】--------------------------------------  
//      描述：全局变量声明  
//-----------------------------------------------------------------------------------------------  
//原图，原图的灰度版，目标图  
Mat g_srcImage, g_srcGrayImage, g_dstImage;

//梯度图
Mat g_srcGrad, g_outEdge;
Mat g_medImg;//中值滤波
Mat g_thresold;
int g_dg = 8, g_di;//Trackbar位置参数
Mat g_listk;

//Canny边缘检测相关变量  
Mat g_cannyDetectedEdges;
int g_cannyLowThreshold = 1;//TrackBar位置参数  

//YC边缘检测相关变量
Mat g_listKS;//存放此方法边缘图
Mat g_imgFlg;//图像标志位
Mat g_calImg;//转换了成了浮点数据类型的灰度值
Mat g_angle;//法向量
Mat g_intAngle;//把法向量统一到8个方向上
Mat g_gradBmp;//梯度图像
Mat g_nType;//用来保存边缘点是几类型的（星型）
Mat g_typeEdge;//保存星型边缘图
Mat g_biGrad, g_dstBiGrad;

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


vector<Mat> g_edge(3);
vector<Mat> g_srcImageChannels;//保存原图的通道

Mat g_mergImg;//合并彩色边缘
Mat g_srcImageBGR;//混合边缘通道后的图像
Mat g_srcImageB, g_srcImageG, g_srcImageR;

Mat g_dedgeX, g_dedgeY;//隐边缘点位置
Mat g_inedgeX, g_inedgeY;//显示边缘点位置

//Mat ltedge(g_listk.rows, g_listk.cols, CV_8SC2, Scalar(0, 0));
Mat_ <Vec3b> img(240, 320, Vec3b(0, 255, 0));//Mat_ 为Mat的模板类型
float temp = 0;
vector<float> tempB_t, tempB, tempS;//_t表示临时存放变量
int tempM[8] = { 0 };//存入循环遍历每层中的最大连续相同的个数
int md_t = 0, md;
int seq_num = 0;//连续相同的最大个数
int start_pt;
int module;//模板大小设置
int g_tracenum;//save the number of tracing by testing
MinMaxIndex mmi, mmik, man;//定义函数的两个返回值
float dsm, grd;//dsm为中值与原图中心点差值，grd梯度值
float tempGrad[8] = { 0 };
vector<Point2i> arrP;
Point2i A, B, C, D;//定义四个顶点


//图像8领域下标变化索引
int a8i[] = { 0, -1, -1, -1, 0, 1, 1, 1, 0 };
int a8j[] = { 1, 1, 0, -1, -1, -1, 0, 1, 1 };
//领域索引，备用
int a5[] = { 2, 1, 0, -1, -2 };


int TH = 0, TH1 = 20;
int n1 = 0, n2 = 0;
int cnt = 0;
int gTH = 50, nGrad = 8;//gTH为梯度阈值，nGrad 为5*5模板中》gTH的个数

int g_temth[8] = { 0 };
int g_maxth;
bool g_clk;
//YC边缘检测相关函数
//template<class T>
void outxls(Mat listK1, string filename);//导致数据到Excel表格
void outxlsInt(Mat listK1, string filename);//导致数据到Excel表格
MinMaxIndex findSeqNum(int i, int j);//得到连续相同的数目
float getAngle(int i, int seq_num);//计算法向量
float getIangle(int i, int len);//计算法向量
int getIangle(Point2i centPt, Point2i A);//
float getMedLine(int bdA, int bdB, int direction);
void TraceEdge(int x, int y, float grd);
bool clockDirection(int b, int d);
void calType(int i, int j);
void strEdge(int i, int j, int bd, int clk);
bool isSeqAngle(float nd1, float nd2);//判断角度是否连续
//float cosAngle(float nd1, float nd2);//判断角度是否连续,有错误？
int bordDirection(int i, int j);//求边缘走向
int drt1;
bool outOfrange(int i, int j);//越界检测
void fillPixel(int i, int j, float nd, int bd);
void fillAdjacent(int i, int j);
//template<class T>
Point2f drtToVector(float nd);//把角度转换为向量坐标
Point2f drtToVector(int bd);
void fillRect(Point2i A, Point2i B, Point2i C, Point2i D);//填充四边形
int getThresold(Point2i A, Point2i B, Point2i C);//由连续的三个点产生一个动态阈值
Point2i normalToXY(float nv);//把法向量转为坐标形式
int getDth(Point2i A, Point2i B, Point2i C, Point2i D, Point2i E, int direction);
//添加椒盐噪音函数、
void salt(cv::Mat&);
int  lengthAB(int A, int B, int direction);
void getMix();
void nextInitial();
void getTypeEdge();
bool startPoint(int i, int j);
bool semiPlanePt(int i, int j);
bool TraceEdgeTwoBoundary(Point2i A, Point2i B, Point2i C, Point2i &D, Point2i &E, int  direction);
bool real_edge = 0, virtual_edge = 0, real_fill = 0, bufill = 0, red_blue = 0, xls = 0, init_edge = 0, noice = 0;
bool findStartPt(Point2i centerPt, Point2i &A, Point2i &B, Point2i &AC, Point2i &BC, int &nType, int &direction);
void controlFuction(bool n1, bool n2, bool n3, bool n4, bool n5, bool n6, bool n7, bool n8);//控制函数
void initMain();//载入图像前的初始设置

//-----------------------------------【类声明部分】--------------------------------------  
//      描述：Edge边缘检测类的声明  
//-----------------------------------------------------------------------------------------------  
static void ShowHelpText(Mat Image);//系统提示信息
//static void on_Canny(int, void*);//Canny边缘检测窗口滚动条的回调函数  
void histGram(Mat hist);//计算图像直方图
//template <class T>
MinMaxIndex find_Max(int *arr, int N);//得到数组的最大值，并保存其下标
MinMaxIndex find_Min(int *arr, int N);//得到数组的最小值，并保存其下标
template <class T>
T findMax(T *arr, int N);
template <class T>
T findMin(T *arr, int N);
bool findSeqNumSE(Point2i A);
