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

#include "element_change.h"
#include "image.h"
#include "loadpng.h"
#include "seamlessBg.h"

#include "bcm_host.h"

//-------------------------------------------------------------------------

void
initSeamlessBg(
    SEAMLESS_BACKGROUND_T *sb)
{
    int result = 0;

    //---------------------------------------------------------------------
    const char *texture = "texture.png";

    if (loadSeamlessBgPng(&sb->image, texture, true, true) == false)
    {
        fprintf(stderr, "seamlessBg: unable to load %s\n", texture);
        exit(EXIT_FAILURE);
    }

    sb->viewWidth = sb->image.width / 2;
    sb->viewHeight = sb->image.height / 2;

    sb->xOffsetMax = sb->viewWidth - 1;
    sb->xOffset = sb->xOffsetMax / 2;

    sb->yOffsetMax = sb->viewHeight - 1;
    sb->yOffset = sb->yOffsetMax / 2;

    sb->direction = 0;
    sb->directionMax = 7;

    sb->xDirections[0] = 0;
    sb->xDirections[1] = 3;
    sb->xDirections[2] = 4;
    sb->xDirections[3] = 3;
    sb->xDirections[4] = 0;
    sb->xDirections[5] = -3;
    sb->xDirections[6] = -4;
    sb->xDirections[7] = -3;

    sb->yDirections[0] = 4;
    sb->yDirections[1] = 3;
    sb->yDirections[2] = 0;
    sb->yDirections[3] = -3;
    sb->yDirections[4] = -4;
    sb->yDirections[5] = -3;
    sb->yDirections[6] = 0;
    sb->yDirections[7] = 3;

    //---------------------------------------------------------------------

    uint32_t vc_image_ptr;

    sb->frontResource =
        vc_dispmanx_resource_create(
            sb->image.type,
            sb->image.width | (sb->image.pitch << 16),
            sb->image.height | (sb->image.alignedHeight << 16),
            &vc_image_ptr);
    assert(sb->frontResource != 0);

    sb->backResource =
        vc_dispmanx_resource_create(
            sb->image.type,
            sb->image.width | (sb->image.pitch << 16),
            sb->image.height | (sb->image.alignedHeight << 16),
            &vc_image_ptr);
    assert(sb->backResource != 0);

    //---------------------------------------------------------------------

    vc_dispmanx_rect_set(&(sb->dstRect),
                         0,
                         0,
                         sb->image.width,
                         sb->image.height);

    result = vc_dispmanx_resource_write_data(sb->frontResource,
                                             sb->image.type,
                                             sb->image.pitch,
                                             sb->image.buffer,
                                             &(sb->dstRect));
    assert(result == 0);

    result = vc_dispmanx_resource_write_data(sb->backResource,
                                             sb->image.type,
                                             sb->image.pitch,
                                             sb->image.buffer,
                                             &(sb->dstRect));
    assert(result == 0);
}

//-------------------------------------------------------------------------

void
addElementSeamlessBg(
    SEAMLESS_BACKGROUND_T *sb,
    DISPMANX_MODEINFO_T *info,
    DISPMANX_DISPLAY_HANDLE_T display,
    DISPMANX_UPDATE_HANDLE_T update)
{
    VC_DISPMANX_ALPHA_T alpha =
    {
        DISPMANX_FLAGS_ALPHA_FROM_SOURCE, 
        255,
        0
    };

    vc_dispmanx_rect_set(&sb->srcRect,
                         sb->xOffset << 16,
                         sb->yOffset << 16,
                         sb->viewWidth << 16,
                         sb->viewHeight << 16);

    vc_dispmanx_rect_set(&(sb->dstRect),
                         (info->width - sb->viewWidth) / 2,
                         (info->height - sb->viewHeight) / 2,
                         sb->viewWidth,
                         sb->viewHeight);

    sb->element = vc_dispmanx_element_add(update,
                                          display,
                                          1,
                                          &(sb->dstRect),
                                          sb->frontResource,
                                          &(sb->srcRect),
                                          DISPMANX_PROTECTION_NONE,
                                          &alpha,
                                          NULL,
                                          DISPMANX_NO_ROTATE);
    assert(sb->element != 0);
}

//-------------------------------------------------------------------------

void
setDirectionSeamlessBg(
    SEAMLESS_BACKGROUND_T *sb,
    char c)
{
    switch (tolower(c))
    {
    case ',':
    case '<':

        --(sb->direction);
        if (sb->direction < 0)
        {
            sb->direction = sb->directionMax;
        }

        break;

    case '.':
    case '>':

        ++(sb->direction);

        if (sb->direction > sb->directionMax)
        {
            sb->direction = 0;
        }

        break;

    default:

        // do nothing

        break;
    }
}

//-------------------------------------------------------------------------

void
updatePositionSeamlessBg(
    SEAMLESS_BACKGROUND_T *sb,
    DISPMANX_UPDATE_HANDLE_T update)
{
    int result = 0;

    //---------------------------------------------------------------------

    sb->xOffset += sb->xDirections[sb->direction];

    if (sb->xOffset < 0)
    {
        sb->xOffset = sb->xOffsetMax;
    }
    else if (sb->xOffset > sb->xOffsetMax)
    {
        sb->xOffset = 0;
    }

    sb->yOffset -= sb->yDirections[sb->direction];

    if (sb->yOffset < 0)
    {
        sb->yOffset = sb->yOffsetMax;
    }
    else if (sb->yOffset > sb->yOffsetMax)
    {
        sb->yOffset = 0;
    }

    //---------------------------------------------------------------------

    result = vc_dispmanx_element_change_source(update,
                                               sb->element,
                                               sb->backResource);
    assert(result == 0);

    vc_dispmanx_rect_set(&(sb->srcRect),
                         sb->xOffset << 16,
                         sb->yOffset << 16,
                         sb->viewWidth << 16,
                         sb->viewHeight << 16);

    result = 
    vc_dispmanx_element_change_attributes(update,
                                          sb->element,
                                          ELEMENT_CHANGE_SRC_RECT,
                                          0,
                                          255,
                                          &(sb->dstRect),
                                          &(sb->srcRect),
                                          0,
                                          DISPMANX_NO_ROTATE);
    assert(result == 0);

    //---------------------------------------------------------------------

    DISPMANX_RESOURCE_HANDLE_T tmp = sb->frontResource;
    sb->frontResource = sb->backResource;
    sb->backResource = tmp;
}

//-------------------------------------------------------------------------

void
destroySeamlessBg(
    SEAMLESS_BACKGROUND_T *sb)
{
    int result = 0;

    DISPMANX_UPDATE_HANDLE_T update = vc_dispmanx_update_start(0);
    assert(update != 0);
    result = vc_dispmanx_element_remove(update, sb->element);
    assert(result == 0);
    result = vc_dispmanx_update_submit_sync(update);
    assert(result == 0);

    //---------------------------------------------------------------------

    result = vc_dispmanx_resource_delete(sb->frontResource);
    assert(result == 0);
    result = vc_dispmanx_resource_delete(sb->backResource);
    assert(result == 0);

    //---------------------------------------------------------------------

    destroyImage(&(sb->image));
}

//-------------------------------------------------------------------------

bool
loadSeamlessBgPng(
    IMAGE_T* image,
    const char *file,
    bool extendX,
    bool extendY)
{
    IMAGE_T baseImage;
    bool loaded = loadPng(&baseImage, file);

    if (loaded)
    {
        int32_t width = (extendX) ? baseImage.width*2 : baseImage.width;
        int32_t height = (extendY) ? baseImage.height*2 : baseImage.height;

        initImage(image, baseImage.type, width, height);

        if (extendX)
        {
            int32_t rowLength = baseImage.width * baseImage.bytesPerPixel;

            int32_t baseOffset = 0;
            int32_t offset = 0;

            int32_t y = 0;
            for (y = 0 ; y < baseImage.width ; y++)
            {
                baseOffset = y * baseImage.pitch;
                offset = y * image->pitch;

                memcpy(image->buffer + offset,
                       baseImage.buffer + baseOffset,
                       rowLength);

                memcpy(image->buffer + offset + rowLength,
                       baseImage.buffer + baseOffset,
                       rowLength);
            }
        }
        else
        {
            memcpy(image->buffer, baseImage.buffer, baseImage.size);
        }

        if (extendY)
        {
            int32_t size = image->pitch * baseImage.height;

            memcpy(image->buffer + size, image->buffer, size);
        }
    }

    return loaded;
}

