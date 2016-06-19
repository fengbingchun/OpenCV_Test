#include <iostream>
#include <string>
#include <fstream>
#include <assert.h>

#include "test_core.hpp"
#include "test_directory.hpp"
#include "test_resize.hpp"
#include "test_cvtColor.hpp"


int main()
{
	test_cvtColor_YUV2Gray();

	std::cout << "ok" << std::endl;
	return 0;
}

