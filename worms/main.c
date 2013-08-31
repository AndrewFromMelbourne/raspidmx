//-------------------------------------------------------------------------
//
// The MIT License (MIT)
//
// Copyright (c) 2013 Andrew Duncan
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
// CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//-------------------------------------------------------------------------

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

#include "bcm_host.h"

#include "image.h"
#include "key.h"
#include "worms.h"

//-----------------------------------------------------------------------

#define NDEBUG

//-----------------------------------------------------------------------

char* program = NULL;

//-------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    int opt = 0;

    const char* imageTypeName = "RGBA32";
    VC_IMAGE_TYPE_T imageType = VC_IMAGE_MIN;

    program = argv[0];

    //-------------------------------------------------------------------

    while ((opt = getopt(argc, argv, "t:")) != -1)
    {
        switch (opt)
        {
        case 't':

            imageTypeName = optarg;
            break;

        default:

            fprintf(stderr, "Usage: %s [-t <type>]\n", program);
            fprintf(stderr, "    -t - type of image to create\n");
            fprintf(stderr, "         can be one of the following:");
            printImageTypes(stderr, " ", "", IMAGE_TYPES_WITH_ALPHA);
            fprintf(stderr, "\n");

            exit(EXIT_FAILURE);
            break;
        }
    }

    //-------------------------------------------------------------------

    IMAGE_TYPE_INFO_T typeInfo;

    if (findImageType(&typeInfo, imageTypeName, IMAGE_TYPES_WITH_ALPHA))
    {
        imageType = typeInfo.type;
    }
    else
    {
        fprintf(stderr,
                "%s: unknown image type %s\n",
                program,
                imageTypeName);

        exit(EXIT_FAILURE);
    }

    //---------------------------------------------------------------------

    bcm_host_init();

    //---------------------------------------------------------------------

    DISPMANX_DISPLAY_HANDLE_T display = vc_dispmanx_display_open(0);
    assert(display != 0);

    //---------------------------------------------------------------------

    DISPMANX_MODEINFO_T info;
    int result = vc_dispmanx_display_get_info(display, &info);
    assert(result == 0);

    //---------------------------------------------------------------------

    IMAGE_T image;
    initImage(&image, imageType, info.width, info.height);

    //---------------------------------------------------------------------

    uint16_t number_of_worms = 36;
    uint16_t worm_length = 25;
    WORMS_T worms;

    initWorms(number_of_worms, worm_length, &worms, &image);

    //---------------------------------------------------------------------

    uint32_t vc_image_ptr;

    DISPMANX_RESOURCE_HANDLE_T frontResource =
        vc_dispmanx_resource_create(imageType,
                                    image.width|(image.pitch<<16),
                                    image.height|(image.alignedHeight<<16),
                                    &vc_image_ptr);
    assert(frontResource != 0);

    DISPMANX_RESOURCE_HANDLE_T backResource =
        vc_dispmanx_resource_create(imageType,
                                    image.width|(image.pitch<<16),
                                    image.height|(image.alignedHeight<<16),
                                    &vc_image_ptr);
    assert(backResource != 0);

    //---------------------------------------------------------------------

    VC_RECT_T dst_rect;
    vc_dispmanx_rect_set(&dst_rect, 0, 0, image.width, image.height);

    result = vc_dispmanx_resource_write_data(frontResource,
                                             imageType,
                                             image.pitch,
                                             image.buffer,
                                             &dst_rect);
    assert(result == 0);

    //---------------------------------------------------------------------

    VC_DISPMANX_ALPHA_T alpha = { DISPMANX_FLAGS_ALPHA_FROM_SOURCE, 0, 0 };

    VC_RECT_T src_rect;
    vc_dispmanx_rect_set(&src_rect,
                         0,
                         0,
                         image.width << 16,
                         image.height << 16);

    vc_dispmanx_rect_set(&dst_rect, 0, 0, image.width, image.height);

    DISPMANX_UPDATE_HANDLE_T update = vc_dispmanx_update_start(0);
    assert(update != 0);

    DISPMANX_ELEMENT_HANDLE_T element =
        vc_dispmanx_element_add(update,
                                display,
                                2000, // layer
                                &dst_rect,
                                frontResource,
                                &src_rect,
                                DISPMANX_PROTECTION_NONE,
                                &alpha,
                                NULL, // clamp
                                DISPMANX_NO_ROTATE);
    assert(element != 0);

    result = vc_dispmanx_update_submit_sync(update);
    assert(result == 0);

    //---------------------------------------------------------------------

    struct timeval start_time;
    gettimeofday(&start_time, NULL);

    //---------------------------------------------------------------------

    int c = 0;
    uint32_t frame = 0;

    while (c != 27)
    {
        keyPressed(&c);

        //-----------------------------------------------------------------

        undrawWorms(&worms, &image);
        updateWorms(&worms, &image);
        drawWorms(&worms, &image);

        //-----------------------------------------------------------------

        result = vc_dispmanx_resource_write_data(backResource,
                                                 imageType,
                                                 image.pitch,
                                                 image.buffer,
                                                 &dst_rect);
        assert(result == 0);

        //-----------------------------------------------------------------

        update = vc_dispmanx_update_start(0);
        assert(update != 0);
        result = vc_dispmanx_element_change_source(update,
                                                   element,
                                                   backResource);
        assert(result == 0);
        result = vc_dispmanx_update_submit_sync(update);
        assert(result == 0);

        //-----------------------------------------------------------------

        DISPMANX_RESOURCE_HANDLE_T tmp = frontResource;
        frontResource = backResource;
        backResource = tmp;

        //-----------------------------------------------------------------

        ++frame;
    }

    //---------------------------------------------------------------------

    struct timeval end_time;
    gettimeofday(&end_time, NULL);

    int32_t time_taken = ((end_time.tv_sec - start_time.tv_sec) * 1000000) +
                     (end_time.tv_usec - start_time.tv_usec);
    double frames_per_second = (frame * 1000000.0) / time_taken;

    printf("%0.1f frames per second\n", frames_per_second);

    //---------------------------------------------------------------------

    update = vc_dispmanx_update_start(0);
    assert(update != 0);
    result = vc_dispmanx_element_remove(update, element);
    assert(result == 0);
    result = vc_dispmanx_update_submit_sync(update);
    assert(result == 0);

    //---------------------------------------------------------------------

    result = vc_dispmanx_resource_delete(frontResource);
    assert(result == 0);
    result = vc_dispmanx_resource_delete(backResource);
    assert(result == 0);

    //---------------------------------------------------------------------

    result = vc_dispmanx_display_close(display);
    assert(result == 0);

    //---------------------------------------------------------------------

    destroyImage(&image);

    //---------------------------------------------------------------------

    return 0;
}

