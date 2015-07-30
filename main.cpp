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
Mat g_PTh, g_PType, g_PBuffer, g_PBigSmall;
Mat g_type;
Mat g_thresold;
int g_dg = 8, g_di;//Trackbarλ�ò���
bool rgb = false;
float g_avgACBC;
//Canny��Ե�����ر���  
Mat g_cannyDetectedEdges;
int g_cannyLowThreshold = 10;//TrackBarλ�ò���  
Mat gray;
//YC��Ե�����ر���
Mat g_listKS;//��Ŵ˷�����Եͼ
Mat g_imgFlg;//ͼ���־λ
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

vector<Point2i> g_missedPt;
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

//Mat ltedge(g_listk.rows, g_listk.cols, CV_8SC2, Scalar(0, 0));
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
bool real_edge = 0, virtual_edge = 0, real_fill = 0, bufill = 0, green = 0, xls = 0, init_edge = 0, noice = 0;

//ͼ��8�����±�仯����
int a8i[] = { 0, -1, -1, -1, 0, 1, 1, 1, 0 };
int a8j[] = { 1, 1, 0, -1, -1, -1, 0, 1, 1 };
//��������������
int a5[] = { 2, 1, 0, -1, -2 };
Mat bigThanTh;
int N1, N2;

int TH1=1;
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
	pfindStart = findStartPt3;
	//load srcImage
	g_srcImage = imread(".\\sourcePicture\\test/1.jpg"); //noicePicture 2018 poisson.jpg saltpepper.bmp edgenoice.bmp gaussian.bmp
	//load success or not
	if (!g_srcImage.data)
	{
		cout << "��ȡͼƬsrcImageʱ����\n";
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
		cout << "��ȡͼƬgtImageʱ����\n";
		return false;
	}

#endif

	//outxls(g_gtImg, "output/biedge.xls");
	//blur(g_srcImage, g_srcImage, Size(5, 5));
	//����һ��Mat���Ͳ������趨ROI����  
	//g_srcImage = g_srcImage(Rect(0, 0, 50, 15));



	//һ��Ĭ�ϰѲ�ɫͼ��ת��Ϊ�Ҷ�ͼ��----------------------------------
	//*/
	cvtColor(g_srcImage, g_srcGrayImage, CV_BGR2GRAY);
	//imshow("YUV",g_srcGrayImage);
	int chan=g_srcGrayImage.channels();
	Mat YUV_channel;
	//split(g_srcGrayImage, YUV_channel);
	//Mat Y_channel = YUV_channel.at(1);
	Edge edge(g_srcGrayImage);//��ɫ��ֵ��ʼ��
	//cout << "tes=" << typeid((int)g_srcGrayImage.at<uchar>(10,10)).name() << endl;
	//*/
	//����ȡ��ע������������ʵ�ֲ�ɫͼ����--------------------------
	//Edge edge(g_srcImage);//��ɫ��ֵ��ʼ��




	//outXls("./output/maxgrad.xls", g_maxGrad, "float");
	//outXls("./output/mm.xls", maxminD, "float");
	//outXls("./output/opmm.xls", opmaxminD, "float");


	//namedWindow("g_mergImg", CV_WINDOW_NORMAL);//, CV_WINDOW_NORMAL
	//createTrackbar("�ݶȣ�", "g_mergImg", &TH, 80, edge.edgeDection);	

	//createTrackbar("���ƶȣ�", "g_mergImg", &TH1, 240, edge.edgeDection);
	//createTrackbar("���ƶȣ�", "g_edge[1]", &g_di, 80, edge.listK);

	//deNoice(gray.rows, gray.cols);
    
	for (int m = 1; m < NPT; m++)
	{
		
		edge.Init();//�����ʼ��
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


	//����Canny����tracebar
	namedWindow("canny");
	//namedWindow("listk1"); namedWindow("listk1&&listk2");
	createTrackbar("����ֵ��", "canny", &g_cannyLowThreshold, 160, edge.on_Canny);

	outXls("./output/gray.xls", g_srcGrayImage, "float");
	
	//outXls("./output/ntype.xls", g_nType, "int");
	if (MIXGAT == 1)
	{
		outXls("./output/mixNtype.xls", g_type, "float");
	}

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