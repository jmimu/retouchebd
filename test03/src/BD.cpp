/*
 * BD.cpp
 *
 *  Created on: Apr 13, 2009
 *      Author: roa
 */

#include "BD.h"

#include <sstream>
#include <iostream>
#include <stdlib.h>
#include <fstream>

using namespace std;

BD::BD(string filename_orig) : m_pages(), m_filename_orig(filename_orig), m_foldername(filename_orig+"_tmp") {
	cout<<"I am building a new BD "<<filename_orig<<endl;
	cout<<"unzip the .cbz file"<<endl;

	//Creating a folder
	ostringstream oss_folder;
	oss_folder<<"rm -rf "<<m_foldername;
	system(oss_folder.str().c_str());
	ostringstream oss;
	oss<<"unzip -j \""<<m_filename_orig<<"\" -d "<<m_foldername;
	system( oss.str().c_str() );

	//Creating a list with all the images .jpg
	string ls_command="ls "+m_foldername+"/*.jpg >"+m_foldername+"/list";
	system(ls_command.c_str());

	ifstream list_file((m_foldername+"/list").c_str(), ios::in);  // opening the file reading only

	string image_filename;
	if (list_file) // if file opening succeeded
	{
		cout<<"File "<< m_foldername+"/list"<<" found"<<endl;
		while(getline(list_file, image_filename))
		{
			//cout<<"image's name: "<<image_filename<<endl;

			m_pages.push_back(new Page(image_filename,m_pages.size()+1,m_foldername));
			//m_pages[m_pages.size()-1]->show_image_modif(); //showing each image
		}
		list_file.close(); // closing the file
	} else
		cerr << "Impossible to open the file "<<m_foldername+"/list"<< endl;

}

void BD::improve() {
	for (unsigned int i=0; i<m_pages.size(); i++)
	{
		m_pages[i]->turn_image();
		//m_pages[i]->resize_image();
	}

}

void BD::write_pdf() {
	for (unsigned int i=0; i<m_pages.size(); i++)
	{
		m_pages[i]->save_image();
	}

	string tex_filename=m_foldername+"/"+m_filename_orig+".tex";
	ofstream latex_file(tex_filename.c_str(), ios::out | ios::trunc);
	if (latex_file) // if file creation succeeded
	{
		cout<<"File "<< tex_filename<<" created"<<endl;
		latex_file<<"\\documentclass[a4paper, 12pt]{article} %article"<<endl;
		latex_file<<"\\usepackage[english, french]{babel}" <<endl;
		latex_file<<"\\usepackage[utf8]{inputenc} "<<endl;
		latex_file<<"\\usepackage{geometry}"<<endl;
		latex_file<<"\\usepackage{epsfig}"<<endl;
		latex_file<<"\\geometry{hmargin=0cm, vmargin=0cm }"<<endl;
		latex_file<<"\\begin{document}"<<endl<<endl;

		for (unsigned int i=0; i<m_pages.size(); i++)
		{
			m_pages[i]->write_latex(latex_file);
		}

		latex_file<<"\\end{document}"<<endl;

		latex_file.close(); // closing the file

		system( ("pdflatex "+tex_filename).c_str() );


	} else
		cerr << "Impossible to create the file "<<tex_filename<< endl;


}

BD::~BD() {
	cout<<"Begin destroy BD"<<endl;
	for (unsigned int i=0; i<m_pages.size(); i++)
		delete m_pages[i];
	cout<<"Remove temporary files"<<endl;
	system(("rm -r "+m_foldername).c_str());
	system(("rm "+m_filename_orig+".aux "+m_filename_orig+".log").c_str());

	cout<<"End destroy BD"<<endl;
}
