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
#include <stdio.h>
#include <string.h>
#include <termio.h>
#include <unistd.h>

#include "key.h"

//-------------------------------------------------------------------------

int stdin_fd = -1;
struct termios original;

//-------------------------------------------------------------------------

bool keyPressed(int *character)
{
    if (stdin_fd == -1)
    {
        struct termios term;

        stdin_fd = fileno(stdin);

        tcgetattr(stdin_fd, &original);
        memcpy(&term, &original, sizeof(term));
        term.c_lflag &= ~(ICANON|ECHO);
        tcsetattr(stdin_fd, TCSANOW, &term);

        setbuf(stdin, NULL);
    }

    int characters_buffered = 0;
    ioctl(stdin_fd, FIONREAD, &characters_buffered);

    bool pressed = (characters_buffered != 0);

    if (characters_buffered == 1)
    {
        int c = fgetc(stdin);

        if (character != NULL)
        {
            *character = c;
        }
    }
    else if (characters_buffered > 1)
    {
        while (characters_buffered)
        {
            fgetc(stdin);
            --characters_buffered;
        }
    }

    return pressed;
}

//-------------------------------------------------------------------------

void keyboardReset(void)
{
    if (stdin_fd != -1)
    {
        tcsetattr(stdin_fd, TCSANOW, &original);
    }
}
