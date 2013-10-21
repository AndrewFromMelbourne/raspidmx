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

#define _GNU_SOURCE

#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <termio.h>
#include <unistd.h>
#include <sys/time.h>

#include "bcm_host.h"

#include "backgroundLayer.h"
#include "font.h"
#include "imageGraphics.h"
#include "imageLayer.h"
#include "key.h"
#include "mandelbrot.h"

//-------------------------------------------------------------------------

#define NDEBUG

//-------------------------------------------------------------------------

bool
zoom(
    MANDELBROT_COORDS_T *coords,
    IMAGE_LAYER_T *zoomLayer)
{
    bool changed = false;

    RGBA8_T maskColour = { 127, 127, 127, 127 };
    RGBA8_T clearColour = { 0, 0, 0, 0 };

    int32_t width = zoomLayer->image.width / 4;
    int32_t height = zoomLayer->image.height / 4;
    int32_t x = (zoomLayer->image.width - width) / 2;
    int32_t y = (zoomLayer->image.height - height) / 2;
    int32_t step = 1;

    clearImage(&(zoomLayer->image), &maskColour);
    imageBoxFilled(&(zoomLayer->image),
                   x,
                   y,
                   x + width - 1,
                   y + height - 1,
                   &clearColour);
    changeSourceAndUpdateImageLayer(zoomLayer);

    int c = 0;
    while ((c != 27) && (changed == false))
    {
        if (keyPressed(&c))
        {
            c = tolower(c);

            bool update = false;

            switch (c)
            {
            case 10:

                changed = true;
                break;

            case 'a':

                if ((x - step) >= 0)
                {
                    x -= step;
                    update = true;
                }
                break;

            case 'd':

                if ((x + width + step) <= zoomLayer->image.width)
                {
                    x += step;
                    update = true;
                }
                break;

            case 'w':

                if ((y - step) >= 0)
                {
                    y -= step;
                    update = true;
                }
                break;

            case 's':

                if ((y + height + step) <= zoomLayer->image.height)
                {
                    y += step;
                    update = true;
                }
                break;
            }

            if (update)
            {
                clearImage(&(zoomLayer->image), &maskColour);
                imageBoxFilled(&(zoomLayer->image),
                               x,
                               y,
                               x + width - 1,
                               y + height - 1,
                               &clearColour);
                changeSourceAndUpdateImageLayer(zoomLayer);
            }
        }
    }

    clearImage(&(zoomLayer->image), &clearColour);
    changeSourceAndUpdateImageLayer(zoomLayer);

    if (changed)
    {
        coords->x0 += (coords->side * x) / zoomLayer->image.width;
        coords->y0 += (coords->side * y) / zoomLayer->image.height;
        coords->side *=  (double)width / zoomLayer->image.width;
    }

    return changed;
}

//-------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    bcm_host_init();

    //---------------------------------------------------------------------

    DISPMANX_DISPLAY_HANDLE_T display = vc_dispmanx_display_open(0);
    assert(display != 0);

    //---------------------------------------------------------------------

    DISPMANX_MODEINFO_T info;

    int result = vc_dispmanx_display_get_info(display, &info);
    assert(result == 0);

    //---------------------------------------------------------------------

    BACKGROUND_LAYER_T bg;
    initBackgroundLayer(&bg, 0x000F, 0);

    IMAGE_LAYER_T mandelbrotLayer;
    initImageLayer(&mandelbrotLayer,
                   info.height,
                   info.height,
                   VC_IMAGE_RGB888);
    createResourceImageLayer(&mandelbrotLayer, 1);

    IMAGE_LAYER_T zoomLayer;
    initImageLayer(&zoomLayer,
                   mandelbrotLayer.image.width,
                   mandelbrotLayer.image.height,
                   VC_IMAGE_RGBA16);
    createResourceImageLayer(&zoomLayer, 10);

    //---------------------------------------------------------------------

    DISPMANX_UPDATE_HANDLE_T update = vc_dispmanx_update_start(0);
    assert(update != 0);

    //---------------------------------------------------------------------

    addElementBackgroundLayer(&bg, display, update);
    addElementImageLayerCentered(&mandelbrotLayer, &info, display, update);
    addElementImageLayerCentered(&zoomLayer, &info, display, update);

    //---------------------------------------------------------------------

    result = vc_dispmanx_update_submit_sync(update);
    assert(result == 0);

    //---------------------------------------------------------------------

    MANDELBROT_COORDS_T coords = { -2.0, -1.5, 3.0 };
    mandelbrotImage(&mandelbrotLayer, &coords);

    //---------------------------------------------------------------------

    int c = 0;
    while (c != 27)
    {
        if (keyPressed(&c))
        {
            c = tolower(c);

            switch (c)
            {
            case 'z':

                if (zoom(&coords, &zoomLayer))
                {
                    mandelbrotImage(&mandelbrotLayer, &coords);
                }
                break;
            }
        }
    }

    //---------------------------------------------------------------------

    keyboardReset();

    //---------------------------------------------------------------------

    destroyBackgroundLayer(&bg);
    destroyImageLayer(&mandelbrotLayer);
    destroyImageLayer(&zoomLayer);

    //---------------------------------------------------------------------

    result = vc_dispmanx_display_close(display);
    assert(result == 0);

    //---------------------------------------------------------------------

    return 0;
}

