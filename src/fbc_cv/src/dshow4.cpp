// fbc_cv is free software and uses the same licence as FFmpeg
// Email: fengbingchun@163.com

// reference: ffmpeg 4.2
//            libavdevice/dshow.c

#ifdef _MSC_VER

#include <Shlwapi.h>
#include "dshow_capture.hpp"

namespace fbc {

HRESULT SHCreateStreamOnFile_xxx(LPCSTR pszFile, DWORD grfMode, IStream **ppstm)
{
	return SHCreateStreamOnFile(pszFile, grfMode, ppstm);
}

} // namespace fbc

#endif // _MSC_VER
