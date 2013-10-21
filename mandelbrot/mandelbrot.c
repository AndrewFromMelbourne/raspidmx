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
#include <complex.h>

#include "bcm_host.h"

#include "hsv2rgb.h"
#include "image.h"
#include "mandelbrot.h"

//-------------------------------------------------------------------------

void
mandelbrotImage(
    IMAGE_LAYER_T *imageLayer,
    MANDELBROT_COORDS_T *coords)
{
    IMAGE_T *image = &(imageLayer->image);
    RGBA8_T colours[256];

    size_t numberOfColours = (sizeof(colours) / sizeof(colours[0]));
    size_t colour = 0;

    for (colour = 0 ; colour < numberOfColours ; colour++)
    {
        hsv2rgb((numberOfColours - 1 - colour) * (2400 / numberOfColours),
                1000,
                1000,
                &(colours[colour]));
    }

    double dx = (coords->side / (image->width - 1));
    double dy = (coords->side / (image->height - 1));

    RGBA8_T black = {0, 0, 0, 0};
    clearImage(image, &black);

    int32_t j;
    for (j = 0 ; j < image->height ; j++)
    {
        int32_t i;
        for (i = 0 ; i < image->width ; i++)
        {
            double complex c = (coords->x0 + dx * i)
                             + I * (coords->y0 + dy * j);
            double complex z = 0.0;
            size_t n = 0;

            while ((cabs(z) < 2.0) && (n < numberOfColours))
            {
                z = z * z + c;
                n++;
            }

            if (n < numberOfColours)
            {
                setPixel(image, i, j, &(colours[n]));
            }
        }

        changeSourceAndUpdateImageLayer(imageLayer);
    }
}

