/*
 * Page.cpp
 *
 *  Created on: Apr 13, 2009
 *      Author: roa
 */

#include "Page.h"


#include <opencv/highgui.h>

using namespace std;

Page::Page(string filename_orig,int number,std::string foldername)
: m_filename_orig(filename_orig),m_filename_modif(""),m_number(number),m_img_orig(NULL), m_img_modif(NULL) {
	cout<<"Create page "<<filename_orig<<endl;
	m_img_orig=cvLoadImage(m_filename_orig.c_str());
	m_img_modif=cvCloneImage(m_img_orig);

	ostringstream oss_filename_modif;
	oss_filename_modif<<foldername<<"/img_"<<m_number<<".png";
	m_filename_modif=oss_filename_modif.str();
}

void Page::show_image_modif() {
	string window_name=m_filename_orig+ " modified";
	cvNamedWindow(window_name.c_str(), 0);
	cvShowImage(window_name.c_str(), m_img_modif);
	cvWaitKey(0);
	cvDestroyWindow(window_name.c_str());
}

int Page::save_image() {
	return cvSaveImage(m_filename_modif.c_str(), m_img_modif);
}

void Page::turn_image() {
	IplImage *img_gray = cvCreateImage(cvGetSize(m_img_modif), m_img_modif->depth, 1);
	IplImage *img_output = cvCloneImage(m_img_modif);//copy for display purpose only

	//Convert into grayscale
	cvConvertImage(m_img_modif, img_gray);

	IplImage *img_canny = cvCreateImage(cvGetSize(img_gray), img_gray->depth, 1);

	cvCanny(img_gray, img_canny, 500,300); //******CANNY - EDGES DETECTION

	//Creation of img_ext in which we keep only the external edges
	IplImage *img_ext = cvCreateImage(cvGetSize(img_canny), img_canny->depth, 1);

	CvScalar scalar; //scalar -  place where we stock the pixel
	//Initialize the image in black (TO IMPROVE!!!!!!!)
	for(int y=0; y<img_ext->height; y++)
	{
		for(int x=0; x<img_ext->width; x++)
		{
			//Get the pixel (x,y) from img_canny
			//scalar=cvGet2D(img_canny, y, x);
			scalar.val[0]=0;//pixels - all black
			cvSet2D(img_ext, y, x, scalar);
		}
	}

	//Only the first and the last pixels lit (true) in each line
	bool found_first, found_last;
	for(int y=0; y<img_canny->height; y++)
	{
		found_first=false;
		found_last=false;
		for(int x=0; x<img_canny->width; x++)
		{
			//Get the (x,y) pixel from img_canny
			scalar=cvGet2D(img_canny, y, x);
			//Search the first not black pixel (>0)
			if ((scalar.val[0]>0) && (!found_first)) //if the pixel is not black and if the first not black pixel was not found yet
			{
				cvSet2D(img_ext, y, x, scalar);
				found_first=true;
			}
			scalar=cvGet2D(img_canny, y,img_canny->width-x-1);
			////Search the last not black pixel (>0)
			if ((scalar.val[0]>0)&& (!found_last)) //if the pixel is not black and if the last not black pixel was not found yet
			{
				cvSet2D(img_ext, y, img_canny->width-x-1, scalar);
				found_last=true;
			}
			if ((found_first) && (found_last)) break;
		}
	}

	/*cvNamedWindow("Edges Detection", 1);
	cvShowImage("Edges Detection", img_canny);
	cvWaitKey(0);
	cvDestroyWindow("Edges Detection");

	cvNamedWindow("Only External Edges", 1);
	cvShowImage("Only External Edges", img_ext);
	cvWaitKey(0);
	cvDestroyWindow("Only External Edges");*/


	//Search lines using Hough transformation
	CvMemStorage* storage = cvCreateMemStorage(0);
	CvSeq* lines = 0;
	lines = cvHoughLines2(img_ext, storage, CV_HOUGH_PROBABILISTIC, 0.1, CV_PI/180/20, 20, 20, 10 );
	cout<<"Number of found lines: "<<lines->total<<endl;

	double mean_theta=0;
	for( int i = 0; i < lines->total; i++ )
	{
		CvPoint* line = (CvPoint*)cvGetSeqElem(lines,i);
		cvLine(img_output, line[0], line[1], CV_RGB(255,0,0), 13, 8 );
		double theta = atan ((line[1].x-line[0].x) / (double)(line[1].y-line[0].y))*180/CV_PI;
		mean_theta+=theta;
		cout<<"    Line number "<<i<<"   theta "<<theta<<endl;
	}
	mean_theta/=lines->total;
	cout<<"Mean orientation: "<<mean_theta<<endl;

	/* //Call imagemagick
	ostringstream oss;
	oss<<"convert "<<filename<<" -rotate "<<mean_theta<<" _"<<filename;
	system(oss.str().c_str());*/

	//Draw white frame before rotating to avoid dripping
	cvLine(m_img_modif, cvPoint(0,0), cvPoint(m_img_modif->width,0), CV_RGB(255,255,255));
	cvLine(m_img_modif, cvPoint(0,0), cvPoint(0,m_img_modif->height), CV_RGB(255,255,255));
	cvLine(m_img_modif, cvPoint(0,m_img_modif->height), cvPoint(m_img_modif->width,m_img_modif->height), CV_RGB(255,255,255));
	cvLine(m_img_modif, cvPoint(m_img_modif->width,0), cvPoint(m_img_modif->width,m_img_modif->height), CV_RGB(255,255,255));

	IplImage *img_turned = cvCreateImage(cvGetSize(m_img_modif), m_img_modif->depth, 3);
	float m[6];
	double factor = 1;//(cos(mean_theta*CV_PI/180.) + 1.1)*3;
	CvMat M = cvMat( 2, 3, CV_32F, m );
	int w = m_img_modif->width;
	int h = m_img_modif->height;
	cout<<"width and height----------"<<w<<"     "<<h<<endl;
	m[0] = (float)(factor*cos(mean_theta*CV_PI/180.));
	m[1] = (float)(factor*sin(mean_theta*CV_PI/180.));
	m[2] = w*0.5f;
	m[3] = -m[1];
	m[4] = m[0];
	m[5] = h*0.5f;
	cvGetQuadrangleSubPix(m_img_modif, img_turned, &M);
	cvReleaseImage(&m_img_modif);//before changing the pointer value
	m_img_modif=img_turned;//make the m_img_modif pointer to point on the turned image

	/*cvNamedWindow("rotation lines", 0);
	cvShowImage("rotation lines", img_output);
	cvWaitKey(0);
	cvDestroyWindow("rotation lines");*/

	cvReleaseImage(&img_gray);
	cvReleaseImage(&img_canny);
	cvReleaseImage(&img_output);
	cvReleaseImage(&img_ext);
}

void Page::resize_image() {
	IplImage *img_gray = cvCreateImage(cvGetSize(m_img_modif), m_img_modif->depth, 1);
	cvConvertImage(m_img_modif, img_gray);

	IplImage *img_canny = cvCreateImage(cvGetSize(img_gray), img_gray->depth, 1);
	cvCanny(img_gray,img_canny,500,300); //******CANNY - EDGES DETECTION

	int y_top=-1; //first lit pixel in y
	int y_bottom=img_canny->height; //last lit pixel in y
	int x_left=-1; //first lit pixel in x
	int x_right=img_canny->width; //last lit pixel in x
	CvScalar scalar; //scalar: place where we stock the pixel

	//Keep only the first lit pixel in y
	for(int y=0; y<img_canny->height; y++)
	{
		for(int x=0; x<img_canny->width; x++)
		{
			//Get the (x,y) pixel from img_canny
			scalar=cvGet2D(img_canny, y, x);
			//Search the first not black pixel in y
			if (scalar.val[0]>0)
			{
				y_top=y-1;
				break;
			}
		}
		if (y_top>-1) break;
	}
	cout<<"y top side: "<<y_top<<"\n";

	//Keep only the last lit pixel in y
	for(int y=img_canny->height-1;y>=0; y--)
	{
		for(int x=0; x<img_canny->width; x++)
		{
			//Get the (x,y) pixel from img_canny
			scalar=cvGet2D(img_canny, y, x);
			//Search the last not black pixel in y
			if (scalar.val[0]>0)
			{
				y_bottom=y+1;
				break;
			}
		}
		if (y_bottom<img_canny->height) break;
	}
	cout<<"y bottom side: "<<y_bottom<<"\n";


	//Keep only the first lit pixel in x
	for(int x=0; x<img_canny->width; x++)
	{
		for(int y=0;y<img_canny->height;y++)
		{
			//Get the (x,y) pixel from img_canny
			scalar=cvGet2D(img_canny, y, x);
			//Search the first not black pixel in x
			if (scalar.val[0]>0)
			{
				x_left=x-1;
				break;
			}
		}
		if (x_left>-1) break;
	}
	cout<<"x left side: "<<x_left<<"\n";

	//Keep only the last lit pixel in x
	for(int x=img_canny->width-1;x>=0; x--)
	{
		for(int y=0;y<img_canny->height;y++)
		{
			//Get the (x,y) pixel from img_canny
			scalar=cvGet2D(img_canny, y, x);
			//Search the last not black pixel in x
			if (scalar.val[0]>0)
			{
				x_right=x+1;
				break;
			}
		}
		if (x_right<img_canny->width) break;
	}
	cout<<"x right side: "<<x_right<<"\n";

	//Drawing lines to show the limits
	IplImage *img_output = cvCloneImage(m_img_modif);
	cvLine(img_output, cvPoint(0,y_top), cvPoint(img_output->width,y_top), CV_RGB(0,0,255),1);
	cvLine(img_output, cvPoint(0,y_bottom), cvPoint(img_output->width,y_bottom), CV_RGB(0,0,255),1);
	cvLine(img_output, cvPoint(x_left,0), cvPoint(x_left,img_output->height), CV_RGB(0,0,255),1);
	cvLine(img_output, cvPoint(x_right,0), cvPoint(x_right,img_output->height), CV_RGB(0,0,255),1);

	/*cvNamedWindow("Limits Marks", 1);
	cvShowImage("Limits Marks", img_output);
	cvWaitKey(0);
	cvDestroyWindow("Limits Marks");*/

	//Resized image
	int width_fin=x_right-x_left;
	int height_fin=y_bottom-y_top;
	//cout<<"height of final im: "<<height_fin<<endl;
	//cout<<"width of final im: "<<width_fin<<endl;
	CvSize size_im_fin;
	size_im_fin = cvSize(width_fin, height_fin);
	IplImage *resized_image = cvCreateImage(size_im_fin, m_img_modif->depth, 3);

	for(int y=y_top; y<y_bottom; y++)
	{
		for(int x=x_left; x<x_right; x++)
		{
			scalar=cvGet2D(m_img_modif, y, x);
			cvSet2D(resized_image, y-y_top, x-x_left, scalar);
		}
	}

	cvReleaseImage(&m_img_modif);
	m_img_modif=resized_image;

	//Release the IplImage (give to cvReleaseImage an IplImage** parameter).
	cvReleaseImage(&img_gray);
	cvReleaseImage(&img_canny);
	cvReleaseImage(&img_output);
}

void Page::write_latex(ofstream &latex_file) {
	latex_file<<"\\begin{figure}"<<endl;
	latex_file<<"\\begin{center}"<<endl;
	latex_file<<"	\\includegraphics[height = 29.3cm]{./"<<m_filename_modif<<"}"<<endl;
	latex_file<<"\\end{center}"<<endl;
	latex_file<<"\\end{figure}"<<endl;
	latex_file<<"\\clearpage"<<endl<<endl;
}

Page::~Page() {
	cout<<"Destroy page"<<endl;
	cvReleaseImage(&m_img_orig);
	cvReleaseImage(&m_img_modif);
}
