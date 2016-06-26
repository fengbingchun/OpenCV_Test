#include <iostream>
#include <string>
#include <fstream>
#include <assert.h>

#include "test_core.hpp"
#include "test_directory.hpp"
#include "test_resize.hpp"
#include "test_cvtColor.hpp"
#include "test_split.hpp"
#include "test_merge.hpp"

int main()
{
	test_merge_uchar();
	test_merge_float();

	std::cout << "ok" << std::endl;
	return 0;
}

