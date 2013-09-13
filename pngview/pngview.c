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

#include "backgroundLayer.h"
#include "imageLayer.h"
#include "key.h"
#include "loadpng.h"

#include "bcm_host.h"

//-------------------------------------------------------------------------

#define NDEBUG

//-------------------------------------------------------------------------

const char *program = NULL;

//-------------------------------------------------------------------------

void usage(void)
{
    fprintf(stderr, "Usage: %s [-b <RGBA>] <file.png>\n", program);
    fprintf(stderr, "    -b - set background colour 16 bit RGBA\n");
    fprintf(stderr, "         e.g. 0x000F is opaque black\n");

    exit(EXIT_FAILURE);
}

//-------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    uint16_t background = 0x000F;

    program = argv[0];

    //---------------------------------------------------------------------

    int opt = 0;

    while ((opt = getopt(argc, argv, "b:")) != -1)
    {
        switch(opt)
        {
        case 'b':

            background = strtol(optarg, NULL, 16);
            break;

        default:

            usage();
            break;
        }
    }

    //---------------------------------------------------------------------

    if (optind >= argc)
    {
        usage();
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

    BACKGROUND_LAYER_T backgroundLayer;
    initBackgroundLayer(&backgroundLayer, background, 0);

    IMAGE_LAYER_T imageLayer;
    initImageLayer(&imageLayer, argv[optind], 1);

    //---------------------------------------------------------------------

    DISPMANX_UPDATE_HANDLE_T update = vc_dispmanx_update_start(0);
    assert(update != 0);

    addElementBackgroundLayer(&backgroundLayer, display, update);
    addElementImageLayerCentered(&imageLayer, &info, display, update);

    result = vc_dispmanx_update_submit_sync(update);
    assert(result == 0);

    //---------------------------------------------------------------------

    while (keyPressed(NULL) == false)
    {
        // do nothing
    }

    //---------------------------------------------------------------------

    destroyBackgroundLayer(&backgroundLayer);
    destroyImageLayer(&imageLayer);

    //---------------------------------------------------------------------

    result = vc_dispmanx_display_close(display);
    assert(result == 0);

    //---------------------------------------------------------------------

    return 0;
}

