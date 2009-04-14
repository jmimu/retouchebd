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


IplImage *turn_image(IplImage *img)
{
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
		cvLine( img_output, line[0], line[1], CV_RGB(255,0,0), 13, 8 );
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
 	
 	cvReleaseImage(&img_gray);
    cvReleaseImage(&img_canny);
    cvReleaseImage(&img_ext);
 	
 	return img_turned;
}

IplImage *resize_image(IplImage *img)
{
	
	IplImage *img_gray = cvCreateImage(cvGetSize(img), img->depth, 1);
    cvConvertImage(img, img_gray);
 	//Creation of img_canny : same size and depth as img_gray, but only one channel
	IplImage *img_canny = cvCreateImage(cvGetSize(img_gray), img_gray->depth, 1);
    cvCanny(img_gray, img_canny,500,300); //******CANNY - EDGES DETECTION
    
    int y_top=-1;
    int y_bottom=img->height;
    int x_left=-1;
    int x_right=img->width;
    CvScalar scalar; //scalar -  place where we stock the pixel
     
	//Keep only the first lit pixel in y
	for(int y=0; y<img->height; y++)
	{
		for(int x=0; x<img->width; x++)
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
 	for(int y=img->height-1;y>=0; y--)
	{
		for(int x=0; x<img->width; x++)
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
		if (y_bottom<img->height) break;
	}
    cout<<"y bottom side: "<<y_bottom<<"\n";
	
	
	//Keep only the first lit pixel in x
	for(int x=0; x<img->width; x++)
	{
        for(int y=0;y<img->height;y++)
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
	for(int x=img->width-1;x>=0; x--)
	{
        for(int y=0;y<img->height;y++)
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
		if (x_right<img->width) break;
	}
    cout<<"x right side: "<<x_right<<"\n";
    
    //Drawing lines to show the limits
    IplImage *img_output = cvCloneImage(img);
	cvLine( img_output, cvPoint(0,y_top), cvPoint(img->width,y_top), CV_RGB(0,0,255),1);
	cvLine( img_output, cvPoint(0,y_bottom), cvPoint(img->width,y_bottom), CV_RGB(0,0,255),1);
	cvLine( img_output, cvPoint(x_left,0), cvPoint(x_left,img->height), CV_RGB(0,0,255),1);
	cvLine( img_output, cvPoint(x_right,0), cvPoint(x_right,img->height), CV_RGB(0,0,255),1);
    
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
	IplImage *resized_image = cvCreateImage(size_im_fin, img->depth, 3);
		
	for(int y=y_top; y<y_bottom; y++)
	{
    	for(int x=x_left; x<x_right; x++)
    	{
            scalar=cvGet2D(img, y, x);
        	cvSet2D(resized_image, y-y_top, x-x_left, scalar);
        }
	}
	
	//Release the IplImage (give to cvReleaseImage an IplImage** parameter).
    cvReleaseImage(&img_gray);
    cvReleaseImage(&img_canny);
    cvReleaseImage(&img_output);
	
	
	return resized_image;
}

//////////////////////////////////////////////////////////////////////////////////////

int main()
{
	string filename = "geekscottes_105.png";
    IplImage *img = cvLoadImage(filename.c_str());
    
    IplImage *img_turned = turn_image(img);
  
    cvNamedWindow("Turned Image (main)", 0);
    cvShowImage("Turned Image (main)", img_turned);
    cvWaitKey(0);
    cvDestroyWindow("Turned Image (main)");
    
    IplImage *resized_im = resize_image(img_turned);
	 		
    cvNamedWindow("Retouche BD (main)", 0);
    cvShowImage("Retouche BD (main)", resized_im);
    cvWaitKey(0);
    cvDestroyWindow("Retouche BD (main)");
  
    cvSaveImage("toto.png", resized_im);
     	
   	cvReleaseImage(&img);
 	cvReleaseImage(&img_turned);
    cvReleaseImage(&resized_im);
   	
    return 0;

}
