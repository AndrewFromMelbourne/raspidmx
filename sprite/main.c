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
#include "sprite.h"

#include "bcm_host.h"

//-------------------------------------------------------------------------

#define NDEBUG

//-------------------------------------------------------------------------

int main(void)
{
    IMAGE_T image;
    const char *texture = "texture.png";

    if (loadSeamlessBackgroundPng(&image, texture, true, true) == false)
    {
        fprintf(stderr, "scrolling: unable to load %s\n", texture);
        exit(EXIT_FAILURE);
    }

    int32_t tViewWidth = image.width / 2;
    int32_t tViewHeight = image.height / 2;

    int32_t tXoffsetMax = tViewWidth - 1;
    int32_t tXoffset = tXoffsetMax / 2;
    int32_t tXdirection = 1;

    int32_t tYoffsetMax = tViewHeight - 1;
    int32_t tYoffset = tYoffsetMax / 2;
    int32_t tYdirection = 0;

    //---------------------------------------------------------------------

    SPRITE_T sprite;
    loadSpritePng(&sprite, "sprite.png", 12, 1);

    int32_t sXoffsetMax = (sprite.columns - 1) * sprite.width;
    int32_t sXoffset = 0;

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

    DISPMANX_RESOURCE_HANDLE_T frontTResource =
        vc_dispmanx_resource_create(
            image.type,
            image.width | (image.pitch << 16),
            image.height | (image.alignedHeight << 16),
            &vc_image_ptr);
    assert(frontTResource != 0);

    DISPMANX_RESOURCE_HANDLE_T backTResource =
        vc_dispmanx_resource_create(
            image.type,
            image.width | (image.pitch << 16),
            image.height | (image.alignedHeight << 16),
            &vc_image_ptr);
    assert(backTResource != 0);

    //---------------------------------------------------------------------

    DISPMANX_RESOURCE_HANDLE_T frontSResource =
        vc_dispmanx_resource_create(
            sprite.image.type,
            sprite.image.width | (sprite.image.pitch << 16),
            sprite.image.height | (sprite.image.alignedHeight << 16),
            &vc_image_ptr);
    assert(frontSResource != 0);

    DISPMANX_RESOURCE_HANDLE_T backSResource =
        vc_dispmanx_resource_create(
            sprite.image.type,
            sprite.image.width | (sprite.image.pitch << 16),
            sprite.image.height | (sprite.image.alignedHeight << 16),
            &vc_image_ptr);
    assert(backSResource != 0);

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

    result = vc_dispmanx_resource_write_data(frontTResource,
                                             image.type,
                                             image.pitch,
                                             image.buffer,
                                             &dst_rect);
    assert(result == 0);

    result = vc_dispmanx_resource_write_data(backTResource,
                                             image.type,
                                             image.pitch,
                                             image.buffer,
                                             &dst_rect);
    assert(result == 0);

    //---------------------------------------------------------------------

    vc_dispmanx_rect_set(&dst_rect,
                         0,
                         0,
                         sprite.image.width,
                         sprite.image.height);

    result = vc_dispmanx_resource_write_data(frontSResource,
                                             sprite.image.type,
                                             sprite.image.pitch,
                                             sprite.image.buffer,
                                             &dst_rect);
    assert(result == 0);

    result = vc_dispmanx_resource_write_data(backSResource,
                                             sprite.image.type,
                                             sprite.image.pitch,
                                             sprite.image.buffer,
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
                         tXoffset << 16,
                         tYoffset << 16,
                         tViewWidth << 16,
                         tViewHeight << 16);

    vc_dispmanx_rect_set(&dst_rect,
                         (info.width - tViewWidth) / 2,
                         (info.height - tViewHeight) / 2,
                         tViewWidth,
                         tViewHeight);

    DISPMANX_ELEMENT_HANDLE_T tElement =
        vc_dispmanx_element_add(update,
                                display,
                                2, // layer
                                &dst_rect,
                                frontTResource,
                                &src_rect,
                                DISPMANX_PROTECTION_NONE,
                                &alpha,
                                NULL, // clamp
                                DISPMANX_NO_ROTATE);
    assert(tElement != 0);

    //---------------------------------------------------------------------

    vc_dispmanx_rect_set(&src_rect,
                         sXoffset << 16,
                         0 << 16,
                         sprite.width << 16,
                         sprite.height << 16);

    vc_dispmanx_rect_set(&dst_rect,
                         (info.width - sprite.width) / 2,
                         (info.height - sprite.height) / 2,
                         sprite.width,
                         sprite.height);

    DISPMANX_ELEMENT_HANDLE_T sElement =
        vc_dispmanx_element_add(update,
                                display,
                                3, // layer
                                &dst_rect,
                                frontSResource,
                                &src_rect,
                                DISPMANX_PROTECTION_NONE,
                                &alpha,
                                NULL, // clamp
                                DISPMANX_NO_ROTATE);
    assert(tElement != 0);

    //---------------------------------------------------------------------

    result = vc_dispmanx_update_submit_sync(update);
    assert(result == 0);

    //---------------------------------------------------------------------

    int c = 0;

    while (c != 27)
    {
        if (keyPressed(&c))
        {
            switch (tolower(c))
            {
            case 'w':

                tXdirection = 0;
                tYdirection = -1;
                break;

            case 's':

                tXdirection = 0;
                tYdirection = 1;
                break;

            case 'a':

                tXdirection = -1;
                tYdirection = 0;
                break;

            case 'd':

                tXdirection = 1;
                tYdirection = 0;
                break;

            default:

                // do nothing
                break;
            }
        }

        //-----------------------------------------------------------------

        tXoffset += tXdirection;

        if (tXoffset < 0)
        {
            tXoffset = tXoffsetMax;
        }
        else if (tXoffset > tXoffsetMax)
        {
            tXoffset = 0;
        }

        tYoffset += tYdirection;

        if (tYoffset < 0)
        {
            tYoffset = tYoffsetMax;
        }
        else if (tYoffset > tYoffsetMax)
        {
            tYoffset = 0;
        }

        //-----------------------------------------------------------------

        sXoffset += sprite.width;

        if (sXoffset > sXoffsetMax)
        {
            sXoffset = 0;
        }

        //-----------------------------------------------------------------

        DISPMANX_UPDATE_HANDLE_T update = vc_dispmanx_update_start(0);
        assert(update != 0);

        //-----------------------------------------------------------------

        result = vc_dispmanx_element_change_source(update,
                                                   tElement,
                                                   backTResource);
        assert(result == 0);

        vc_dispmanx_rect_set(&src_rect,
                             tXoffset << 16,
                             tYoffset << 16,
                             tViewWidth << 16,
                             tViewHeight << 16);

        vc_dispmanx_rect_set(&dst_rect,
                             (info.width - tViewWidth) / 2,
                             (info.height - tViewHeight) / 2,
                             tViewWidth,
                             tViewHeight);

        result = 
        vc_dispmanx_element_change_attributes(update,
                                              tElement,
                                              ELEMENT_CHANGE_SRC_RECT,
                                              0,
                                              255,
                                              &dst_rect,
                                              &src_rect,
                                              0,
                                              DISPMANX_NO_ROTATE);
        assert(result == 0);

        //-----------------------------------------------------------------

        vc_dispmanx_rect_set(&src_rect,
                             sXoffset << 16,
                             0 << 16,
                             sprite.width << 16,
                             sprite.height << 16);

        vc_dispmanx_rect_set(&dst_rect,
                             (info.width - sprite.width) / 2,
                             (info.height - sprite.height) / 2,
                             sprite.width,
                             sprite.height);

        result = 
        vc_dispmanx_element_change_attributes(update,
                                              sElement,
                                              ELEMENT_CHANGE_SRC_RECT,
                                              0,
                                              255,
                                              &dst_rect,
                                              &src_rect,
                                              0,
                                              DISPMANX_NO_ROTATE);
        assert(result == 0);

        //-----------------------------------------------------------------

        result = vc_dispmanx_update_submit_sync(update);
        assert(result == 0);

        //-----------------------------------------------------------------

        DISPMANX_RESOURCE_HANDLE_T tmp;
        
        tmp = frontTResource;
        frontTResource = backTResource;
        backTResource = tmp;

        tmp = frontSResource;
        frontSResource = backSResource;
        backSResource = tmp;
    }

    //---------------------------------------------------------------------

    update = vc_dispmanx_update_start(0);
    assert(update != 0);
    result = vc_dispmanx_element_remove(update, tElement);
    assert(result == 0);
    result = vc_dispmanx_element_remove(update, bgElement);
    assert(result == 0);
    result = vc_dispmanx_update_submit_sync(update);
    assert(result == 0);

    //---------------------------------------------------------------------

    result = vc_dispmanx_resource_delete(frontTResource);
    assert(result == 0);
    result = vc_dispmanx_resource_delete(backTResource);
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

