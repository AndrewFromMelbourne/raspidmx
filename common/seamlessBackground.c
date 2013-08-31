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

#include <stdbool.h>

#include "image.h"
#include "loadpng.h"
#include "seamlessBackground.h"

//-------------------------------------------------------------------------

bool
loadSeamlessBackgroundPng(
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

