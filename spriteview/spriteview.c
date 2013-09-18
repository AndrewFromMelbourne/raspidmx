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
#include "element_change.h"
#include "image.h"
#include "key.h"
#include "spriteLayer.h"

#include "bcm_host.h"

//-------------------------------------------------------------------------

#define NDEBUG

//-------------------------------------------------------------------------

const char *program = NULL;

//-------------------------------------------------------------------------

void usage(void)
{
    fprintf(stderr, "Usage: %s ", program);
    fprintf(stderr, "[-c <columns>] [-r <rows>] <file.png>\n");
    fprintf(stderr, "    -c - number of columns in sprite\n");
    fprintf(stderr, "    -r - number of rows in sprite\n");

    exit(EXIT_FAILURE);
}

//-------------------------------------------------------------------------

int main(int argc, char *argv[])
{
	int columns = 1;
	int rows = 1;
	const char *file = NULL;

    program = argv[0];

    //---------------------------------------------------------------------

    int opt = 0;

    while ((opt = getopt(argc, argv, "c:r:")) != -1)
    {
        switch(opt)
        {
        case 'c':

            columns = atoi(optarg);
            break;

        case 'r':

            rows = atoi(optarg);
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

	file = argv[optind];

    //---------------------------------------------------------------------

    bcm_host_init();

    //---------------------------------------------------------------------

    BACKGROUND_LAYER_T bg;
    initBackgroundLayer(&bg, 0x000F, 0);

    SPRITE_LAYER_T sprite;
    initSpriteLayer(&sprite, columns, rows, file, 1);

    //---------------------------------------------------------------------

    DISPMANX_DISPLAY_HANDLE_T display = vc_dispmanx_display_open(0);
    assert(display != 0);

    //---------------------------------------------------------------------

    DISPMANX_MODEINFO_T info;
    int result = vc_dispmanx_display_get_info(display, &info);
    assert(result == 0);

    //---------------------------------------------------------------------

    DISPMANX_UPDATE_HANDLE_T update = vc_dispmanx_update_start(0);
    assert(update != 0);

    addElementBackgroundLayer(&bg, display, update);
    addElementSpriteLayerCentered(&sprite, &info, display, update);

    result = vc_dispmanx_update_submit_sync(update);
    assert(result == 0);

    //---------------------------------------------------------------------

    int c = 0;

    while (c != 27)
    {
        keyPressed(&c);

        //-----------------------------------------------------------------

        DISPMANX_UPDATE_HANDLE_T update = vc_dispmanx_update_start(0);
        assert(update != 0);

        updatePositionSpriteLayer(&sprite, update);

        result = vc_dispmanx_update_submit_sync(update);
        assert(result == 0);
    }

    //---------------------------------------------------------------------

    destroyBackgroundLayer(&bg);
    destroySpriteLayer(&sprite);

    //---------------------------------------------------------------------

    result = vc_dispmanx_display_close(display);
    assert(result == 0);

    //---------------------------------------------------------------------

    return 0;
}

