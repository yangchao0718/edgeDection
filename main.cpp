// ---------------------------------- - ������˵����----------------------------------------------
//      ��������:OpenCV��Ե��⣺�·���ʵ�ֱ�Ե������ȡ���ܶ�λ����
//      ��������IDE�汾��Visual Studio 2013  
//      <span style="white-space:pre">  </span>��������OpenCV�汾��    2.4.9  
//      2014��11��19�� Create by �  
//    
//----------------------------------------------------------------------------------------------  
#include "edge.h"  
#include"stdlib.h"
#include "chistogram.h"
#include "outXls.h"
#include "structdef.h"

using namespace std;
using namespace cv;

//-----------------------------------��ȫ�ֱ����������֡�--------------------------------------  
//      ������ȫ�ֱ�������  
//-----------------------------------------------------------------------------------------------  
//ԭͼ��ԭͼ�ĻҶȰ棬Ŀ��ͼ  
Mat g_srcImage, g_srcGrayImage, g_dstImage;
MyStruct mmi, mmik, man;
//�ݶ�ͼ
Mat g_srcGrad, g_outEdge;
Mat g_medImg;//��ֵ�˲�
Mat g_mixGS;
Mat g_PTh, g_PType, g_PBuffer, g_PBigSmall,g_PNdifference,g_PPdifference,g_PNumbOfS;
Mat g_type;
Mat g_thresold;
int g_dg = 8, g_di;//Trackbarλ�ò���
bool rgb = false;
float g_avgACBC;
//Canny��Ե�����ر���  
Mat g_cannyDetectedEdges;
int g_cannyLowThreshold = 10;//TrackBarλ�ò���  
Mat gray;
Mat g_listKS;//��Ŵ˷�����Եͼ
Mat g_imgFlg,g_imgFlg2,imgFlg3;//ͼ���־λ
Mat g_nSectionFlg;//���򻮷ִ�����
Mat g_calImg;//ת���˳��˸����������͵ĻҶ�ֵ
Mat g_calImg_I;
Mat g_angle;//������
Mat g_intAngle;//�ѷ�����ͳһ��8��������
Mat g_gradBmp;//�ݶ�ͼ��
Mat g_nType;//���������Ե���Ǽ����͵ģ����ͣ�
Mat g_typeEdge;//�������ͱ�Եͼ
Mat g_biGrad, g_dstBiGrad;
int g_numPixel;
Mat g_circle;
vector<Point2i> disappearedPoint,triBranchPoint;
vector<Mat> g_edge(3);
vector<Mat> g_srcImageChannels, g_srcImageChannels_canny;//����ԭͼ��ͨ��
vector<Mat> srcBGR(3);
Mat g_mergImg;//�ϲ���ɫ��Ե
Mat g_srcImageBGR, g_srcImageBGR_canny;//��ϱ�Եͨ�����ͼ��
Mat g_srcImageB, g_srcImageG, g_srcImageR;
Mat g_gtImg;
Mat g_nTypeFlag;
Mat g_dedgeX, g_dedgeY;//����Ե��λ��
Mat g_inedgeX, g_inedgeY;//��ʾ��Ե��λ��
vector<Mat> nTypeEdge(3);
Mat_ <Vec3b> img(240, 320, Vec3b(0, 255, 0));//Mat_ ΪMat��ģ������
float temp = 0;
vector<float> tempB_t, tempB, tempS;//_t��ʾ��ʱ��ű���
int tempM[8] = { 0 };//����ѭ������ÿ���е����������ͬ�ĸ���
int md_t = 0, md;
int module;//ģ���С����
float dsm, grd;//dsmΪ��ֵ��ԭͼ���ĵ��ֵ��grd�ݶ�ֵ
float tempGrad[8] = { 0 };
vector<Point2i> arrP;
Mat g_minGrad, g_maxGrad;
Mat g_grad0, g_grad1, g_grad2, g_grad3, g_grad4, g_grad5, g_grad6, g_grad7, g_grad8;
vector<startPT> S37,S5;
int m1 = 0, m2 = 0;
//ͼ��8�����±�仯����
int a8i[] = { 0, -1, -1, -1, 0, 1, 1, 1, 0 };
int a8j[] = { 1, 1, 0, -1, -1, -1, 0, 1, 1 };
//��������������
int a5[] = { 2, 1, 0, -1, -2 };
Mat bigThanTh;
int N1, N2;
int stretchTime;
int TH1=20;
int calTime;
bool find7;
int TH = 0, TH2 = 2 * TH1, TH_D;
int n1 = 0, n2 = 0;
int cnt = 0;
int gTH = 50, nGrad = 8;//gTHΪ�ݶ���ֵ��nGrad Ϊ5*5ģ���С�gTH�ĸ���
int bluePt;
int startPt;
int nTH;
int g_temth[8] = { 0 };
Mat maxminD, opmaxminD;
bool(*pfindStart)(Point2i centerPt, Point2i &A, Point2i &B, Point2i &AC, Point2i &BC, int &direction, bool & min_max);//declare a function point

int main(int agrc, char** agrv)
{
	

	initMain();//first inital
	//load srcImage
	g_srcImage = imread(".\\sourcePicture\\test/1.jpg"); //CONTRADICTIONPicture 2018 poisson.jpg saltpepper.bmp edgeCONTRADICTION.bmp gaussian.bmp
	//load success or not
	if (!g_srcImage.data)
	{
		cout << "��ȡͼƬsrcImageʱ����\n";
		return false;
	}

	//load GT image
	g_gtImg = imread(".\\sourcePicture\\gtimage\\1.png");

	//use filter
	//blur(g_srcImage, g_srcImage, Size(5, 5));

	//����һ��Mat���Ͳ������趨ROI����  
	//g_srcImage = g_srcImage(Rect(0, 0, 50, 15));



	//һ��Ĭ�ϰѲ�ɫͼ��ת��Ϊ�Ҷ�ͼ��----------------------------------
	cvtColor(g_srcImage, g_srcGrayImage, CV_BGR2GRAY);
	Edge edge(g_srcGrayImage);//��ɫ��ֵ��ʼ��

	//����ȡ��ע������������ʵ�ֲ�ɫͼ����--------------------------
	//Edge edge(g_srcImage);//��ɫ��ֵ��ʼ��

	for (int m = 1; m < NPT; m++)
	{
		
		edge.Init();//�����ʼ��
		//showHistogram(g_minGrad);
		namedWindow("th");
		imwrite("./output/th.png", g_PTh);
		cout << "the " << m << " th:" << TH1 << endl;
		nTH = m;
		//edge.edgeDection();
		edge.edgeDetection3();

		TH1 = TH1 + 2;
		
	}
	/*outXls("./output/02 g_PType.xls", g_PType, "int");
	outXls("./output/bigThanTh.xls", bigThanTh, "int");*/

	cout << "g_numPixel=" << g_numPixel << endl;
	g_srcGrayImage.convertTo(g_srcGrayImage, CV_32F);


	//����Canny����tracebar
	namedWindow("canny");
	createTrackbar("����ֵ��", "canny", &g_cannyLowThreshold, 160, edge.on_Canny);

	outXls("./output/gray.xls", g_srcGrayImage, "float");

	outXls("./output/ntype.xls", g_PType, "int");
	outXls("./output/numPN.xls", g_PNdifference, "int");
	outXls("./output/numPP.xls", g_PPdifference, "int");
	outXls("./output/numPS.xls", g_PNumbOfS, "int");
	outXls("./output/imFlag.xls", g_imgFlg, "int");
	outXls("./output/imgFlag2.xls", g_imgFlg2, "int");
	g_maxGrad.convertTo(g_gradBmp, CV_8UC1);
	imshow("�ݶ�ͼ", g_gradBmp);
	imwrite("./output/grad.png", g_gradBmp);

	//showHistogram(g_maxGrad, "maxGrad");
	//showHistogram(g_PTh, "TH");
	//��ѯ��ȡ������Ϣ��������Q�������˳�  
	while ((char(waitKey(1)) != 'q'))
	{
	}
	return 0;
}

