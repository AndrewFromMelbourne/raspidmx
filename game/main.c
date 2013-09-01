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

#include "background.h"
#include "element_change.h"
#include "image.h"
#include "key.h"
#include "seamlessBg.h"
#include "sprite.h"

#include "bcm_host.h"

//-------------------------------------------------------------------------

#define NDEBUG

//-------------------------------------------------------------------------

int main(void)
{
    bcm_host_init();

    //---------------------------------------------------------------------

    BACKGROUND_T bg;
    initBackground(&bg);

    SEAMLESS_BACKGROUND_T sb;
    initSeamlessBg(&sb);

    SPRITE_T sprite;
    initSprite(&sprite);

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

    addElementBackground(&bg, display, update);
    addElementSeamlessBg(&sb, &info, display, update);
    addElementSprite(&sprite, &info, display, update);

    result = vc_dispmanx_update_submit_sync(update);
    assert(result == 0);

    //---------------------------------------------------------------------

    int c = 0;

    while (c != 27)
    {
        if (keyPressed(&c))
        {
            setDirectionSeamlessBg(&sb, c);
        }

        //-----------------------------------------------------------------

        DISPMANX_UPDATE_HANDLE_T update = vc_dispmanx_update_start(0);
        assert(update != 0);

        updatePositionSeamlessBg(&sb, update);
        updatePositionSprite(&sprite, update);

        result = vc_dispmanx_update_submit_sync(update);
        assert(result == 0);
    }

    //---------------------------------------------------------------------

    destroyBackground(&bg);
    destroySeamlessBg(&sb);
    destroySprite(&sprite);

    //---------------------------------------------------------------------

    result = vc_dispmanx_display_close(display);
    assert(result == 0);

    //---------------------------------------------------------------------

    return 0;
}

