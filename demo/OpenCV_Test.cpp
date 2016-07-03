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
#include "test_warpAffine.hpp"

int main()
{
	test_Mat();


	std::cout << "ok" << std::endl;
	return 0;
}

