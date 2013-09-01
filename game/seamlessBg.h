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

#ifndef SEAMLESS_BG_H
#define SEAMLESS_BG_H

#include <stdbool.h>

#include "image.h"

#include "bcm_host.h"

//-------------------------------------------------------------------------

typedef struct
{
    IMAGE_T image;
    int32_t viewWidth;
    int32_t viewHeight;
    int32_t xOffsetMax;
    int32_t xOffset;
    int32_t yOffsetMax;
    int32_t yOffset;
    int16_t direction;
    int16_t directionMax;
    int32_t xDirections[8];
    int32_t yDirections[8];
    VC_RECT_T srcRect;
    VC_RECT_T dstRect;
    DISPMANX_RESOURCE_HANDLE_T frontResource;
    DISPMANX_RESOURCE_HANDLE_T backResource;
    DISPMANX_ELEMENT_HANDLE_T element;
} SEAMLESS_BACKGROUND_T;

//-------------------------------------------------------------------------

void initSeamlessBg(SEAMLESS_BACKGROUND_T *sb);

void
addElementSeamlessBg(
    SEAMLESS_BACKGROUND_T *sb,
    DISPMANX_MODEINFO_T *info,
    DISPMANX_DISPLAY_HANDLE_T display,
    DISPMANX_UPDATE_HANDLE_T update);

void setDirectionSeamlessBg(SEAMLESS_BACKGROUND_T *sb, char c);

void
updatePositionSeamlessBg(
    SEAMLESS_BACKGROUND_T *sb,
    DISPMANX_UPDATE_HANDLE_T update);

void destroySeamlessBg(SEAMLESS_BACKGROUND_T *sb);

bool
loadSeamlessBgPng(
    IMAGE_T* image,
    const char *file,
    bool extendX,
    bool extendY);

//-------------------------------------------------------------------------

#endif
