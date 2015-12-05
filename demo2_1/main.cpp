﻿/*===============================================================================================
 SimpleEvent Example
 Copyright (c), Firelight Technologies Pty, Ltd 2004-2011.

 Demonstrates basic usage of FMOD's data-driven event library (fmod_event.dll)
===============================================================================================*/
#include "fmod_event.hpp"
#include "fmod_errors.h"
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>

void ERRCHECK(FMOD_RESULT result)
{
    if (result != FMOD_OK)
    {
        printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
        exit(-1);
    }
}

int main(int argc, char *argv[])
{
    FMOD::EventSystem *eventsystem;
    FMOD::Event       *event;
    FMOD_RESULT        result;
    int                key;

    printf("======================================================================\n");
    printf("Simple Event Example.  Copyright (c) Firelight Technologies 2004-2011.\n");
    printf("======================================================================\n");
    printf("This example simply plays an event created with the FMOD Designer tool.\n");
    printf("======================================================================\n\n");

    ERRCHECK(result = FMOD::EventSystem_Create(&eventsystem));
    ERRCHECK(result = eventsystem->init(64, FMOD_INIT_NORMAL, 0, FMOD_EVENT_INIT_NORMAL));
    ERRCHECK(result = eventsystem->setMediaPath("..\\demo2_3\\Media\\Sound\\"));
    ERRCHECK(result = eventsystem->load("examples.fev", 0, 0));
    ERRCHECK(result = eventsystem->getEvent("examples/FeatureDemonstration/Effects/Autopitch", FMOD_EVENT_DEFAULT, &event));

    printf("======================================================================\n");
    printf("Press SPACE to play the event.\n");
    printf("Press ESC   to quit\n");
    printf("======================================================================\n");

    key = 0;
    do
    {
        if (_kbhit())
        {
            key = _getch();

            if (key == ' ')
            {
                ERRCHECK(result = event->start());
            }
        }

        ERRCHECK(result = eventsystem->update());
        Sleep(15);

        FMOD_EVENT_STATE state;
        ERRCHECK(result = event->getState(&state));
        printf("Event is %s       \r", (state & FMOD_EVENT_STATE_PLAYING) ? "playing" : "stopped");

    } while (key != 27);

    ERRCHECK(result = eventsystem->release());
    return 0;
}
