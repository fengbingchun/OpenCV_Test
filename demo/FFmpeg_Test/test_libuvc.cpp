#include "funset.hpp"
#include <chrono>
#include <thread>
#ifndef _MSC_VER
#include <libuvc/libuvc.h>

///////////////////////////////////////////////////////////
// Blog: https://blog.csdn.net/fengbingchun/article/details/120310338
namespace {
void cb(uvc_frame_t* frame, void* ptr)
{
    // We'll convert the image from YUV/JPEG to BGR, so allocate space
     uvc_frame_t* bgr = uvc_allocate_frame(frame->width * frame->height * 3);
    if (!bgr) {
        printf("unable to allocate bgr frame!\n");
        return;
    }

    // Do the BGR conversion
    uvc_error_t ret = uvc_any2bgr(frame, bgr);
    if (ret) {
        uvc_perror(ret, "uvc_any2bgr");
        uvc_free_frame(bgr);
        return;
    }

    /* Call a user function:
    *
    * my_type *my_obj = (*my_type) ptr;
    * my_user_function(ptr, bgr);
    * my_other_function(ptr, bgr->data, bgr->width, bgr->height);
    */

    /* Call a C++ method:
    *
    * my_type *my_obj = (*my_type) ptr;
    * my_obj->my_func(bgr);
    */

    /* Use opencv.highgui to display the image:
    *
    * cvImg = cvCreateImageHeader(
    *     cvSize(bgr->width, bgr->height),
    *     IPL_DEPTH_8U,
    *     3);
    *
    * cvSetData(cvImg, bgr->data, bgr->width * 3);
    *
    * cvNamedWindow("Test", CV_WINDOW_AUTOSIZE);
    * cvShowImage("Test", cvImg);
    * cvWaitKey(10);
    *
    * cvReleaseImageHeader(&cvImg);
    */

    uvc_free_frame(bgr);
}

} // namespace

int test_libuvc_get_webcam_info()
{
    // reference: https://ken.tossell.net/libuvc/doc/
    // Initialize a UVC service context. Libuvc will set up its own libusb context.
    // Replace NULL with a libusb_context pointer to run libuvc from an existing libusb context.
    uvc_context_t* ctx = nullptr;
    uvc_error_t res = uvc_init(&ctx, nullptr);
    if (res < 0) {
        uvc_perror(res, "uvc_init");
        return res;
    }
    fprintf(stdout, "UVC initialized\n");

    // Locates the first attached UVC device, stores in dev
    uvc_device_t* dev = nullptr;
    uvc_device_handle_t* devh = nullptr;
    res = uvc_find_device(ctx, &dev, 0, 0, nullptr); // filter devices: vendor_id, product_id, "serial_num"
    if (res < 0) {
        uvc_perror(res, "uvc_find_device"); // no devices found
    } else {
        fprintf(stdout, "Device found\n");
        // Try to open the device: requires exclusive access
        res = uvc_open(dev, &devh);
        if (res < 0) {
            uvc_perror(res, "uvc_open"); // unable to open device
        } else {
            fprintf(stdout, "Device opened\n");
            // Print out a message containing all the information that libuvc knows about the device
            uvc_print_diag(devh, stderr);
            // Try to negotiate a 640x480 30 fps YUYV stream profile
            uvc_stream_ctrl_t ctrl;
            res = uvc_get_stream_ctrl_format_size(
                devh, &ctrl, /* result stored in ctrl */
                UVC_FRAME_FORMAT_YUYV, /* YUV 422, aka YUV 4:2:2. try _COMPRESSED */
                640, 480, 30 /* width, height, fps */);
            // Print out the result
            uvc_print_stream_ctrl(&ctrl, stderr);
            if (res < 0) {
                uvc_perror(res, "get_mode"); // device doesn't provide a matching stream
                return res;
            } else {
                // Start the video stream. The library will call user function cb: cb(frame, (void*) 12345)
                void* user_ptr = nullptr;
                res = uvc_start_streaming(devh, &ctrl, cb, user_ptr, 0);
                if (res < 0) {
                    uvc_perror(res, "start_streaming"); // unable to start stream
                } else {
                    fprintf(stdout, "Streaming...\n");
                    uvc_set_ae_mode(devh, 1); // e.g., turn on auto exposure
                    std::this_thread::sleep_for(std::chrono::seconds(1)); // stream for 1 seconds
                    // End the stream. Blocks until last callback is serviced
                    uvc_stop_streaming(devh);
                    fprintf(stdout, "Done streaming.\n");
                }
            }
            // Release our handle on the device
            uvc_close(devh);
            fprintf(stdout, "Device closed\n");
        }
        // Release the device descriptor
        uvc_unref_device(dev);
    }

    // Close the UVC context. This closes and cleans up any existing device handles,
    // and it closes the libusb context if one was not provided.
    uvc_exit(ctx);
    fprintf(stdout, "UVC exited\n");

    return 0;
}
#else
int test_libuvc_get_webcam_info()
{
    fprintf(stderr, "libuvc is not supported on windows\n");
    return -1;
}
#endif