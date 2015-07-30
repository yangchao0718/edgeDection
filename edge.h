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
//-----------------------------------���궨�岿�֡�--------------------------------------------
//	����������һЩ������
//------------------------------------------------------------------------------------------------
#define LONG 6
#define STEP 10//�����䳤��
#define BLUE_FLAG 0  //��0��ʾ��ʾһ�໺������
#define  LTH  2
#define  NPT 81
#define  MMC 0  //MIN MAX CONSTRAIN
#define START  1//1����ֹ 2����ֹ�Ҿֲ�����ֵ
#define STRETCH 1//1��������С 2����С 3����� 
//һ��11 12 
//����22 
//����23 
#define MIXGAT 0
//���ڱ��������Сֵ�����Ӧ���±�ṹ��


//-----------------------------------��ȫ�ֱ����������֡�--------------------------------------  
//      ������ȫ�ֱ�������  
//-----------------------------------------------------------------------------------------------  
//ԭͼ��ԭͼ�ĻҶȰ棬Ŀ��ͼ  

//global Struct declaration is more complicated than normal type
extern MyStruct mmi, mmik, man;


extern Mat g_srcImage, g_srcGrayImage, g_dstImage;

//�ݶ�ͼ
extern Mat g_srcGrad, g_outEdge;
extern Mat g_medImg;//��ֵ�˲�
extern Mat g_mixGS;
extern Mat g_PTh, g_PType, g_PBuffer, g_PBigSmall;
extern Mat g_type;
extern Mat g_thresold;
extern int g_dg, g_di;//Trackbarλ�ò���
extern bool rgb;
extern float g_avgACBC;
//Canny��Ե�����ر���  
extern Mat g_cannyDetectedEdges;
extern int g_cannyLowThreshold;//TrackBarλ�ò���  
extern Mat gray;
//YC��Ե�����ر���
extern Mat g_listKS;//��Ŵ˷�����Եͼ
extern Mat g_imgFlg;//ͼ���־λ
extern Mat g_nSectionFlg;//���򻮷ִ�����
extern Mat g_calImg;//ת���˳��˸����������͵ĻҶ�ֵ
extern Mat g_calImg_I;
extern Mat g_angle;//������
extern Mat g_intAngle;//�ѷ�����ͳһ��8��������
extern Mat g_gradBmp;//�ݶ�ͼ��
extern Mat g_nType;//���������Ե���Ǽ����͵ģ����ͣ�
extern Mat g_typeEdge;//�������ͱ�Եͼ
extern Mat g_biGrad, g_dstBiGrad;
extern int g_numPixel;
extern Mat g_kernel;//�˺���

//Mat g_kernel = (Mat_<float>(7, 7) << 1, 1, 1, 1, 1, 1, 1,
//	1, 1, 1, 1, 1, 1, 1,
//	1, 1, 1, 1, 1, 1, 1,
//	1, 1, 1, 1, 1, 1, 1,
//	1, 1, 1, 1, 1, 1, 1);//�˺���

extern vector<Point2i> g_missedPt;
extern vector<Mat> g_edge;
extern vector<Mat> g_srcImageChannels, g_srcImageChannels_canny;//����ԭͼ��ͨ��
extern vector<Mat> srcBGR;
extern Mat g_mergImg;//�ϲ���ɫ��Ե
extern Mat g_srcImageBGR,g_srcImageBGR_canny;//��ϱ�Եͨ�����ͼ��
extern Mat g_srcImageB, g_srcImageG, g_srcImageR;
extern Mat g_gtImg;
extern Mat g_nTypeFlag;
extern Mat g_circle;
extern vector<Mat> nTypeEdge;
//Mat ltedge(g_listk.rows, g_listk.cols, CV_8SC2, Scalar(0, 0));
extern Mat_ <Vec3b> img;//Mat_ ΪMat��ģ������
extern float temp;
extern vector<float> tempB_t, tempB, tempS;//_t��ʾ��ʱ��ű���
extern int tempM[8];//����ѭ������ÿ���е����������ͬ�ĸ���
extern int md_t, md;
extern int module;//ģ���С����
extern float dsm, grd;//dsmΪ��ֵ��ԭͼ���ĵ��ֵ��grd�ݶ�ֵ
extern float tempGrad[8];
extern vector<Point2i> arrP;
extern Mat g_minGrad, g_maxGrad;
extern Mat g_grad0, g_grad1, g_grad2, g_grad3, g_grad4, g_grad5, g_grad6, g_grad7, g_grad8;
extern vector<startPT> S37,S5;

extern bool real_edge, virtual_edge, real_fill, bufill, green, xls, init_edge, noice;

extern bool(*pfindStart)(Point2i centerPt, Point2i &A, Point2i &B, Point2i &AC, Point2i &BC, int &direction, bool& min_max);//declare a function point
extern int calTime;
extern bool find7;

//ͼ��8�����±�仯����
extern int a8i[];
extern int a8j[];
//��������������
extern int a5[] ;
extern int TH1;
extern int TH , TH2, TH_D;
extern int n1, n2;
extern int cnt;
extern int gTH , nGrad;//gTHΪ�ݶ���ֵ��nGrad Ϊ5*5ģ���С�gTH�ĸ���
extern int bluePt;
extern int startPt;
extern int nTH;
extern int g_temth[8];
extern Mat bigThanTh;
extern Mat maxminD, opmaxminD;

//�Զ����Ե��
class Edge
{
public:
	Edge()
	{
		//cout << "Ĭ�Ϲ��캯��ִ�У�\n " << endl;
	}
	Edge(Mat in);//���캯��
	~Edge();
	void Init();
	void showImg();
	static void edgeDetection1();//���ı�Ե��ⷽ������
	static void edgeDetection2();
	void edgeDetection3();
	static void on_Canny(int, void*);//Canny���
private:
	Mat src;
	Mat dst;
	//Mat listK1, listK2;
};

extern int N1, N2;
//YC��Ե�����غ���
//template<class T>
bool  GetMissedPoint(Point2i centPtB, Point2i C, Point2i  E, int direction, Point2i& missedPt);
float getDth(Point2i A, Point2i B, Point2i C, Point2i D, Point2i E, int direction);
float getAngle(int i, int seq_num);//���㷨����
int getIangle(Point2i centPt, Point2i A);//
float getMedLine(int bdA, int bdB, int direction);
void TraceEdge(int x, int y, float grd);
bool clockDirection(int b, int d);
void calType(int i, int j);
void strEdge(int i, int j, int bd, int clk);
bool isSeqAngle(float nd1, float nd2);//�жϽǶ��Ƿ�����
//float cosAngle(float nd1, float nd2);//�жϽǶ��Ƿ�����,�д���
int bordDirection(int i, int j);//���Ե����
//int drt1;
bool outOfrange(int i, int j);//Խ����
void fillPixel(int i, int j, float nd, int bd);
void fillAdjacent(int i, int j);
//template<class T>
Point2f drtToVector(float nd);//�ѽǶ�ת��Ϊ��������
Point2f drtToVector(int bd);
void fillRect(Point2i A, Point2i B, Point2i C, Point2i D);//����ı���
int getThresold(Point2i A, Point2i B, Point2i C);//�����������������һ����̬��ֵ
Point2i normalToXY(float nv);//�ѷ�����תΪ������ʽ

//��ӽ�������������
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

void controlFuction(bool n1, bool n2, bool n3, bool n4, bool n5, bool n6, bool n7, bool n8);//���ƺ���
void initMain();//����ͼ��ǰ�ĳ�ʼ����
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
//-----------------------------------�����������֡�--------------------------------------  
//      ������Edge��Ե����������  
//-----------------------------------------------------------------------------------------------  
static void ShowHelpText(Mat Image);//ϵͳ��ʾ��Ϣ
//static void on_Canny(int, void*);//Canny��Ե��ⴰ�ڹ������Ļص�����  
void histGram(Mat hist);//����ͼ��ֱ��ͼ
//template <class T>
MyStruct find_Max(int *arr, int N);//�õ���������ֵ�����������±�
MyStruct find_Min(int *arr, int N);//�õ��������Сֵ�����������±�
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
