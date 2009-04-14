/*
 * BD.h
 *
 *  Created on: Apr 13, 2009
 *      Author: roa
 */

#ifndef BD_H_
#define BD_H_

#include <vector>
#include <string>
#include "Page.h"

class BD {
public:
	BD(std::string filename_orig);
	virtual ~BD();
	void improve();
	void write_pdf();
private:
	std::vector <Page*> m_pages;
	std::string m_filename_orig;
	std::string m_foldername;
};

#endif /* BD_H_ */
