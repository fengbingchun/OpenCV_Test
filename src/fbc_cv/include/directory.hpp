// fbc_cv is free software and uses the same licence as OpenCV
// Email: fengbingchun@163.com

#ifndef FBC_CV_DIRECTORY_HPP_
#define FBC_CV_DIRECTORY_HPP_

// reference: include/opencv2/contrib/contrib.hpp (2.4.9)

#ifndef __cplusplus
	#error directory.hpp header must be compiled as C++
#endif

#include <vector>
#include <string>
#include "core/fbcdef.hpp"

namespace fbc {

class FBC_EXPORTS Directory {
public:
	std::vector<std::string> GetListFiles(const std::string& path, const std::string& exten = "*", bool addPath = true);
	std::vector<std::string> GetListFilesR(const std::string& path, const std::string& exten = "*", bool addPath = true);
	std::vector<std::string> GetListFolders(const std::string& path, const std::string& exten = "*", bool addPath = true);
};

}

#endif // FBC_CV_DIRECTORY_HPP_
