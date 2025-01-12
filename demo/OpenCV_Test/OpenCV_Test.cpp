#include <assert.h>
#include <iostream>
#include <string>
#include <fstream>

#include "fbc_cv_funset.hpp"
#include "opencv_funset.hpp"

int main()
{
	if (auto ret = test_write_video(); ret == 0)
		std::cout << "========== test success ==========\n";
	else
		std::cerr << "########## test fail ##########\n";

	return 0;
}

