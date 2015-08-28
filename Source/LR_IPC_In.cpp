/*
  ==============================================================================

    LR_IPC_In.cpp
    Created: 22 Aug 2015 9:44:03pm
    Author:  Parth

  ==============================================================================
*/

#include "LR_IPC_In.h"
#include "MIDISender.h"
#include "CommandMap.h"

const int LR_IPC_IN::LR_IN_PORT = 58764;

LR_IPC_IN& LR_IPC_IN::getInstance()
{
    static LR_IPC_IN instance;
    return instance;
}

LR_IPC_IN::LR_IPC_IN() : StreamingSocket(),
                         Thread("LR_IPC_IN")
{
    startTimer(1000);
    MIDISender::getInstance(); // enumerate MIDI output devices
}

void LR_IPC_IN::shutdown()
{
    stopTimer();
    stopThread(1000);
    close();
}

void LR_IPC_IN::timerCallback()
{
    if (!isConnected())
    {
        if (connect("127.0.0.1", LR_IN_PORT, 100))
            startThread();
    }
}

void LR_IPC_IN::run()
{
    while (!threadShouldExit())
    {
        char line[256] = { '\0' };
        int sizeRead = 0;
        bool canReadLine = true;

        while (!String(line).endsWithChar('\n') && !threadShouldExit())
        {
            auto waitStatus = waitUntilReady(true, 0);
            if (waitStatus < 0)
            {
                canReadLine = false;
                break;
            }
            else if (waitStatus == 0)
            {
                wait(100);
                continue;
            }
            sizeRead += read(line + sizeRead, 1, false);
        }

        if (canReadLine)
        {
            String param(line);
            processLine(param);
        }
    }
}

void LR_IPC_IN::processLine(String& line)
{
    line.trimEnd();
    String command = line.upToFirstOccurrenceOf(" ", false, false);
    String valueString = line.replace(line.upToFirstOccurrenceOf(" ", true, true), "", true);
    auto value = valueString.getIntValue();

    if (CommandMap::getInstance().commandHasAssociatedMessage(command))
    {
        MIDI_Message& msg = CommandMap::getInstance().getMessageForCommand(command);
        MIDISender::getInstance().queueCCForSending(msg.channel, msg.controller, value);
    }
}