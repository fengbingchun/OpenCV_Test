#include <assert.h>
#include <iostream>
#include <string>
#include <fstream>

#include "test_all.hpp"

int main()
{
	//test_remap_uchar();
	//test_remap_float();

	run_all_test();
	std::cout << "ok" << std::endl;
	return 0;
}

