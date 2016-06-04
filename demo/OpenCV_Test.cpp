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

	std::cout << "ok" << std::endl;
	return 0;
}

