//// VIDEO TUTORIAL: https://www.youtube.com/watch?v=KrbTEhsK3fo

#include <iostream>
#include <iostream>
#include <Windows.h>
#include <thread>
#include "SerialPort.h"

DWORD OffsetCurrentBPM = 0x01C1A790;
DWORD OffsetCurrentBall = 0x01C1AF10;
DWORD OffsetCurrentLevel = 0x01C1B790;
DWORD OffsetButtonFunction = 0x009D1CDA;
byte newmem[17] = { 0x89, 0x0A, 0x89, 0x15, 0x18, 0x00, 0xE7, 0x0F, 0x40, 0x83, 0xC2, 0x04, 0xE9, 0xCF, 0x1C, 0xB6, 0xF0 };
byte jumpAssembly[6] = { 0xE9, 0x21, 0xE3, 0x49, 0x0F, 0x90 };


float currentBPM;
float currentBall;
int currentLevel;

DWORD pid;
HANDLE AuditionHandle;
LPVOID myAssemblyFunction;
DWORD lastButtonAddress;


SerialPort arduino;


void Space() {
    char* charArray2 = new char[2];
    while (true) {
        ReadProcessMemory(AuditionHandle, (LPCVOID)OffsetCurrentBall, &currentBall, sizeof(currentBall), 0);
        if (currentBall > 0.745 && currentBall < 0.8) {
            charArray2[0] = 's';
            charArray2[1] = '\n';
            arduino.writeSerialPort(charArray2, 2);
            Sleep(500);
        }
        Sleep(1);
    }
}

void MyLoop() {
    while (true) {
        ReadProcessMemory(AuditionHandle, (LPCVOID)OffsetCurrentBall, &currentBall, sizeof(currentBall), 0);
        ReadProcessMemory(AuditionHandle, (LPCVOID)OffsetCurrentBPM, &currentBPM, sizeof(currentBPM), 0);
        ReadProcessMemory(AuditionHandle, (LPCVOID)OffsetCurrentLevel, &currentLevel, sizeof(currentLevel), 0);

        ReadProcessMemory(AuditionHandle, (LPVOID)((DWORD)myAssemblyFunction + 0x1C), &lastButtonAddress, sizeof(lastButtonAddress), 0);

        byte button;
        for (int i = currentLevel - 1 ; i >= 0; i--) {
            ReadProcessMemory(AuditionHandle, (LPVOID)(lastButtonAddress - i * 0x4), &button, sizeof(button), 0);
            char* charArray2 = new char[2];
            charArray2[0] = (int)button +'0';
            charArray2[1] = '\n';
            arduino.writeSerialPort(charArray2, 2);
            Sleep(100);
        }
        Sleep(800);
    }
}
int main()
{
    std::string port = "\\\\.\\COM3";
    arduino.Init(port);

    HWND hWnd = FindWindowA(0, ("Audition"));
    
    GetWindowThreadProcessId(hWnd, &pid);
    AuditionHandle = OpenProcess(PROCESS_ALL_ACCESS, false, pid);

    myAssemblyFunction = VirtualAllocEx(AuditionHandle, NULL, 128, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);

    DWORD myAssemblyFunctionDword = (DWORD)myAssemblyFunction;
    DWORD temp = myAssemblyFunctionDword + 0x1C;
    memcpy(&newmem[4], &temp, sizeof(temp));

    // So o nhay nguoc ve
    DWORD backStep = (DWORD)OffsetButtonFunction + 0x6 - (myAssemblyFunctionDword + sizeof(newmem));
    memcpy(&newmem[13], &backStep, sizeof(backStep));

    // Ghi ham vao vung dia chi xin duoc
    WriteProcessMemory(AuditionHandle, myAssemblyFunction, &newmem, sizeof(newmem), 0);

    // Inject assembly vao ham goc cua game
    DWORD nextStep = myAssemblyFunctionDword - (DWORD)OffsetButtonFunction - 5;
    memcpy(&jumpAssembly[1], &nextStep, sizeof(nextStep));
    WriteProcessMemory(AuditionHandle, (LPVOID)OffsetButtonFunction, &jumpAssembly, sizeof(jumpAssembly), 0);

    std::thread MyThread(MyLoop);
    std::thread MyThread2(Space);
    while (true) {
        Sleep(100);
    }
    return 1;
}

