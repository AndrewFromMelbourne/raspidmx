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
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "element_change.h"
#include "image.h"
#include "key.h"
#include "seamlessBackground.h"

#include "bcm_host.h"

//-------------------------------------------------------------------------

#define NDEBUG

//-------------------------------------------------------------------------

int main(void)
{
    IMAGE_T image;
    const char *texture = "texture.png";

    //---------------------------------------------------------------------

    if (loadSeamlessBackgroundPng(&image, texture, true, true) == false)
    {
        fprintf(stderr, "scrolling: unable to load %s\n", texture);
        exit(EXIT_FAILURE);
    }

    int32_t viewWidth = image.width / 2;
    int32_t viewHeight = image.height / 2;

    int32_t xOffsetMax = viewWidth - 1;
    int32_t xOffset = xOffsetMax / 2;
    int32_t xDirection = 1;

    int32_t yOffsetMax = viewHeight - 1;
    int32_t yOffset = yOffsetMax / 2;
    int32_t yDirection = 0;

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

    DISPMANX_RESOURCE_HANDLE_T bgResource;
    VC_IMAGE_TYPE_T bgType = VC_IMAGE_RGB565;
    uint32_t vc_image_ptr;

    bgResource = vc_dispmanx_resource_create(bgType, 1, 1, &vc_image_ptr);
    assert(bgResource != 0);

    //---------------------------------------------------------------------

    DISPMANX_RESOURCE_HANDLE_T frontResource =
        vc_dispmanx_resource_create(
            image.type,
            image.width | (image.pitch << 16),
            image.height | (image.alignedHeight << 16),
            &vc_image_ptr);
    assert(frontResource != 0);

    DISPMANX_RESOURCE_HANDLE_T backResource =
        vc_dispmanx_resource_create(
            image.type,
            image.width | (image.pitch << 16),
            image.height | (image.alignedHeight << 16),
            &vc_image_ptr);
    assert(backResource != 0);

    //---------------------------------------------------------------------

    VC_RECT_T src_rect;
    VC_RECT_T dst_rect;

    //---------------------------------------------------------------------

    vc_dispmanx_rect_set(&dst_rect, 0, 0, 1, 1);
    uint16_t background = 0x0000;

    result = vc_dispmanx_resource_write_data(bgResource,
                                             bgType,
                                             sizeof(background),
                                             &background,
                                             &dst_rect);
    assert(result == 0);

    //---------------------------------------------------------------------

    vc_dispmanx_rect_set(&dst_rect, 0, 0, image.width, image.height);

    result = vc_dispmanx_resource_write_data(frontResource,
                                             image.type,
                                             image.pitch,
                                             image.buffer,
                                             &dst_rect);
    assert(result == 0);

    result = vc_dispmanx_resource_write_data(backResource,
                                             image.type,
                                             image.pitch,
                                             image.buffer,
                                             &dst_rect);
    assert(result == 0);

    //---------------------------------------------------------------------

    DISPMANX_UPDATE_HANDLE_T update = vc_dispmanx_update_start(0);
    assert(update != 0);

    //---------------------------------------------------------------------

    VC_DISPMANX_ALPHA_T alpha =
    {
        DISPMANX_FLAGS_ALPHA_FROM_SOURCE, 
        255, /*alpha 0->255*/
        0
    };

    //---------------------------------------------------------------------

    vc_dispmanx_rect_set(&src_rect, 0, 0, 1, 1);
    vc_dispmanx_rect_set(&dst_rect, 0, 0, 0, 0);

    DISPMANX_ELEMENT_HANDLE_T bgElement =
        vc_dispmanx_element_add(update,
                                display,
                                1, // layer
                                &dst_rect,
                                bgResource,
                                &src_rect,
                                DISPMANX_PROTECTION_NONE,
                                &alpha,
                                NULL, // clamp
                                DISPMANX_NO_ROTATE);
    assert(bgElement != 0);

    //---------------------------------------------------------------------

    vc_dispmanx_rect_set(&src_rect,
                         xOffset << 16,
                         yOffset << 16,
                         viewWidth << 16,
                         viewHeight << 16);

    vc_dispmanx_rect_set(&dst_rect,
                         (info.width - viewWidth) / 2,
                         (info.height - viewHeight) / 2,
                         viewWidth,
                         viewHeight);

    DISPMANX_ELEMENT_HANDLE_T element =
        vc_dispmanx_element_add(update,
                                display,
                                2, // layer
                                &dst_rect,
                                frontResource,
                                &src_rect,
                                DISPMANX_PROTECTION_NONE,
                                &alpha,
                                NULL, // clamp
                                DISPMANX_NO_ROTATE);
    assert(element != 0);

    //---------------------------------------------------------------------

    result = vc_dispmanx_update_submit_sync(update);
    assert(result == 0);

    //---------------------------------------------------------------------

    vc_dispmanx_rect_set(&dst_rect,
                         (info.width - viewWidth) / 2,
                         (info.height - viewHeight) / 2,
                         viewWidth,
                         viewHeight);

    //---------------------------------------------------------------------

    int c = 0;

    while (c != 27)
    {
        if (keyPressed(&c))
        {
            switch (tolower(c))
            {
            case 'w':

                xDirection = 0;
                yDirection = -1;
                break;

            case 's':

                xDirection = 0;
                yDirection = 1;
                break;

            case 'a':

                xDirection = -1;
                yDirection = 0;
                break;

            case 'd':

                xDirection = 1;
                yDirection = 0;
                break;

            default:

                // do nothing
                break;
            }
        }

        //-----------------------------------------------------------------

        xOffset += xDirection;

        if (xOffset < 0)
        {
            xOffset = xOffsetMax;
        }
        else if (xOffset > xOffsetMax)
        {
            xOffset = 0;
        }

        yOffset += yDirection;

        if (yOffset < 0)
        {
            yOffset = yOffsetMax;
        }
        else if (yOffset > yOffsetMax)
        {
            yOffset = 0;
        }

        //-----------------------------------------------------------------

        vc_dispmanx_rect_set(&src_rect,
                             xOffset << 16,
                             yOffset << 16,
                             viewWidth << 16,
                             viewHeight << 16);

        //-----------------------------------------------------------------

        DISPMANX_UPDATE_HANDLE_T update = vc_dispmanx_update_start(0);
        assert(update != 0);

        result = vc_dispmanx_element_change_source(update,
                                                   element,
                                                   backResource);
        assert(result == 0);

        result = 
        vc_dispmanx_element_change_attributes(update,
                                              element,
                                              ELEMENT_CHANGE_SRC_RECT,
                                              0,
                                              255,
                                              &dst_rect,
                                              &src_rect,
                                              0,
                                              DISPMANX_NO_ROTATE);
        assert(result == 0);

        result = vc_dispmanx_update_submit_sync(update);
        assert(result == 0);

        //-----------------------------------------------------------------

        DISPMANX_RESOURCE_HANDLE_T tmp = frontResource;
        frontResource = backResource;
        backResource = tmp;
    }

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
    result = vc_dispmanx_resource_delete(bgResource);
    assert(result == 0);

    //---------------------------------------------------------------------

    result = vc_dispmanx_display_close(display);
    assert(result == 0);

    //---------------------------------------------------------------------

    destroyImage(&image);

    //---------------------------------------------------------------------

    return 0;
}

