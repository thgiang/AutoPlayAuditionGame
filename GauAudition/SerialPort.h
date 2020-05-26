#ifndef SERIALPORT_H
#define SERIALPORT_H

#define ARDUINO_WAIT_TIME 2000
#define MAX_DATA_LENGTH 4096

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

class SerialPort
{
private:
    HANDLE handler;
    bool connected;
    COMSTAT status;
    DWORD errors;
public:
    void Init(std::string portName);
    ~SerialPort();

    int readSerialPort(char* buffer, unsigned int buf_size);
    bool writeSerialPort(char* buffer, unsigned int buf_size);
    bool isConnected();
};

#endif // SERIALPORT_H
