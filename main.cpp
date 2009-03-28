/*
* main.cpp
*
* Copyright 2009 JMM & Roa <jmimu@free.fr>
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
* MA 02110-1301, USA.
*/


/**********************
 *    retouche BD    *
 **********************/

#include <iostream>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <math.h>
#include <sstream>

using namespace std;


int main()
{
	string filename="image_bd2.jpg";
    IplImage *img=cvLoadImage(filename.c_str());
    IplImage *img_gray = cvCreateImage(cvGetSize(img), img->depth, 1);
    IplImage *img_output = cvCloneImage(img);
    
    //Convert into grayscale
	cvConvertImage(img, img_gray);

    //Creation of img_canny: same size and depth as img_gray, but only one channel
	IplImage *img_canny = cvCreateImage(cvGetSize(img_gray), img_gray->depth, 1);
        
    cvCanny(img_gray, img_canny,500,300); //******CANNY - EDGES DETECTION
    
    //Creation of img_ext in which we keep only the external edges
    IplImage *img_ext = cvCreateImage(cvGetSize(img_gray), img_gray->depth, 1);
    
    CvScalar scalar; //scalar -  place where we stock the pixel
	
	//Initialize the image in black (TO IMPROVE!!!!!!!)
	for(int y=0; y<img_canny->height; y++)
	{
    	for(int x=0; x<img_canny->width; x++)
    	{
            //Get the pixel (x,y) from img_canny
        	scalar=cvGet2D(img_canny, y, x);
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
	
	
	cvNamedWindow("Retouche BD", 1);
    cvShowImage("Retouche BD", img_ext);
    cvWaitKey(0);
    cvDestroyWindow("Retouche BD");
    
	
	//Search lines using Hough transformation
	CvMemStorage* storage = cvCreateMemStorage(0);
	CvSeq* lines = 0;
	lines = cvHoughLines2(img_ext, storage, CV_HOUGH_PROBABILISTIC, 0.1, CV_PI/180/20, 20, 20, 10 );
	cout<<"Number of found lines: "<<lines->total<<endl;
	
	double mean_theta=0;
	for( int i = 0; i < lines->total; i++ )
	{
		CvPoint* line = (CvPoint*)cvGetSeqElem(lines,i);
		cvLine( img_output, line[0], line[1], CV_RGB(255,0,0), 13, 8 );
		double theta = atan ((line[1].x-line[0].x) / (double)(line[1].y-line[0].y))*180/CV_PI;
		mean_theta+=theta;
		cout<<"    Line number "<<i<<"   theta "<<theta<<endl;
	}
	mean_theta/=lines->total;
	cout<<"Mean orientation: "<<mean_theta<<endl;
 
 	//Call imagemagick
 	ostringstream oss;
 	oss<<"convert "<<filename<<" -rotate "<<mean_theta<<" _"<<filename;
	system(oss.str().c_str());
 	
 	cvLine( img, cvPoint(0,0), cvPoint(img->width,0), CV_RGB(255,255,255));
 	cvLine( img, cvPoint(0,0), cvPoint(0,img->height), CV_RGB(255,255,255));
 	cvLine( img, cvPoint(0,img->height), cvPoint(img->width,img->height), CV_RGB(255,255,255));
 	cvLine( img, cvPoint(img->width,0), cvPoint(img->width,img->height), CV_RGB(255,255,255));

 	IplImage *img_turned = cvCreateImage(cvGetSize(img), img->depth, 3);
 	float m[6];
	double factor = 1;//(cos(mean_theta*CV_PI/180.) + 1.1)*3;
	CvMat M = cvMat( 2, 3, CV_32F, m );
	int w = img->width;
	int h = img->height;
	m[0] = (float)(factor*cos(mean_theta*CV_PI/180.));
	m[1] = (float)(factor*sin(mean_theta*CV_PI/180.));
	m[2] = w*0.5f;
	m[3] = -m[1];
	m[4] = m[0];
	m[5] = h*0.5f;
	cvGetQuadrangleSubPix( img, img_turned, &M);//, 1, cvScalarAll(0));
 	
 	cvSaveImage("toto.jpg",img_turned);
 	
//-------------------------------------------------------------------------------------
 	IplImage *img_gray_turned = cvCreateImage(cvGetSize(img_turned), img_turned->depth, 1);
    cvConvertImage(img_turned, img_gray_turned);
 	//Creation of img_canny_turned : same size and depth as img_gray, but only one channel
	IplImage *img_canny_turned = cvCreateImage(cvGetSize(img_gray_turned), img_gray_turned->depth, 1);
    cvCanny(img_gray_turned, img_canny_turned,500,300); //******CANNY - EDGES DETECTION

    /*cvNamedWindow("Retouche BD", 1);
    cvShowImage("Retouche BD", img_canny_turned);
    cvWaitKey(0);
    cvDestroyWindow("Retouche BD");
    */
    
	//Keep only the first lit pixel in y
	int top_side_coord=-1;
	for(int y=0; y<img_canny_turned->height; y++)
	{
		for(int x=0; x<img_canny_turned->width; x++)
		{
			//Get the (x,y) pixel from img_canny_turned
			scalar=cvGet2D(img_canny_turned, y, x);
			//Search the first not black pixel in y
			if (scalar.val[0]>0) 
			{
				top_side_coord=y-1;
				break;
			}
		}
		if (top_side_coord>-1) break;
	}
    cout<<"y top side: "<<top_side_coord<<"\n";
	//on connait maintenant le y du bord haut de l'image
 	
	//Keep only the last lit pixel in y
 	int bottom_side_coord=img_canny_turned->height;
	for(int y=img_canny_turned->height-1;y>=0; y--)
	{
		for(int x=0; x<img_canny_turned->width; x++)
		{
			//Get the (x,y) pixel from img_canny_turned
			scalar=cvGet2D(img_canny_turned, y, x);
			//Search the last not black pixel in y
			if (scalar.val[0]>0) 
			{
				bottom_side_coord=y+1;
				break;
			}
		}
		if (bottom_side_coord<img_canny_turned->height) break;
	}
    cout<<"y bottom side: "<<bottom_side_coord<<"\n";
	
	
	//Keep only the first lit pixel in x
	int left_side_coord=-1;
    for(int x=0; x<img_canny_turned->width; x++)
	{
        for(int y=0;y<img_canny_turned->height;y++)
		{
			//Get the (x,y) pixel from img_canny_turned
			scalar=cvGet2D(img_canny_turned, y, x);
			//Search the first not black pixel in x
			if (scalar.val[0]>0) 
			{
				left_side_coord=x-1;
				break;
			}
		}
		if (left_side_coord>-1) break;
	}
    cout<<"x left side: "<<left_side_coord<<"\n";
		
    //Keep only the last lit pixel in y
	int right_side_coord=img_canny_turned->width;
    for(int x=img_canny_turned->width-1;x>=0; x--)
	{
        for(int y=0;y<img_canny_turned->height;y++)
		{
			//Get the (x,y) pixel from img_canny_turned
			scalar=cvGet2D(img_canny_turned, y, x);
			//Search the last not black pixel in x
			if (scalar.val[0]>0) 
			{
				right_side_coord=x+1;
				break;
			}
		}
		if (right_side_coord<img_canny_turned->width) break;
	}
    cout<<"x right side: "<<right_side_coord<<"\n";
	
	
	IplImage *img_output_turned = cvCloneImage(img_turned);
	cvLine( img_output_turned, cvPoint(0,top_side_coord), cvPoint(img->width,top_side_coord), CV_RGB(0,0,255),1);
	cvLine( img_output_turned, cvPoint(0,bottom_side_coord), cvPoint(img->width,bottom_side_coord), CV_RGB(0,0,255),1);
	cvLine( img_output_turned, cvPoint(left_side_coord,0), cvPoint(left_side_coord,img->height), CV_RGB(0,0,255),1);
	cvLine( img_output_turned, cvPoint(right_side_coord,0), cvPoint(right_side_coord,img->height), CV_RGB(0,0,255),1);
 	

    cvNamedWindow("Retouche BD", 1);
    cvShowImage("Retouche BD", img_output_turned);
    cvWaitKey(0);
    cvDestroyWindow("Retouche BD");
    
    
    //Release the IplImage (give to cvReleaseImage an IplImage** parameter).
    cvReleaseImage(&img);
 	cvReleaseImage(&img_gray);
    cvReleaseImage(&img_canny);
    cvReleaseImage(&img_ext);
 	
    return 0;

}
