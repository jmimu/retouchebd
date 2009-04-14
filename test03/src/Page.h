/*
 * Page.h
 *
 *  Created on: Apr 13, 2009
 *      Author: roa
 */

#ifndef PAGE_H_
#define PAGE_H_

#include <string>
#include <opencv/cv.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <math.h>

class Page {
public:
	Page(std::string filename_orig,int number,std::string foldername);
	virtual ~Page();
	void turn_image();
	void resize_image();
	void show_image_modif();
	int save_image();
	void write_latex(std::ofstream & latex_file);
private:
	std::string m_filename_orig;
	std::string m_filename_modif;
	int m_number;
	IplImage *m_img_orig;
	IplImage *m_img_modif;
};

#endif /* PAGE_H_ */
