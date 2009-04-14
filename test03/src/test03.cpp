//============================================================================
// Name        : test03.cpp
// Author      : AM
// Version     :
// Copyright   : Your copyright notice
// Description : test in C++, Ansi-style
//============================================================================

#include <iostream>

#include "BD.h"

using namespace std;

int main() {

	BD bd("book06.cbz");
	bd.improve();
	bd.write_pdf();

	return 0;
}
