#include <assert.h>
#include <iostream>
#include <string>
#include <fstream>

#include "fbc_cv_funset.hpp"
#include "opencv_funset.hpp"

int main()
{
	int ret = test_opencv_calcCovarMatrix();

	if (ret == 0) fprintf(stderr, "========== test success ==========\n");
	else fprintf(stderr, "********** testn fail **********\n");

	return 0;
}

