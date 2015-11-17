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
#include "imageLayer.h"
#include "key.h"
#include "life.h"

//-------------------------------------------------------------------------

#define NDEBUG

//-------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    int opt = 0;
    int32_t size = 350;
    uint32_t displayNumber = 0;

    //-------------------------------------------------------------------

    while ((opt = getopt(argc, argv, "d:s:")) != -1)
    {
        switch (opt)
        {
        case 'd':

            displayNumber = atoi(optarg);
            break;

        case 's':

            size = atoi(optarg);
            break;

        default:

            fprintf(stderr,
                    "Usage: %s [-d <number>] [-s <size>]\n",
                    basename(argv[0]));

            fprintf(stderr, "    -d - Raspberry Pi display number\n");
            fprintf(stderr, "    -s - size of image to create\n");
            exit(EXIT_FAILURE);
            break;
        }
    }

    //-------------------------------------------------------------------

    bcm_host_init();

    //---------------------------------------------------------------------

    DISPMANX_DISPLAY_HANDLE_T display
        = vc_dispmanx_display_open(displayNumber);
    assert(display != 0);

    //---------------------------------------------------------------------

    DISPMANX_MODEINFO_T info;

    int result = vc_dispmanx_display_get_info(display, &info);
    assert(result == 0);

    //---------------------------------------------------------------------

    if (size < 1)
    {
        size = info.height;
    }

    if (size > info.height)
    {
        size = info.height;
    }

    if (size > info.width)
    {
        size = info.width;
    }

    //---------------------------------------------------------------------

    BACKGROUND_LAYER_T bg;
    initBackgroundLayer(&bg, 0x000F, 0);

    LIFE_T life;
    newLife(&life, size);

    IMAGE_LAYER_T fpsLayer;
    RGBA8_T fpsBgColour = { 0, 0, 0, 0 };
    RGBA8_T fpsFgColour = { 255, 255, 0, 255 };
    char fpsBuffer[20];

    initImageLayer(&fpsLayer, 64, 16, VC_IMAGE_RGBA16);
    clearImageRGB(&(fpsLayer.image), &fpsBgColour);
    snprintf(fpsBuffer, sizeof(fpsBuffer), "fps: --");
    drawStringRGB(0, 0, fpsBuffer, &fpsFgColour, &(fpsLayer.image));
    createResourceImageLayer(&fpsLayer, 10);

    //---------------------------------------------------------------------

    DISPMANX_UPDATE_HANDLE_T update = vc_dispmanx_update_start(0);
    assert(update != 0);

    //---------------------------------------------------------------------

    int32_t fpsXoffset = 0;
    int32_t fpsYoffset = 0;
    int32_t dst_size = size;
    
    if (dst_size < info.height)
    {
        dst_size = info.height - (info.height % size);
    }

    if (((info.width - dst_size) / 2) > fpsLayer.image.width)
    {
        fpsXoffset = ((info.width - dst_size) / 2) - fpsLayer.image.width;
    }

    if (((info.height - dst_size) / 2) > fpsLayer.image.height)
    {
        fpsYoffset = ((info.height - dst_size) / 2);
    }

    //---------------------------------------------------------------------

    addElementBackgroundLayer(&bg, display, update);
    addElementLife(&life, &info, display, update);
    addElementImageLayerOffset(&fpsLayer,
                               fpsXoffset,
                               fpsYoffset,
                               display,
                               update);

    //---------------------------------------------------------------------

    result = vc_dispmanx_update_submit_sync(update);
    assert(result == 0);

    //---------------------------------------------------------------------

    bool paused = false;
    bool step = false;

    uint32_t frame = 0;

    struct timeval start_time;
    struct timeval end_time;

    gettimeofday(&start_time, NULL);

    int c = 0;
    while (c != 27)
    {
        if (keyPressed(&c))
        {
            c = tolower(c);

            switch (c)
            {
            case 'p':

                if (paused)
                {
                    frame = 0;
                    gettimeofday(&start_time, NULL);
                }
                else
                {
                    clearImageRGB(&(fpsLayer.image), &fpsBgColour);
                    snprintf(fpsBuffer, sizeof(fpsBuffer), "fps: --");
                    drawStringRGB(0,
                                  0,
                                  fpsBuffer,
                                  &fpsFgColour,
                                  &(fpsLayer.image));
                    step = true;
                }

                paused = !paused;
                break;

            case ' ':

                if (paused)
                {
                    step = true;
                }
            }
        }

        //-----------------------------------------------------------------

        ++frame;

        if ((frame == 200) && (paused == false))
        {
            frame = 0;
            gettimeofday(&end_time, NULL);

            struct timeval diff;
            timersub(&end_time, &start_time, &diff);
            int32_t time_taken = (diff.tv_sec * 1000)+(diff.tv_usec / 1000);
            double frames_per_second = 2.0e5 / time_taken;
            snprintf(fpsBuffer,
                     sizeof(fpsBuffer),
                     "fps: %.0f\n",
                     frames_per_second);
            clearImageRGB(&(fpsLayer.image), &fpsBgColour);
            drawStringRGB(0, 0, fpsBuffer, &fpsFgColour, &(fpsLayer.image));

            memcpy(&start_time, &end_time, sizeof(start_time));
        }

        //-----------------------------------------------------------------

        if ((paused == false) || step)
        {
            iterateLife(&life);

            //-------------------------------------------------------------

            update = vc_dispmanx_update_start(0);
            assert(update != 0);

            changeSourceLife(&life, update);

            if ((frame == 0) || step)
            {
                changeSourceImageLayer(&fpsLayer, update);
            }

            result = vc_dispmanx_update_submit_sync(update);
            assert(result == 0);

            //-------------------------------------------------------------

            step = false;
        }
    }

    //---------------------------------------------------------------------

    keyboardReset();

    //---------------------------------------------------------------------

    destroyBackgroundLayer(&bg);
    destroyLife(&life);
    destroyImageLayer(&fpsLayer);

    //---------------------------------------------------------------------

    result = vc_dispmanx_display_close(display);
    assert(result == 0);

    //---------------------------------------------------------------------

    return 0;
}

