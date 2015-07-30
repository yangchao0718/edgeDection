#include <opencv2/highgui/highgui.hpp>  
#include <opencv2/imgproc/imgproc.hpp>  
#include "highgui.h"  
#include <cv.h>  
#include <cvaux.h>
#include <iostream>
#include <fstream>
using namespace std;





//-----------------------------------���궨�岿�֡�--------------------------------------------
//	����������һЩ������
//------------------------------------------------------------------------------------------------
//#define TH1 10//�̶��ı�Ե�㼸�ζ��嶨����ֵ
#define LONG 6
#define STEP 10//�����䳤��
#define BLUE_FLAG 0  //��0��ʾ��ʾһ�໺������




using namespace cv;


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
	static void edgeDection();//���ı�Ե��ⷽ������
	static void edgeDection1();
	static void on_Canny(int, void*);//Canny���
	void outXls(Mat outxls, string filename);//��Ե���еĵ�����Excel�ĺ���
private:
	Mat src;
	Mat gray;
	Mat dst;
	//Mat listK1, listK2;
};

//���ڱ��������Сֵ�����Ӧ���±�ṹ��
typedef struct MyStruct
{
	int m_num;
	int m_index;
}
MinMaxIndex;

//���ýṹ�����
typedef struct MyPoint
{
	float x;
	float y;

}
PT;

//-----------------------------------��ȫ�ֱ����������֡�--------------------------------------  
//      ������ȫ�ֱ�������  
//-----------------------------------------------------------------------------------------------  
//ԭͼ��ԭͼ�ĻҶȰ棬Ŀ��ͼ  
Mat g_srcImage, g_srcGrayImage, g_dstImage;

//�ݶ�ͼ
Mat g_srcGrad, g_outEdge;
Mat g_medImg;//��ֵ�˲�
Mat g_thresold;
int g_dg = 8, g_di;//Trackbarλ�ò���
Mat g_listk;

//Canny��Ե�����ر���  
Mat g_cannyDetectedEdges;
int g_cannyLowThreshold = 1;//TrackBarλ�ò���  

//YC��Ե�����ر���
Mat g_listKS;//��Ŵ˷�����Եͼ
Mat g_imgFlg;//ͼ���־λ
Mat g_calImg;//ת���˳��˸����������͵ĻҶ�ֵ
Mat g_angle;//������
Mat g_intAngle;//�ѷ�����ͳһ��8��������
Mat g_gradBmp;//�ݶ�ͼ��
Mat g_nType;//���������Ե���Ǽ����͵ģ����ͣ�
Mat g_typeEdge;//�������ͱ�Եͼ
Mat g_biGrad, g_dstBiGrad;

Mat g_kernel = (Mat_<float>(5, 5) << 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1,
	1, 1, 1, 1, 1,
	1, 1, 1, 1, 1,
	1, 1, 1, 1, 1);//�˺���

//Mat g_kernel = (Mat_<float>(7, 7) << 1, 1, 1, 1, 1, 1, 1,
//	1, 1, 1, 1, 1, 1, 1,
//	1, 1, 1, 1, 1, 1, 1,
//	1, 1, 1, 1, 1, 1, 1,
//	1, 1, 1, 1, 1, 1, 1);//�˺���


vector<Mat> g_edge(3);
vector<Mat> g_srcImageChannels;//����ԭͼ��ͨ��

Mat g_mergImg;//�ϲ���ɫ��Ե
Mat g_srcImageBGR;//��ϱ�Եͨ�����ͼ��
Mat g_srcImageB, g_srcImageG, g_srcImageR;

Mat g_dedgeX, g_dedgeY;//����Ե��λ��
Mat g_inedgeX, g_inedgeY;//��ʾ��Ե��λ��

//Mat ltedge(g_listk.rows, g_listk.cols, CV_8SC2, Scalar(0, 0));
Mat_ <Vec3b> img(240, 320, Vec3b(0, 255, 0));//Mat_ ΪMat��ģ������
float temp = 0;
vector<float> tempB_t, tempB, tempS;//_t��ʾ��ʱ��ű���
int tempM[8] = { 0 };//����ѭ������ÿ���е����������ͬ�ĸ���
int md_t = 0, md;
int seq_num = 0;//������ͬ��������
int start_pt;
int module;//ģ���С����
int g_tracenum;//save the number of tracing by testing
MinMaxIndex mmi, mmik, man;//���庯������������ֵ
float dsm, grd;//dsmΪ��ֵ��ԭͼ���ĵ��ֵ��grd�ݶ�ֵ
float tempGrad[8] = { 0 };
vector<Point2i> arrP;
Point2i A, B, C, D;//�����ĸ�����


//ͼ��8�����±�仯����
int a8i[] = { 0, -1, -1, -1, 0, 1, 1, 1, 0 };
int a8j[] = { 1, 1, 0, -1, -1, -1, 0, 1, 1 };
//��������������
int a5[] = { 2, 1, 0, -1, -2 };


int TH = 0, TH1 = 20;
int n1 = 0, n2 = 0;
int cnt = 0;
int gTH = 50, nGrad = 8;//gTHΪ�ݶ���ֵ��nGrad Ϊ5*5ģ���С�gTH�ĸ���

int g_temth[8] = { 0 };
int g_maxth;
bool g_clk;
//YC��Ե�����غ���
//template<class T>
void outxls(Mat listK1, string filename);//�������ݵ�Excel���
void outxlsInt(Mat listK1, string filename);//�������ݵ�Excel���
MinMaxIndex findSeqNum(int i, int j);//�õ�������ͬ����Ŀ
float getAngle(int i, int seq_num);//���㷨����
float getIangle(int i, int len);//���㷨����
int getIangle(Point2i centPt, Point2i A);//
float getMedLine(int bdA, int bdB, int direction);
void TraceEdge(int x, int y, float grd);
bool clockDirection(int b, int d);
void calType(int i, int j);
void strEdge(int i, int j, int bd, int clk);
bool isSeqAngle(float nd1, float nd2);//�жϽǶ��Ƿ�����
//float cosAngle(float nd1, float nd2);//�жϽǶ��Ƿ�����,�д���
int bordDirection(int i, int j);//���Ե����
int drt1;
bool outOfrange(int i, int j);//Խ����
void fillPixel(int i, int j, float nd, int bd);
void fillAdjacent(int i, int j);
//template<class T>
Point2f drtToVector(float nd);//�ѽǶ�ת��Ϊ��������
Point2f drtToVector(int bd);
void fillRect(Point2i A, Point2i B, Point2i C, Point2i D);//����ı���
int getThresold(Point2i A, Point2i B, Point2i C);//�����������������һ����̬��ֵ
Point2i normalToXY(float nv);//�ѷ�����תΪ������ʽ
int getDth(Point2i A, Point2i B, Point2i C, Point2i D, Point2i E, int direction);
//��ӽ�������������
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
void controlFuction(bool n1, bool n2, bool n3, bool n4, bool n5, bool n6, bool n7, bool n8);//���ƺ���
void initMain();//����ͼ��ǰ�ĳ�ʼ����

//-----------------------------------�����������֡�--------------------------------------  
//      ������Edge��Ե����������  
//-----------------------------------------------------------------------------------------------  
static void ShowHelpText(Mat Image);//ϵͳ��ʾ��Ϣ
//static void on_Canny(int, void*);//Canny��Ե��ⴰ�ڹ������Ļص�����  
void histGram(Mat hist);//����ͼ��ֱ��ͼ
//template <class T>
MinMaxIndex find_Max(int *arr, int N);//�õ���������ֵ�����������±�
MinMaxIndex find_Min(int *arr, int N);//�õ��������Сֵ�����������±�
template <class T>
T findMax(T *arr, int N);
template <class T>
T findMin(T *arr, int N);
bool findSeqNumSE(Point2i A);
