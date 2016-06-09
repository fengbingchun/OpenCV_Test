#include <iostream>
#include <string>
#include <fstream>

#include "test_core.hpp"
#include "test_directory.hpp"
#include "test_resize.hpp"

int main()
{
	test_resize_uchar();
	test_resize_float();
	test_resize_area();

	std::cout << "ok" << std::endl;
	return 0;
}

