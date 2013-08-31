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
#include <termio.h>
#include <unistd.h>
#include <sys/time.h>

#include "bcm_host.h"

#include "key.h"
#include "life.h"

//-------------------------------------------------------------------------

#define NDEBUG

//-------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    int opt = 0;
    int32_t size = 350;

    //-------------------------------------------------------------------

    while ((opt = getopt(argc, argv, "s:")) != -1)
    {
        switch (opt)
        {
        case 's':

            size = atoi(optarg);
            break;

        default:

            fprintf(stderr, "Usage: %s [-s size]\n", argv[0]);
            fprintf(stderr, "    -s - size of image to create\n");
            exit(EXIT_FAILURE);
            break;
        }
    }

    //-------------------------------------------------------------------

    VC_IMAGE_TYPE_T type = VC_IMAGE_RGB565;

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

    LIFE_T life;

    newLife(&life, size);

    //---------------------------------------------------------------------

    DISPMANX_RESOURCE_HANDLE_T bgResource;
    uint32_t vc_image_ptr;
    uint32_t background = 0;

    bgResource = vc_dispmanx_resource_create(type, 1, 1, &vc_image_ptr);
    assert(bgResource != 0);

    //---------------------------------------------------------------------

    DISPMANX_RESOURCE_HANDLE_T frontResource;

    frontResource = 
        vc_dispmanx_resource_create(
            type,
            life.width | (life.pitch <<16),
            life.height | (life.alignedHeight << 16),
            &vc_image_ptr);
    assert(frontResource != 0);

    DISPMANX_RESOURCE_HANDLE_T backResource;

    backResource = 
        vc_dispmanx_resource_create(
            type,
            life.width | (life.pitch <<16),
            life.height | (life.alignedHeight << 16),
            &vc_image_ptr);
    assert(backResource != 0);

    //---------------------------------------------------------------------

    VC_RECT_T src_rect;
    VC_RECT_T dst_rect;


    vc_dispmanx_rect_set(&dst_rect, 0, 0, 1, 1);

    result = vc_dispmanx_resource_write_data(bgResource,
                                             type,
                                             sizeof(background),
                                             &background,
                                             &dst_rect);
    assert(result == 0);

    //---------------------------------------------------------------------

    vc_dispmanx_rect_set(&dst_rect, 0, 0, life.width, life.height);

    result = vc_dispmanx_resource_write_data(frontResource,
                                             type,
                                             life.pitch,
                                             life.buffer,
                                             &dst_rect);
    assert(result == 0);

    //---------------------------------------------------------------------

    DISPMANX_UPDATE_HANDLE_T update = vc_dispmanx_update_start(0);
    assert(update != 0);

    //---------------------------------------------------------------------

    VC_DISPMANX_ALPHA_T alpha =
    {
        DISPMANX_FLAGS_ALPHA_FIXED_ALL_PIXELS, 
        255, /*alpha 0->255*/
        0
    };

    //---------------------------------------------------------------------

    vc_dispmanx_rect_set(&src_rect, 0, 0, 1, 1);
    vc_dispmanx_rect_set(&dst_rect, 0, 0, 0, 0);

    DISPMANX_ELEMENT_HANDLE_T bgElement =
        vc_dispmanx_element_add(update,
                                display,
                                0,
                                &dst_rect,
                                bgResource,
                                &src_rect,
                                DISPMANX_PROTECTION_NONE,
                                &alpha,
                                NULL,
                                DISPMANX_NO_ROTATE);
    assert(bgElement != 0);

    //---------------------------------------------------------------------

    int32_t dst_size = info.height - (info.height % life.height);

    //---------------------------------------------------------------------


    vc_dispmanx_rect_set(&src_rect,
                         0,
                         0,
                         life.width << 16,
                         life.height << 16);

    vc_dispmanx_rect_set(&dst_rect,
                         (info.width - dst_size) / 2,
                         (info.height - dst_size) / 2,
                         dst_size,
                         dst_size);

    DISPMANX_ELEMENT_HANDLE_T element =
        vc_dispmanx_element_add(update,
                                display,
                                1,
                                &dst_rect,
                                frontResource,
                                &src_rect,
                                DISPMANX_PROTECTION_NONE,
                                &alpha,
                                NULL,
                                DISPMANX_NO_ROTATE);
    assert(element != 0);

    //---------------------------------------------------------------------

    result = vc_dispmanx_update_submit_sync(update);
    assert(result == 0);

    //---------------------------------------------------------------------

    vc_dispmanx_rect_set(&dst_rect, 0, 0, life.width, life.height);

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
        iterateLife(&life);
        ++frame;

        //-----------------------------------------------------------------

        result = vc_dispmanx_resource_write_data(backResource,
                                                 type,
                                                 life.pitch,
                                                 life.buffer,
                                                 &dst_rect);
        assert(result == 0);

        update = vc_dispmanx_update_start(0);
        assert(update != 0);

        result = vc_dispmanx_element_change_source(update,
                                                   element,
                                                   backResource);
        assert(result == 0);

        result = vc_dispmanx_update_submit_sync(update);
        assert(result == 0);

        // swap front and back resources

        DISPMANX_RESOURCE_HANDLE_T tmp = frontResource;
        frontResource = backResource;
        backResource = tmp;
    }

    //---------------------------------------------------------------------

    struct timeval end_time;
    gettimeofday(&end_time, NULL);

    struct timeval total_time;
    timersub(&end_time, &start_time, &total_time);
    int32_t time_taken = (total_time.tv_sec * 1000000) + total_time.tv_usec;
    double frames_per_second = (frame * 1000000.0) / time_taken;

    printf("%0.1f frames per second\n", frames_per_second);

    //---------------------------------------------------------------------

    update = vc_dispmanx_update_start(0);
    assert(update != 0);
    result = vc_dispmanx_element_remove(update, element);
    assert(result == 0);
    result = vc_dispmanx_element_remove(update, bgElement);
    assert(result == 0);
    result = vc_dispmanx_update_submit_sync(update);
    assert(result == 0);

    //---------------------------------------------------------------------

    result = vc_dispmanx_resource_delete(frontResource);
    assert(result == 0);
    result = vc_dispmanx_resource_delete(backResource);
    assert(result == 0);

    //---------------------------------------------------------------------

    result = vc_dispmanx_resource_delete(bgResource);
    assert(result == 0);

    //---------------------------------------------------------------------

    result = vc_dispmanx_display_close(display);
    assert(result == 0);

    //---------------------------------------------------------------------

    destroyLife(&life);

    //---------------------------------------------------------------------

    return 0;
}

