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

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "hsv2rgb.h"
#include "image.h"
#include "worms.h"

//-------------------------------------------------------------------------

void
initWorm(
    uint16_t worm_number,
    uint16_t number_of_worms,
    uint16_t worm_length,
    WORM_T *worm,
    IMAGE_T *image)
{
    hsv2rgb((3600 * worm_number) / number_of_worms,
            1000,
            1000,
            &(worm->colour));
    
    worm->colour.alpha = 255;
    worm->direction = (rand() * 360.0) / RAND_MAX;
    worm->head = worm_length - 1;
    worm->size = worm_length;
    worm->body = malloc(worm->size * sizeof(COORD_T));

    if (worm->body == NULL)
    {
        fprintf(stderr, "worms: memory exhausted\n");
        exit(EXIT_FAILURE);
    }

    double x = (double)rand() * image->width / RAND_MAX;
    double y = (double)rand() * image->height / RAND_MAX;

    uint16_t i = 0;
    for (i = 0 ; i < worm->size ; i++)
    {
        worm->body[i].x = x;
        worm->body[i].y = y;
    }
}

//-------------------------------------------------------------------------

void
updateWorm(
    WORM_T *worm,
    IMAGE_T *image)
{
    uint16_t wasHead = worm->head;
    worm->head = (worm->head + 1) % worm->size;

    double x = worm->body[wasHead].x;
    double y = worm->body[wasHead].y;

    //---------------------------------------------------------------------
    // randomly change direction

    worm->direction += (40.0 * rand() / RAND_MAX) - 20.0;

    if (worm->direction > 360.0)
    {
        worm->direction -= 360.0;
    }
    else if (worm->direction < 0.0)
    {
        worm->direction += 360.0;
    }

    //---------------------------------------------------------------------
    // calculate new head position

    x = x + cos(worm->direction * M_PI / 180.0);

    if (x < 0.0)
    {
        x += image->width;
    }
    else if (x > image->width)
    {
        x -= image->width;
    }

    y = y + sin(worm->direction * M_PI / 180.0);

    if (y < 0.0)
    {
        y += image->height;
    }
    else if (y > image->height)
    {
        y -= image->height;
    }

    //---------------------------------------------------------------------

    worm->body[worm->head].x = x;
    worm->body[worm->head].y = y;
}

//-------------------------------------------------------------------------

void
drawWorm(
    WORM_T *worm,
    IMAGE_T *image)
{
    uint16_t i = 0;
    for (i = 0 ; i < worm->size ; i++)
    {
        image->setPixel(image,
                        (int32_t)floor(worm->body[i].x),
                        (int32_t)floor(worm->body[i].y),
                        &(worm->colour));
    }
}

//-------------------------------------------------------------------------

void
undrawWorm(
    WORM_T *worm,
    IMAGE_T *image)
{
    RGBA8_T colour = { 0, 0, 0, 0 };

    uint32_t i = 0;
    for (i = 0 ; i < worm->size ; i++)
    {
        image->setPixel(image,
                        (int32_t)floor(worm->body[i].x),
                        (int32_t)floor(worm->body[i].y),
                        &colour);
    }
}

//-------------------------------------------------------------------------

void
destroyWorm(
    WORM_T* worm)
{
    free(worm->body);

    worm->head = 0;
    worm->size = 0;
    worm->body = NULL;
}

//-------------------------------------------------------------------------

void
initWorms(
    uint16_t number,
    uint16_t length,
    WORMS_T *worms,
    IMAGE_T *image)
{
    srand(time(NULL));

    worms->size = number;
    worms->worms = malloc(worms->size * sizeof(WORM_T));

    if (worms->worms == NULL)
    {
        fprintf(stderr, "worms: memory exhausted\n");
        exit(EXIT_FAILURE);
    }

    uint16_t i = 0;
    for (i = 0 ; i < worms->size ; i++)
    {
        WORM_T *worm = &(worms->worms[i]);
        initWorm(i, number, length, worm, image);
    }
}

//-------------------------------------------------------------------------

void
updateWorms(
    WORMS_T *worms,
    IMAGE_T *image)
{
    uint16_t i = 0;
    for (i = 0 ; i < worms->size ; i++)
    {
        WORM_T *worm = &(worms->worms[i]);
        updateWorm(worm, image);
    }
}

//-------------------------------------------------------------------------

void
drawWorms(
    WORMS_T *worms,
    IMAGE_T *image)
{
    uint16_t i = 0;
    for (i = 0 ; i < worms->size ; i++)
    {
        WORM_T *worm = &(worms->worms[i]);
        drawWorm(worm, image);
    }
}

//-------------------------------------------------------------------------

void
undrawWorms(
    WORMS_T *worms,
    IMAGE_T *image)
{
    uint16_t i = 0;
    for (i = 0 ; i < worms->size ; i++)
    {
        WORM_T *worm = &(worms->worms[i]);
        undrawWorm(worm, image);
    }
}

//-------------------------------------------------------------------------

void
destroyWorms(
    WORMS_T *worms)
{
    uint16_t i = 0;
    for (i = 0 ; i < worms->size ; i++)
    {
        WORM_T *worm = &(worms->worms[i]);
        destroyWorm(worm);
    }

    free(worms->worms);

    worms->size = 0;
    worms->worms = NULL;
}

