/**********************
 *    retouche BD    *
 **********************/

//Nous allons utiliser les sous-librairies suivantes :
//"cv.h" pour gérer un objet "image",
//"highgui.h" pour le charger depuis un fichier et l'afficher.
 
#include <iostream>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <math.h>
#include <sstream>

using namespace std;


int main()
{
	string nom_fich="image_bd2.jpg";
    IplImage *img=cvLoadImage(nom_fich.c_str());
    IplImage *img_nvg = cvCreateImage(cvGetSize(img), img->depth, 1);
    IplImage *img_sortie = cvCloneImage(img);
    
    //Convertir en niveaux de gris :
	cvConvertImage(img, img_nvg);

    //Creation de img_canny : même taille que img, même profondeur aussi, mais 1 canal
	IplImage *img_canny = cvCreateImage(cvGetSize(img_nvg), img_nvg->depth, 1);
        
    cvCanny(img_nvg, img_canny,500,300); //******CANNY - DETECTION DES BORDS
    
    //Creation d'une image - on garde que le contour exterieur
    IplImage *img_ext = cvCreateImage(cvGetSize(img_nvg), img_nvg->depth, 1);
    
    CvScalar scalaire; //scalaire - l'endroit ou on va stocker un pixel
	
	//initialise l'image en noir (a ameliorer !)
	for(int y=0; y<img_canny->height; y++)
	{
    	for(int x=0; x<img_canny->width; x++)
    	{
            //On recupere le pixel (x,y) de l'image img_canny.
        	scalaire=cvGet2D(img_canny, y, x);
            scalaire.val[0]=0;//on met tous les pixels en noir
        	cvSet2D(img_ext, y, x, scalaire);
        }
	}
	
	//on ne garde que le premier et le dernier pixel allumes pour chaque ligne
	bool trouve_premier, trouve_dernier;
	for(int y=0; y<img_canny->height; y++)
	{
		trouve_premier=false;
		trouve_dernier=false;
    	for(int x=0; x<img_canny->width; x++)
    	{
            //On recupere le pixel (x,y) de l'image img_canny.
        	scalaire=cvGet2D(img_canny, y, x);
            //on cherche le premier pixel pas noir (>0)
        	if ((scalaire.val[0]>0) && (!trouve_premier)) //si le pixel n'est pas noir et si le premier pas noir n'a pas ete trouve
        	{
        		cvSet2D(img_ext, y, x, scalaire);
        		trouve_premier=true;
        	}
        	scalaire=cvGet2D(img_canny, y,img_canny->width-x-1);
            //on cherche le dernier pixel pas noir (>0)
        	if ((scalaire.val[0]>0)&& (!trouve_dernier))
        	{
        		cvSet2D(img_ext, y, img_canny->width-x-1, scalaire);
        		trouve_dernier=true;
        	}
        	if ((trouve_premier) && (trouve_dernier)) break;

    	}
	}
	
	//recherche de lignes par transformee de Hough 
	CvMemStorage* storage = cvCreateMemStorage(0);
	CvSeq* lines = 0;
	lines = cvHoughLines2(img_ext, storage, CV_HOUGH_PROBABILISTIC, 0.1, CV_PI/180/20, 50, 200, 50 );
	cout<<"Nombre de lignes trouvees  : "<<lines->total<<endl;
	
	double moy_theta=0;
	for( int i = 0; i < lines->total; i++ )
	{
		CvPoint* line = (CvPoint*)cvGetSeqElem(lines,i);
		cvLine( img_sortie, line[0], line[1], CV_RGB(255,0,0), 13, 8 );
		double theta = atan ((line[1].x-line[0].x) / (double)(line[1].y-line[0].y))*180/CV_PI;
		moy_theta+=theta;
		cout<<"    ligne numero "<<i<<"   theta "<<theta<<endl;
	}
	moy_theta/=lines->total;
	cout<<"theta moyen "<<moy_theta<<endl;
 
 	//appel imagemagick
 	ostringstream oss;
 	oss<<"convert "<<nom_fich<<" -rotate "<<moy_theta<<" _"<<nom_fich;
	system(oss.str().c_str());
 	
 	cvLine( img, cvPoint(0,0), cvPoint(img->width,0), CV_RGB(255,255,255));
 	cvLine( img, cvPoint(0,0), cvPoint(0,img->height), CV_RGB(255,255,255));
 	cvLine( img, cvPoint(0,img->height), cvPoint(img->width,img->height), CV_RGB(255,255,255));
 	cvLine( img, cvPoint(img->width,0), cvPoint(img->width,img->height), CV_RGB(255,255,255));

 	IplImage *img_tourne = cvCreateImage(cvGetSize(img), img->depth, 3);
 	float m[6];
	double factor = 1;//(cos(moy_theta*CV_PI/180.) + 1.1)*3;
	CvMat M = cvMat( 2, 3, CV_32F, m );
	int w = img->width;
	int h = img->height;
	m[0] = (float)(factor*cos(moy_theta*CV_PI/180.));
	m[1] = (float)(factor*sin(moy_theta*CV_PI/180.));
	m[2] = w*0.5f;
	m[3] = -m[1];
	m[4] = m[0];
	m[5] = h*0.5f;
	cvGetQuadrangleSubPix( img, img_tourne, &M);//, 1, cvScalarAll(0));
 	
 	cvSaveImage("toto.jpg",img_tourne);
 	
//-------------------------------------------------------------------------------------
 	IplImage *img_nvg_tourne = cvCreateImage(cvGetSize(img_tourne), img_tourne->depth, 1);
    cvConvertImage(img_tourne, img_nvg_tourne);
 	//Creation de img_canny_tourne : meme taille que img_tourne, meme profondeur aussi, mais 1 canal
	IplImage *img_canny_tourne = cvCreateImage(cvGetSize(img_nvg_tourne), img_nvg_tourne->depth, 1);
    cvCanny(img_nvg_tourne, img_canny_tourne,500,300); //******CANNY - DETECTION DES BORDS

    /*cvNamedWindow("Retouche BD", 1);
    cvShowImage("Retouche BD", img_canny_tourne);
    cvWaitKey(0);
    cvDestroyWindow("Retouche BD");
    */
    
	//on ne garde que le premier pixel allume en y
	int coord_bord_haut=-1;
	for(int y=0; y<img_canny_tourne->height; y++)
	{
		for(int x=0; x<img_canny_tourne->width; x++)
		{
			//On recupere le pixel (x,y) de l'image img_canny_tourne
			scalaire=cvGet2D(img_canny_tourne, y, x);
			//on cherche le premier pixel pas noir (>0)
			if (scalaire.val[0]>0) //si le pixel n'est pas noir
			{
				coord_bord_haut=y-1;
				break;
			}
		}
		if (coord_bord_haut>-1) break;
	}
    cout<<"bord haut : "<<coord_bord_haut<<"\n";
	//on connait maintenant le y du bord haut de l'image
 	

 	//on ne garde que le dernier pixel allume en y
	int coord_bord_bas=img_canny_tourne->height;
	for(int y=img_canny_tourne->height-1;y>=0; y--)
	{
		for(int x=0; x<img_canny_tourne->width; x++)
		{
			//On recupere le pixel (x,y) de l'image img_canny_tourne
			scalaire=cvGet2D(img_canny_tourne, y, x);
			//on cherche le premier pixel pas noir (>0)
			if (scalaire.val[0]>0) //si le pixel n'est pas noir
			{
				coord_bord_bas=y+1;
				break;
			}
		}
		if (coord_bord_bas<img_canny_tourne->height) break;
	}
    cout<<"bord bas : "<<coord_bord_bas<<"\n";
	//on connait maintenant le y du bord bas de l'image
	
	//on ne garde que le premier pixel allume en x
	int coord_bord_gauche=-1;
    for(int x=0; x<img_canny_tourne->width; x++)
	{
        for(int y=0;y<img_canny_tourne->height;y++)
		{
			//On recupere le pixel (x,y) de l'image img_canny_tourne
			scalaire=cvGet2D(img_canny_tourne, y, x);
			//on cherche le premier pixel pas noir (>0)
			if (scalaire.val[0]>0) //si le pixel n'est pas noir
			{
				coord_bord_gauche=x-1;
				break;
			}
		}
		if (coord_bord_gauche>-1) break;
	}
    cout<<"bord gauche : "<<coord_bord_gauche<<"\n";
	//on connait maintenant le x du bord gauche de l'image
	
    //on ne garde que le dernier pixel allume en x
	int coord_bord_droite=img_canny_tourne->width;
    for(int x=img_canny_tourne->width-1;x>=0; x--)
	{
        for(int y=0;y<img_canny_tourne->height;y++)
		{
			//On recupere le pixel (x,y) de l'image img_canny_tourne
			scalaire=cvGet2D(img_canny_tourne, y, x);
			//on cherche le premier pixel pas noir (>0)
			if (scalaire.val[0]>0) //si le pixel n'est pas noir
			{
				coord_bord_droite=x+1;
				break;
			}
		}
		if (coord_bord_droite<img_canny_tourne->width) break;
	}
    cout<<"bord droite : "<<coord_bord_droite<<"\n";
	//on connait maintenant le x du bord droit de l'image
	
	
	IplImage *img_sortie_tourne = cvCloneImage(img_tourne);
	cvLine( img_sortie_tourne, cvPoint(0,coord_bord_haut), cvPoint(img->width,coord_bord_haut), CV_RGB(0,0,255),1);
	cvLine( img_sortie_tourne, cvPoint(0,coord_bord_bas), cvPoint(img->width,coord_bord_bas), CV_RGB(0,0,255),1);
	cvLine( img_sortie_tourne, cvPoint(coord_bord_gauche,0), cvPoint(coord_bord_gauche,img->height), CV_RGB(0,0,255),1);
	cvLine( img_sortie_tourne, cvPoint(coord_bord_droite,0), cvPoint(coord_bord_droite,img->height), CV_RGB(0,0,255),1);
 	

    cvNamedWindow("Retouche BD", 1);
    cvShowImage("Retouche BD", img_sortie_tourne);
    cvWaitKey(0);
    cvDestroyWindow("Retouche BD");
    
    
    //Libération de l'IplImage (on lui passe un IplImage**).
    cvReleaseImage(&img);
 	cvReleaseImage(&img_nvg);
    cvReleaseImage(&img_canny);
    cvReleaseImage(&img_ext);
 	
    return 0;

}
