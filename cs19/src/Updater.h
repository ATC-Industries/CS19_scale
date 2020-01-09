#ifndef UPDATER_H
#define UPDATER_H

#include <Arduino.h>
#include <Update.h>
#include <FS.h>
#include <SD.h>

class Updater {
    private:

    public:
        void performUpdate(Stream &updateSource, size_t updateSize);
        void updateFromFS(fs::FS &fs);
        void rebootEspWithReason(String reason);
};

#endif