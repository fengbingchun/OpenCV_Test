#include "opencv_funset.hpp"
#include <iostream>
#include <libexif/exif-loader.h>

// Blog: https://blog.csdn.net/fengbingchun/article/details/135430674

int test_libexif_thumbnail()
{
	// reference: https://github.com/libexif/libexif/blob/master/contrib/examples/thumbnail.c
	// Create an ExifLoader object to manage the EXIF loading process
	ExifLoader* l = exif_loader_new();
	if (!l) {
		std::cerr << "Error: fail to exif_loader_new\n";
		return -1;
	}

#ifdef _MSC_VER
	constexpr char jpg_name[]{"../../../test_images/exif.jpg"};
#else
	constexpr char* jpg_name{ "test_images/exif.jpg" };
#endif

	// Load the EXIF data from the image file
	exif_loader_write_file(l, jpg_name);

	// Get a pointer to the EXIF data
	ExifData* ed = exif_loader_get_data(l);
	if (!ed) {
		std::cerr << "Error: fail to exif_loader_get_data\n";
		return -1;
	}

	// The loader is no longer needed--free it
	exif_loader_unref(l);
	
	// Make sure the image had a thumbnail before trying to write it
	if (ed->data && ed->size) {
		std::cout << "exif data size: " << ed->size << "\n";

		char thumb_name[1024];
		snprintf(thumb_name, sizeof(thumb_name), "%s_thumb.jpg", jpg_name);

		FILE* thumb = fopen(thumb_name, "wb");
		if (!thumb) {
			std::cerr << "Error: fail to fopen: " << thumb_name << "\n";
			return -1;
		}

		// Write the thumbnail image to the file
		fwrite(ed->data, 1, ed->size, thumb);
		fclose(thumb);
	}

	exif_data_unref(ed);

	return 0;
}