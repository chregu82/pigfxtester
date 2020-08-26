#include    <windows.h>
#include    <stdlib.h>
#include    <stdio.h>
#include    <string.h>
#include    <commdlg.h>
#include    <time.h>

#define BARyPOS 440
#define BARwidth  80
#define BALLwh  10

#define BRICKheight 20
#define BRICKwidth  70

#define INITALbarPOS 280

enum
{
    spIdxBall,
    spIdxBar,
    spIdxTopBorder,
    spIdxBottomBorder,
    spIdxLeftBorder,
    spIdxRightBorder,
    spIdxGameOver
};

typedef struct
{
    unsigned int x;
    unsigned int y;
} tPos;

typedef struct
{
    tPos pos;
    unsigned int hits;
} tBrick;

int nread,nwrite;
HANDLE hSerial;
unsigned char ballMoving = 0;
unsigned int barPosition = INITALbarPOS;
tPos ballPos;
tBrick brick[100];
unsigned char pause = 0;
char lives;
char score[5];
char ballXspeed = -5;
char ballYspeed = -5;
unsigned char collDetState = 0;
unsigned char getColPartIdx = 0;
unsigned int colDetParners[2];
unsigned char changeDirWithBrick;
unsigned char gameOver = 0;
char* title = "LIVES:                           PiGFX Breakout                      SCORE: 0000";

void fSend(void* p, unsigned int size)
{
    DWORD dummy;
    // send
    if (!WriteFile(hSerial, p, size, &dummy, NULL))
    {
        printf("error writing to output buffer \n");
    }
}

void fSendUnicolorBitmap(unsigned char idx, unsigned int w, unsigned int h, unsigned char color)
{
    char tmpString[10];
    char sendbuff[1024];
    unsigned int pos;

    strcpy(sendbuff, "\e[#");

    itoa(idx, tmpString, 10);
    strcat(sendbuff, tmpString);
    strcat(sendbuff, ";");

    itoa(w, tmpString, 10);
    strcat(sendbuff, tmpString);
    strcat(sendbuff, ";");

    itoa(h, tmpString, 10);
    strcat(sendbuff, tmpString);
    strcat(sendbuff, "B");

    pos = strlen(sendbuff);

    unsigned int nbrPixels = w*h;

    while (nbrPixels)
    {
        sendbuff[pos++] = color;
        if (nbrPixels > 255)
        {
            sendbuff[pos++] = 255;
            nbrPixels-=255;
        }
        else
        {
            sendbuff[pos++] = (unsigned char)nbrPixels;
            nbrPixels = 0;
            sendbuff[pos++] = 0;
        }
    }
    fSend(sendbuff, pos);
}

void fSendUnicolorBitmapWithBorder(unsigned char idx, unsigned int w, unsigned int h, unsigned char color)
{
    char tmpString[10];
    char sendbuff[1024];
    unsigned int pos;

    strcpy(sendbuff, "\e[#");

    itoa(idx, tmpString, 10);
    strcat(sendbuff, tmpString);
    strcat(sendbuff, ";");

    itoa(w, tmpString, 10);
    strcat(sendbuff, tmpString);
    strcat(sendbuff, ";");

    itoa(h, tmpString, 10);
    strcat(sendbuff, tmpString);
    strcat(sendbuff, "B");

    pos = strlen(sendbuff);

    unsigned int usedW = w-2;
    unsigned int usedH = h-2;

    sendbuff[pos++] = 0;
    sendbuff[pos++] = w;

    for (unsigned int i=0;i<usedH;i++)
    {
        sendbuff[pos++] = 0;
        sendbuff[pos++] = 1;
        sendbuff[pos++] = color;
        sendbuff[pos++] = usedW;
        sendbuff[pos++] = 0;
        sendbuff[pos++] = 1;
    }

    sendbuff[pos++] = 0;
    sendbuff[pos++] = w;

    fSend(sendbuff, pos);
}

void fCreateBallBitmap(unsigned char idx, unsigned char color)
{
    unsigned int w = BALLwh;
    unsigned int h = BALLwh;
    char tmpString[10];
    char sendbuff[1024];
    unsigned int pos;
    strcpy(sendbuff, "\e[#");

    itoa(idx, tmpString, 10);
    strcat(sendbuff, tmpString);
    strcat(sendbuff, ";");

    itoa(w, tmpString, 10);
    strcat(sendbuff, tmpString);
    strcat(sendbuff, ";");

    itoa(h, tmpString, 10);
    strcat(sendbuff, tmpString);
    strcat(sendbuff, "B");

    pos = strlen(sendbuff);

    sendbuff[pos++] = 0;
    sendbuff[pos++] = 3;
    sendbuff[pos++] = color;
    sendbuff[pos++] = 4;
    sendbuff[pos++] = 0;
    sendbuff[pos++] = 4;
    sendbuff[pos++] = color;
    sendbuff[pos++] = 8;
    sendbuff[pos++] = 0;
    sendbuff[pos++] = 2;
    sendbuff[pos++] = color;
    sendbuff[pos++] = 8;
    sendbuff[pos++] = 0;
    sendbuff[pos++] = 1;
    sendbuff[pos++] = color;
    sendbuff[pos++] = 40;
    sendbuff[pos++] = 0;
    sendbuff[pos++] = 1;
    sendbuff[pos++] = color;
    sendbuff[pos++] = 8;
    sendbuff[pos++] = 0;
    sendbuff[pos++] = 2;
    sendbuff[pos++] = color;
    sendbuff[pos++] = 8;
    sendbuff[pos++] = 0;
    sendbuff[pos++] = 4;
    sendbuff[pos++] = color;
    sendbuff[pos++] = 4;
    sendbuff[pos++] = 0;
    sendbuff[pos++] = 3;

    fSend(sendbuff, pos);
}

void fCreateGameOverBitmap(unsigned char idx)
{
    unsigned int w = 370;
    unsigned int h = 220;//220;
    char tmpString[10];
    char sendbuff[100];
    unsigned int pos;
    strcpy(sendbuff, "\e[#");

    itoa(idx, tmpString, 10);
    strcat(sendbuff, tmpString);
    strcat(sendbuff, ";");

    itoa(w, tmpString, 10);
    strcat(sendbuff, tmpString);
    strcat(sendbuff, ";");

    itoa(h, tmpString, 10);
    strcat(sendbuff, tmpString);
    strcat(sendbuff, "B");

    pos = strlen(sendbuff);

    fSend(sendbuff, pos);

    char pict [] = {0, 255, 0, 255,0, 255,0, 255,0, 255,0, 255,0, 255,0, 255,0, 255,0, 255,0, 255,0, 255,0, 255,0, 255, 0, 130,
                    0, 20, 2, 60, 0, 50, 2, 20, 0, 40, 2, 20, 0, 40, 2, 20, 0, 10, 2, 80, 0, 10, 0, 20, 2, 60, 0, 50, 2, 20, 0, 40, 2, 20, 0, 40, 2, 20, 0, 10, 2, 80, 0, 10,0, 20, 2, 60, 0, 50, 2, 20, 0, 40, 2, 20, 0, 40, 2, 20, 0, 10, 2, 80, 0, 10,0, 20, 2, 60, 0, 50, 2, 20, 0, 40, 2, 20, 0, 40, 2, 20, 0, 10, 2, 80, 0, 10,0, 20, 2, 60, 0, 50, 2, 20, 0, 40, 2, 20, 0, 40, 2, 20, 0, 10, 2, 80, 0, 10,0, 20, 2, 60, 0, 50, 2, 20, 0, 40, 2, 20, 0, 40, 2, 20, 0, 10, 2, 80, 0, 10,0, 20, 2, 60, 0, 50, 2, 20, 0, 40, 2, 20, 0, 40, 2, 20, 0, 10, 2, 80, 0, 10,0, 20, 2, 60, 0, 50, 2, 20, 0, 40, 2, 20, 0, 40, 2, 20, 0, 10, 2, 80, 0, 10,0, 20, 2, 60, 0, 50, 2, 20, 0, 40, 2, 20, 0, 40, 2, 20, 0, 10, 2, 80, 0, 10,0, 20, 2, 60, 0, 50, 2, 20, 0, 40, 2, 20, 0, 40, 2, 20, 0, 10, 2, 80, 0, 10,
                    0, 10, 2, 80, 0, 30, 2, 40, 0, 30, 2, 30, 0, 20, 2, 30, 0, 10, 2, 80, 0, 10, 0, 10, 2, 80, 0, 30, 2, 40, 0, 30, 2, 30, 0, 20, 2, 30, 0, 10, 2, 80, 0, 10,0, 10, 2, 80, 0, 30, 2, 40, 0, 30, 2, 30, 0, 20, 2, 30, 0, 10, 2, 80, 0, 10,0, 10, 2, 80, 0, 30, 2, 40, 0, 30, 2, 30, 0, 20, 2, 30, 0, 10, 2, 80, 0, 10,0, 10, 2, 80, 0, 30, 2, 40, 0, 30, 2, 30, 0, 20, 2, 30, 0, 10, 2, 80, 0, 10,0, 10, 2, 80, 0, 30, 2, 40, 0, 30, 2, 30, 0, 20, 2, 30, 0, 10, 2, 80, 0, 10,0, 10, 2, 80, 0, 30, 2, 40, 0, 30, 2, 30, 0, 20, 2, 30, 0, 10, 2, 80, 0, 10,0, 10, 2, 80, 0, 30, 2, 40, 0, 30, 2, 30, 0, 20, 2, 30, 0, 10, 2, 80, 0, 10,0, 10, 2, 80, 0, 30, 2, 40, 0, 30, 2, 30, 0, 20, 2, 30, 0, 10, 2, 80, 0, 10,0, 10, 2, 80, 0, 30, 2, 40, 0, 30, 2, 30, 0, 20, 2, 30, 0, 10, 2, 80, 0, 10,
                    0, 10, 2, 20, 0, 80, 2, 60, 0, 20, 2, 80, 0, 10, 2, 20, 0, 70, 0, 10, 2, 20, 0, 80, 2, 60, 0, 20, 2, 80, 0, 10, 2, 20, 0, 70, 0, 10, 2, 20, 0, 80, 2, 60, 0, 20, 2, 80, 0, 10, 2, 20, 0, 70, 0, 10, 2, 20, 0, 80, 2, 60, 0, 20, 2, 80, 0, 10, 2, 20, 0, 70, 0, 10, 2, 20, 0, 80, 2, 60, 0, 20, 2, 80, 0, 10, 2, 20, 0, 70, 0, 10, 2, 20, 0, 80, 2, 60, 0, 20, 2, 80, 0, 10, 2, 20, 0, 70, 0, 10, 2, 20, 0, 80, 2, 60, 0, 20, 2, 80, 0, 10, 2, 20, 0, 70, 0, 10, 2, 20, 0, 80, 2, 60, 0, 20, 2, 80, 0, 10, 2, 20, 0, 70, 0, 10, 2, 20, 0, 80, 2, 60, 0, 20, 2, 80, 0, 10, 2, 20, 0, 70, 0, 10, 2, 20, 0, 80, 2, 60, 0, 20, 2, 80, 0, 10, 2, 20, 0, 70,
                    0, 10, 2, 20, 0, 70, 2, 80, 0, 10, 2, 80, 0, 10, 2, 20, 0, 70, 0, 10, 2, 20, 0, 70, 2, 80, 0, 10, 2, 80, 0, 10, 2, 20, 0, 70, 0, 10, 2, 20, 0, 70, 2, 80, 0, 10, 2, 80, 0, 10, 2, 20, 0, 70, 0, 10, 2, 20, 0, 70, 2, 80, 0, 10, 2, 80, 0, 10, 2, 20, 0, 70, 0, 10, 2, 20, 0, 70, 2, 80, 0, 10, 2, 80, 0, 10, 2, 20, 0, 70, 0, 10, 2, 20, 0, 70, 2, 80, 0, 10, 2, 80, 0, 10, 2, 20, 0, 70, 0, 10, 2, 20, 0, 70, 2, 80, 0, 10, 2, 80, 0, 10, 2, 20, 0, 70, 0, 10, 2, 20, 0, 70, 2, 80, 0, 10, 2, 80, 0, 10, 2, 20, 0, 70, 0, 10, 2, 20, 0, 70, 2, 80, 0, 10, 2, 80, 0, 10, 2, 20, 0, 70, 0, 10, 2, 20, 0, 70, 2, 80, 0, 10, 2, 80, 0, 10, 2, 20, 0, 70,
                    0, 10, 2, 20, 0, 20, 2, 40, 0, 10, 2, 30, 0, 20, 2, 30, 0, 10, 2, 20, 0, 10, 2, 20, 0, 10, 2, 20, 0, 10, 2, 50, 0, 40, 0, 10, 2, 20, 0, 20, 2, 40, 0, 10, 2, 30, 0, 20, 2, 30, 0, 10, 2, 20, 0, 10, 2, 20, 0, 10, 2, 20, 0, 10, 2, 50, 0, 40, 0, 10, 2, 20, 0, 20, 2, 40, 0, 10, 2, 30, 0, 20, 2, 30, 0, 10, 2, 20, 0, 10, 2, 20, 0, 10, 2, 20, 0, 10, 2, 50, 0, 40, 0, 10, 2, 20, 0, 20, 2, 40, 0, 10, 2, 30, 0, 20, 2, 30, 0, 10, 2, 20, 0, 10, 2, 20, 0, 10, 2, 20, 0, 10, 2, 50, 0, 40, 0, 10, 2, 20, 0, 20, 2, 40, 0, 10, 2, 30, 0, 20, 2, 30, 0, 10, 2, 20, 0, 10, 2, 20, 0, 10, 2, 20, 0, 10, 2, 50, 0, 40, 0, 10, 2, 20, 0, 20, 2, 40, 0, 10, 2, 30, 0, 20, 2, 30, 0, 10, 2, 20, 0, 10, 2, 20, 0, 10, 2, 20, 0, 10, 2, 50, 0, 40, 0, 10, 2, 20, 0, 20, 2, 40, 0, 10, 2, 30, 0, 20, 2, 30, 0, 10, 2, 20, 0, 10, 2, 20, 0, 10, 2, 20, 0, 10, 2, 50, 0, 40, 0, 10, 2, 20, 0, 20, 2, 40, 0, 10, 2, 30, 0, 20, 2, 30, 0, 10, 2, 20, 0, 10, 2, 20, 0, 10, 2, 20, 0, 10, 2, 50, 0, 40, 0, 10, 2, 20, 0, 20, 2, 40, 0, 10, 2, 30, 0, 20, 2, 30, 0, 10, 2, 20, 0, 10, 2, 20, 0, 10, 2, 20, 0, 10, 2, 50, 0, 40, 0, 10, 2, 20, 0, 20, 2, 40, 0, 10, 2, 30, 0, 20, 2, 30, 0, 10, 2, 20, 0, 10, 2, 20, 0, 10, 2, 20, 0, 10, 2, 50, 0, 40,
                    0, 10, 2, 20, 0, 20, 2, 40, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 50, 0, 40, 0, 10, 2, 20, 0, 20, 2, 40, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 50, 0, 40, 0, 10, 2, 20, 0, 20, 2, 40, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 50, 0, 40, 0, 10, 2, 20, 0, 20, 2, 40, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 50, 0, 40, 0, 10, 2, 20, 0, 20, 2, 40, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 50, 0, 40, 0, 10, 2, 20, 0, 20, 2, 40, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 50, 0, 40, 0, 10, 2, 20, 0, 20, 2, 40, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 50, 0, 40, 0, 10, 2, 20, 0, 20, 2, 40, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 50, 0, 40, 0, 10, 2, 20, 0, 20, 2, 40, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 50, 0, 40, 0, 10, 2, 20, 0, 20, 2, 40, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 50, 0, 40,
                    0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 80, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 70, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 80, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 70, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 80, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 70, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 80, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 70, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 80, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 70, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 80, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 70, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 80, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 70, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 80, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 70, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 80, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 70, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 80, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 70,
                    0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 80, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 70, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 80, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 70, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 80, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 70, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 80, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 70, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 80, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 70, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 80, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 70, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 80, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 70, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 80, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 70, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 80, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 70, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 80, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 70,
                    0, 10, 2, 80, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 80, 0, 10, 0, 10, 2, 80, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 80, 0, 10, 0, 10, 2, 80, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 80, 0, 10, 0, 10, 2, 80, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 80, 0, 10, 0, 10, 2, 80, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 80, 0, 10, 0, 10, 2, 80, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 80, 0, 10, 0, 10, 2, 80, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 80, 0, 10, 0, 10, 2, 80, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 80, 0, 10, 0, 10, 2, 80, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 80, 0, 10, 0, 10, 2, 80, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 80, 0, 10,
                    0, 20, 2, 60, 0, 20, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 80, 0, 10, 0, 20, 2, 60, 0, 20, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 80, 0, 10, 0, 20, 2, 60, 0, 20, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 80, 0, 10, 0, 20, 2, 60, 0, 20, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 80, 0, 10, 0, 20, 2, 60, 0, 20, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 80, 0, 10, 0, 20, 2, 60, 0, 20, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 80, 0, 10, 0, 20, 2, 60, 0, 20, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 80, 0, 10, 0, 20, 2, 60, 0, 20, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 80, 0, 10, 0, 20, 2, 60, 0, 20, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 80, 0, 10, 0, 20, 2, 60, 0, 20, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 80, 0, 10,
                    0, 255, 0, 255,0, 255,0, 255,0, 255,0, 255,0, 255,0, 255,0, 255,0, 255,0, 255,0, 255,0, 255,0, 255, 0, 130,
                    0, 20, 2, 60, 0, 20, 2, 20, 0, 40, 2, 20, 0, 10, 2, 80, 0, 10, 2, 70, 0, 20, 0, 20, 2, 60, 0, 20, 2, 20, 0, 40, 2, 20, 0, 10, 2, 80, 0, 10, 2, 70, 0, 20, 0, 20, 2, 60, 0, 20, 2, 20, 0, 40, 2, 20, 0, 10, 2, 80, 0, 10, 2, 70, 0, 20, 0, 20, 2, 60, 0, 20, 2, 20, 0, 40, 2, 20, 0, 10, 2, 80, 0, 10, 2, 70, 0, 20, 0, 20, 2, 60, 0, 20, 2, 20, 0, 40, 2, 20, 0, 10, 2, 80, 0, 10, 2, 70, 0, 20, 0, 20, 2, 60, 0, 20, 2, 20, 0, 40, 2, 20, 0, 10, 2, 80, 0, 10, 2, 70, 0, 20, 0, 20, 2, 60, 0, 20, 2, 20, 0, 40, 2, 20, 0, 10, 2, 80, 0, 10, 2, 70, 0, 20, 0, 20, 2, 60, 0, 20, 2, 20, 0, 40, 2, 20, 0, 10, 2, 80, 0, 10, 2, 70, 0, 20, 0, 20, 2, 60, 0, 20, 2, 20, 0, 40, 2, 20, 0, 10, 2, 80, 0, 10, 2, 70, 0, 20, 0, 20, 2, 60, 0, 20, 2, 20, 0, 40, 2, 20, 0, 10, 2, 80, 0, 10, 2, 70, 0, 20,
                    0, 10, 2, 80, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 80, 0, 10, 2, 80, 0, 10, 0, 10, 2, 80, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 80, 0, 10, 2, 80, 0, 10, 0, 10, 2, 80, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 80, 0, 10, 2, 80, 0, 10, 0, 10, 2, 80, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 80, 0, 10, 2, 80, 0, 10, 0, 10, 2, 80, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 80, 0, 10, 2, 80, 0, 10, 0, 10, 2, 80, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 80, 0, 10, 2, 80, 0, 10, 0, 10, 2, 80, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 80, 0, 10, 2, 80, 0, 10, 0, 10, 2, 80, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 80, 0, 10, 2, 80, 0, 10, 0, 10, 2, 80, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 80, 0, 10, 2, 80, 0, 10, 0, 10, 2, 80, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 80, 0, 10, 2, 80, 0, 10,
                    0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 70, 2, 20, 0, 40, 2, 20, 0, 10, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 70, 2, 20, 0, 40, 2, 20, 0, 10, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 70, 2, 20, 0, 40, 2, 20, 0, 10, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 70, 2, 20, 0, 40, 2, 20, 0, 10, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 70, 2, 20, 0, 40, 2, 20, 0, 10, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 70, 2, 20, 0, 40, 2, 20, 0, 10, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 70, 2, 20, 0, 40, 2, 20, 0, 10, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 70, 2, 20, 0, 40, 2, 20, 0, 10, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 70, 2, 20, 0, 40, 2, 20, 0, 10, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 70, 2, 20, 0, 40, 2, 20, 0, 10,
                    0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 70, 2, 20, 0, 40, 2, 20, 0, 10, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 70, 2, 20, 0, 40, 2, 20, 0, 10, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 70, 2, 20, 0, 40, 2, 20, 0, 10, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 70, 2, 20, 0, 40, 2, 20, 0, 10, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 70, 2, 20, 0, 40, 2, 20, 0, 10, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 70, 2, 20, 0, 40, 2, 20, 0, 10, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 70, 2, 20, 0, 40, 2, 20, 0, 10, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 70, 2, 20, 0, 40, 2, 20, 0, 10, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 70, 2, 20, 0, 40, 2, 20, 0, 10, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 70, 2, 20, 0, 40, 2, 20, 0, 10,
                    0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 50, 0, 40, 2, 80, 0, 10, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 50, 0, 40, 2, 80, 0, 10, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 50, 0, 40, 2, 80, 0, 10, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 50, 0, 40, 2, 80, 0, 10, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 50, 0, 40, 2, 80, 0, 10, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 50, 0, 40, 2, 80, 0, 10, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 50, 0, 40, 2, 80, 0, 10, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 50, 0, 40, 2, 80, 0, 10, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 50, 0, 40, 2, 80, 0, 10, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 50, 0, 40, 2, 80, 0, 10,
                    0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 50, 0, 40, 2, 70, 0, 20, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 50, 0, 40, 2, 70, 0, 20, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 50, 0, 40, 2, 70, 0, 20, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 50, 0, 40, 2, 70, 0, 20, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 50, 0, 40, 2, 70, 0, 20, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 50, 0, 40, 2, 70, 0, 20, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 50, 0, 40, 2, 70, 0, 20, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 50, 0, 40, 2, 70, 0, 20, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 50, 0, 40, 2, 70, 0, 20, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 2, 50, 0, 40, 2, 70, 0, 20,
                    0, 10, 2, 20, 0, 40, 2, 20, 0, 20, 2, 60, 0, 20, 2, 20, 0, 70, 2, 20, 0, 40, 2, 20, 0, 10, 0, 10, 2, 20, 0, 40, 2, 20, 0, 20, 2, 60, 0, 20, 2, 20, 0, 70, 2, 20, 0, 40, 2, 20, 0, 10, 0, 10, 2, 20, 0, 40, 2, 20, 0, 20, 2, 60, 0, 20, 2, 20, 0, 70, 2, 20, 0, 40, 2, 20, 0, 10, 0, 10, 2, 20, 0, 40, 2, 20, 0, 20, 2, 60, 0, 20, 2, 20, 0, 70, 2, 20, 0, 40, 2, 20, 0, 10, 0, 10, 2, 20, 0, 40, 2, 20, 0, 20, 2, 60, 0, 20, 2, 20, 0, 70, 2, 20, 0, 40, 2, 20, 0, 10, 0, 10, 2, 20, 0, 40, 2, 20, 0, 20, 2, 60, 0, 20, 2, 20, 0, 70, 2, 20, 0, 40, 2, 20, 0, 10, 0, 10, 2, 20, 0, 40, 2, 20, 0, 20, 2, 60, 0, 20, 2, 20, 0, 70, 2, 20, 0, 40, 2, 20, 0, 10, 0, 10, 2, 20, 0, 40, 2, 20, 0, 20, 2, 60, 0, 20, 2, 20, 0, 70, 2, 20, 0, 40, 2, 20, 0, 10, 0, 10, 2, 20, 0, 40, 2, 20, 0, 20, 2, 60, 0, 20, 2, 20, 0, 70, 2, 20, 0, 40, 2, 20, 0, 10, 0, 10, 2, 20, 0, 40, 2, 20, 0, 20, 2, 60, 0, 20, 2, 20, 0, 70, 2, 20, 0, 40, 2, 20, 0, 10,
                    0, 10, 2, 20, 0, 40, 2, 20, 0, 20, 2, 60, 0, 20, 2, 20, 0, 70, 2, 20, 0, 40, 2, 20, 0, 10, 0, 10, 2, 20, 0, 40, 2, 20, 0, 20, 2, 60, 0, 20, 2, 20, 0, 70, 2, 20, 0, 40, 2, 20, 0, 10, 0, 10, 2, 20, 0, 40, 2, 20, 0, 20, 2, 60, 0, 20, 2, 20, 0, 70, 2, 20, 0, 40, 2, 20, 0, 10, 0, 10, 2, 20, 0, 40, 2, 20, 0, 20, 2, 60, 0, 20, 2, 20, 0, 70, 2, 20, 0, 40, 2, 20, 0, 10, 0, 10, 2, 20, 0, 40, 2, 20, 0, 20, 2, 60, 0, 20, 2, 20, 0, 70, 2, 20, 0, 40, 2, 20, 0, 10, 0, 10, 2, 20, 0, 40, 2, 20, 0, 20, 2, 60, 0, 20, 2, 20, 0, 70, 2, 20, 0, 40, 2, 20, 0, 10, 0, 10, 2, 20, 0, 40, 2, 20, 0, 20, 2, 60, 0, 20, 2, 20, 0, 70, 2, 20, 0, 40, 2, 20, 0, 10, 0, 10, 2, 20, 0, 40, 2, 20, 0, 20, 2, 60, 0, 20, 2, 20, 0, 70, 2, 20, 0, 40, 2, 20, 0, 10, 0, 10, 2, 20, 0, 40, 2, 20, 0, 20, 2, 60, 0, 20, 2, 20, 0, 70, 2, 20, 0, 40, 2, 20, 0, 10, 0, 10, 2, 20, 0, 40, 2, 20, 0, 20, 2, 60, 0, 20, 2, 20, 0, 70, 2, 20, 0, 40, 2, 20, 0, 10,
                    0, 10, 2, 80, 0, 30, 2, 40, 0, 30, 2, 80, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 0, 10, 2, 80, 0, 30, 2, 40, 0, 30, 2, 80, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 0, 10, 2, 80, 0, 30, 2, 40, 0, 30, 2, 80, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 0, 10, 2, 80, 0, 30, 2, 40, 0, 30, 2, 80, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 0, 10, 2, 80, 0, 30, 2, 40, 0, 30, 2, 80, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 0, 10, 2, 80, 0, 30, 2, 40, 0, 30, 2, 80, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 0, 10, 2, 80, 0, 30, 2, 40, 0, 30, 2, 80, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 0, 10, 2, 80, 0, 30, 2, 40, 0, 30, 2, 80, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 0, 10, 2, 80, 0, 30, 2, 40, 0, 30, 2, 80, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 0, 10, 2, 80, 0, 30, 2, 40, 0, 30, 2, 80, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10,
                    0, 20, 2, 60, 0, 50, 2, 20, 0, 40, 2, 80, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 0, 20, 2, 60, 0, 50, 2, 20, 0, 40, 2, 80, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 0, 20, 2, 60, 0, 50, 2, 20, 0, 40, 2, 80, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 0, 20, 2, 60, 0, 50, 2, 20, 0, 40, 2, 80, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 0, 20, 2, 60, 0, 50, 2, 20, 0, 40, 2, 80, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 0, 20, 2, 60, 0, 50, 2, 20, 0, 40, 2, 80, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 0, 20, 2, 60, 0, 50, 2, 20, 0, 40, 2, 80, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 0, 20, 2, 60, 0, 50, 2, 20, 0, 40, 2, 80, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 0, 20, 2, 60, 0, 50, 2, 20, 0, 40, 2, 80, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10, 0, 20, 2, 60, 0, 50, 2, 20, 0, 40, 2, 80, 0, 10, 2, 20, 0, 40, 2, 20, 0, 10};
    fSend(pict, sizeof(pict));

}

void fClearScreen()
{
    char* p = "\e[2J";
    fSend(p, strlen(p));
}

void fUpdateLives(char lives)
{
    char tmpStr[20];
    strcpy(tmpStr, "\e[1;8H");
    unsigned int pos = strlen(tmpStr);
    tmpStr[pos++] = lives;
    tmpStr[pos++] = 0;
    fSend(tmpStr, pos);
}

void fDisplayPause(char state)
{
    char tmpStr[20];
    strcpy(tmpStr, "\e[1;19H");
    if (state == 1) strcat(tmpStr, "PAUSE");
    else strcat(tmpStr, "     ");
    fSend(tmpStr, strlen(tmpStr));
}

void fUpdateScore(char* score)
{
    char tmpStr[20];
    strcpy(tmpStr, "\e[1;77H");

    score[3]++;
    if (score[3] > '9')
    {
        score[2]++;
        score[3] = '0';
    }
    if (score[2] > '9')
    {
        score[1]++;
        score[2] = '0';
    }
    if (score[1] > '9')
    {
        score[0]++;
        score[1] = '0';
    }

    strcat(tmpStr, score);

    fSend(tmpStr, strlen(tmpStr));
}

void fDisableCursor()
{
    char* p = "\e[?25l";
    fSend(p, strlen(p));
}

void fPutSprite(unsigned char idx, unsigned char bitmap, unsigned int x, unsigned int y)
{
    char tmpString[10];
    char sendbuff[30];

    strcpy(sendbuff, "\e[#");

    itoa(idx, tmpString, 10);
    strcat(sendbuff, tmpString);
    strcat(sendbuff, ";");

    itoa(bitmap, tmpString, 10);
    strcat(sendbuff, tmpString);
    strcat(sendbuff, ";");

    itoa(x, tmpString, 10);
    strcat(sendbuff, tmpString);
    strcat(sendbuff, ";");

    itoa(y, tmpString, 10);
    strcat(sendbuff, tmpString);
    strcat(sendbuff, "s");

    fSend(sendbuff, strlen(sendbuff));
}

void fMoveSprite(unsigned char idx, unsigned int x, unsigned int y)
{
    char tmpString[10];
    char sendbuff[50];

    strcpy(sendbuff, "\e[#");

    itoa(idx, tmpString, 10);
    strcat(sendbuff, tmpString);
    strcat(sendbuff, ";");

    itoa(x, tmpString, 10);
    strcat(sendbuff, tmpString);
    strcat(sendbuff, ";");

    itoa(y, tmpString, 10);
    strcat(sendbuff, tmpString);
    strcat(sendbuff, "m");

    fSend(sendbuff, strlen(sendbuff));
}

void fRemoveSprite(unsigned char idx)
{
    char tmpString[10];
    char sendbuff[50];

    strcpy(sendbuff, "\e[#");

    itoa(idx, tmpString, 10);
    strcat(sendbuff, tmpString);
    strcat(sendbuff, "x");

    fSend(sendbuff, strlen(sendbuff));
}

void fMoveBar(int move)
{
    if (move == 0) return;
    barPosition+=move;
    fMoveSprite(spIdxBar, barPosition, BARyPOS);

    if (ballMoving == 0)
    {
        // update ballpos
        ballPos.x += move;
        fMoveSprite(spIdxBall, ballPos.x, ballPos.y);
    }
}

void fDrawBricks()
{
    // draw first line
    unsigned char nbrSprites = 0;
    for (unsigned int j=0; j<7; j++)
    {
        for (unsigned int i=0; i<=7; i++)
        {
            unsigned int idx = j*8+i;
            brick[idx].pos.x = 40+i*BRICKwidth;
            brick[idx].pos.y = 60+j*BRICKheight;
            brick[idx].hits = 0;
            fPutSprite(10+nbrSprites++, j, brick[idx].pos.x, brick[idx].pos.y);
        }
    }
}

void fInitGame()
{
    ballMoving = 0;
    barPosition = INITALbarPOS;
    ballXspeed = -5;
    ballYspeed = -5;

    fDisableCursor();
    fClearScreen();

    fSend(title, strlen(title));

    lives = '4';
    fUpdateLives(lives);

    strcpy(score, "0000");

    ballPos.x = barPosition + BARwidth/2-10;        // on position 30
    ballPos.y = BARyPOS-BALLwh;

    // Put ball
    fPutSprite(spIdxBall, 8, ballPos.x, ballPos.y);

    // Put bar
    fPutSprite(spIdxBar, 7, barPosition, BARyPOS);

    // Put top border
    fPutSprite(spIdxTopBorder, 9, 0, 20);

    // Put bottom border
    fPutSprite(spIdxBottomBorder, 9, 0, 479);

    // Put left border
    fPutSprite(spIdxLeftBorder, 10, 0, 21);

    // Put right border
    fPutSprite(spIdxRightBorder, 10, 639, 21);

    fDrawBricks();
}

int main()
{
    COMMTIMEOUTS timeouts;
    COMMCONFIG dcbSerialParams;
    DWORD dwBytesRead;

    hSerial = CreateFile("COM5",GENERIC_READ | GENERIC_WRITE,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);

    if ( hSerial == INVALID_HANDLE_VALUE)
    {
        if (GetLastError() == ERROR_FILE_NOT_FOUND)
        {
            printf(" serial port does not exist \n");
        }
        printf(" some other error occured. Inform user.\n");
    }

    if (!GetCommState(hSerial, &dcbSerialParams.dcb))
    {
        printf("error getting state \n");
    }

    dcbSerialParams.dcb.DCBlength = sizeof(dcbSerialParams.dcb);


    dcbSerialParams.dcb.BaudRate = CBR_57600;
    dcbSerialParams.dcb.ByteSize = 8;
    dcbSerialParams.dcb.StopBits = ONESTOPBIT;
    dcbSerialParams.dcb.Parity = NOPARITY;

    dcbSerialParams.dcb.fBinary = TRUE;
    dcbSerialParams.dcb.fDtrControl = DTR_CONTROL_DISABLE;
    dcbSerialParams.dcb.fRtsControl = RTS_CONTROL_DISABLE;
    dcbSerialParams.dcb.fOutxCtsFlow = FALSE;
    dcbSerialParams.dcb.fOutxDsrFlow = FALSE;
    dcbSerialParams.dcb.fDsrSensitivity= FALSE;
    dcbSerialParams.dcb.fAbortOnError = TRUE;

    if (!SetCommState(hSerial, &dcbSerialParams.dcb))
    {
        printf(" error setting serial port state \n");
    }


    GetCommTimeouts(hSerial,&timeouts);

    /*timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier= 10;*/
    timeouts.ReadIntervalTimeout = 1;
    timeouts.ReadTotalTimeoutConstant = 1;
    timeouts.ReadTotalTimeoutMultiplier = 1;
    timeouts.WriteTotalTimeoutConstant = 5;
    timeouts.WriteTotalTimeoutMultiplier= 5;

    if(!SetCommTimeouts(hSerial, &timeouts))
    {
        printf("error setting port state \n");
    }

    fSendUnicolorBitmapWithBorder(0, BRICKwidth, BRICKheight, 21); // blue
    fSendUnicolorBitmapWithBorder(1, BRICKwidth, BRICKheight, 22); // green
    fSendUnicolorBitmapWithBorder(2, BRICKwidth, BRICKheight, 52); // darkred
    fSendUnicolorBitmapWithBorder(3, BRICKwidth, BRICKheight, 58); // orange
    fSendUnicolorBitmapWithBorder(4, BRICKwidth, BRICKheight, 93); // purple
    fSendUnicolorBitmapWithBorder(5, BRICKwidth, BRICKheight, 142); // gold
    fSendUnicolorBitmapWithBorder(6, BRICKwidth, BRICKheight, 202); // orange

    fSendUnicolorBitmap(7, BARwidth, 10, 252); // grey        // balken

    fCreateBallBitmap(8, 13);   // fuchsia

    fSendUnicolorBitmap(9, 640, 1, 0); // grey        // top / bottom end
    fSendUnicolorBitmap(10, 1, 480-20-2, 0); // grey        // top / bottom end
    fCreateGameOverBitmap(11);

    fInitGame();

    unsigned char serialInput;
    time_t lastTime;

    while (1)
    {
        if (!ReadFile(hSerial, &serialInput, 1, &dwBytesRead, NULL))
        {
            printf("error reading from input buffer \n");
        }
        //if (dwBytesRead) printf("Data read from read buffer is %i \n",serialInput);
        if (dwBytesRead)
        {
            if (collDetState)
            {
                if ((serialInput == '[') || (serialInput == '#')) {;} // do nothing
                else if ((serialInput >= '0') && (serialInput <= '9'))
                {
                    colDetParners[getColPartIdx] = colDetParners[getColPartIdx] * 10 + serialInput - '0';
                }
                else if (serialInput == ';')
                {
                    getColPartIdx++;
                }
                else if (serialInput == 'c')
                {
                    // collision detected!
                    //printf("collision between %i and %i\n", colDetParners[0], colDetParners[1]);

                    if (colDetParners[0] == spIdxBall)
                    {
                        // Screen borders
                        if (colDetParners[1] == spIdxLeftBorder)
                        {
                            ballXspeed = ballXspeed * -1;
                        }
                        else if (colDetParners[1] == spIdxTopBorder)
                        {
                            ballYspeed = ballYspeed * -1;
                        }
                        else if (colDetParners[1] == spIdxRightBorder)
                        {
                            ballXspeed = ballXspeed * -1;
                        }
                        else if (colDetParners[1] == spIdxBottomBorder)
                        {
                            lives--;
                            fUpdateLives(lives);

                            ballMoving = 0;
                            barPosition = INITALbarPOS;
                            fMoveSprite(spIdxBar, barPosition, BARyPOS);

                            ballPos.x = barPosition + BARwidth/2-10;        // on position 30
                            ballPos.y = BARyPOS-BALLwh;
                            fMoveSprite(spIdxBall, ballPos.x, ballPos.y);

                            if (lives == '0')
                            {
                                // game over
                                gameOver = 1;
                                fPutSprite(spIdxGameOver, 11, 135, 130);
                            }
                        }

                        // BAR
                        else if (colDetParners[1] == spIdxBar)
                        {
                            //printf("collision with bar , bar x %i, ballpos %i,%i\n", barPosition, ballPos.x, ballPos.y);
                            if (ballPos.x+BALLwh <= barPosition) ballXspeed = ballXspeed * -1;
                            else if (ballPos.x >= barPosition+BARwidth) ballXspeed = ballXspeed * -1;
                            else
                            {
                                if (ballPos.x >= barPosition + 70)
                                {
                                    ballYspeed = -5;
                                    ballXspeed = 10;
                                }
                                else if (ballPos.x >= barPosition + 50)
                                {
                                    ballYspeed = -5;
                                    ballXspeed = 5;
                                }
                                else if (ballPos.x >= barPosition + 40)
                                {
                                    ballYspeed = -10;
                                    ballXspeed = 5;
                                }
                                else if (ballPos.x >= barPosition + 30)
                                {
                                    ballYspeed = -10;
                                    ballXspeed = -5;
                                }
                                else if (ballPos.x >= barPosition + 10)
                                {
                                    ballYspeed = -5;
                                    ballXspeed = -5;
                                }
                                else
                                {
                                    ballYspeed = -5;
                                    ballXspeed = -10;
                                }
                            }
                            // correct penetration
                            if (ballPos.y % 10 > 0) ballPos.y -=5;
                            if (ballPos.x % 10 > 0)
                            {
                                if (ballXspeed > 0) ballPos.x +=5; else ballPos.x -=5;
                            }
                        }
                        else
                        {
                            // collision with brick
                            unsigned int brickIdx = colDetParners[1] - 10;
                            //printf("collision with brick %i , brickpos %i,%i, ballpos %i,%i\n", brickIdx, brick[brickIdx].pos.x, brick[brickIdx].pos.y, ballPos.x, ballPos.y);

                            brick[brickIdx].hits++;

                            fUpdateScore(score);

                            if (changeDirWithBrick == 0) changeDirWithBrick = brickIdx;

                            // remove bricks
                            if (brick[brickIdx].hits >= 1)
                            {
                                fRemoveSprite(brickIdx+10);
                            }

                            unsigned int tmpScore = (score[0]-'0')*1000+(score[1]-'0')*100+(score[2]-'0')*10+score[3]-'0';
                            if (tmpScore % 56 == 0)
                            {
                                // next level
                                ballMoving = 0;
                                barPosition = INITALbarPOS;
                                fMoveSprite(spIdxBar, barPosition, BARyPOS);

                                ballPos.x = barPosition + BARwidth/2-10;        // on position 30
                                ballPos.y = BARyPOS-BALLwh;
                                fMoveSprite(spIdxBall, ballPos.x, ballPos.y);

                                fDrawBricks();
                            }
                        }
                    }

                    collDetState = 0;
                }
                else
                {
                    collDetState = 0;
                }
            }
            else if ((serialInput == 'v') && (!pause) && (!gameOver) && (ballPos.y+BALLwh <= BARyPOS))
            {
                if (barPosition >= 20) fMoveBar(-20);
            }
            else if ((serialInput == 'b') && (!pause) && (!gameOver) && (ballPos.y+BALLwh <= BARyPOS))
            {
                if (barPosition <= 640-BARwidth-20) fMoveBar(20);
            }
            else if (serialInput == 'p')
            {
                ballMoving = !ballMoving;
                pause = !pause;
                fDisplayPause(pause);
            }
            else if ((serialInput == ' ') && (!pause))
            {
                if (gameOver)
                {
                    gameOver = 0;
                    fInitGame();
                }
                else
                {
                    // space starts ball
                    lastTime = clock();       // ms
                    ballMoving = 1;
                }
            }
            else if (serialInput == '\e')
            {
                collDetState = 1;
                getColPartIdx = 0;
                colDetParners[0] = 0;
                colDetParners[1] = 0;
            }
            else
            {
                //printf("Data read from read buffer is %i \n",serialInput);
            }
        }

        if (ballMoving)
        {
            time_t tnow = clock();
            if ((tnow - lastTime) >= 50)
            {
                lastTime = tnow;

                // Change dir after collision
                if (changeDirWithBrick)
                {
                    if (ballPos.y == brick[changeDirWithBrick].pos.y+BRICKheight)
                    {
                        // from below
                        ballYspeed = ballYspeed * -1;
                    }
                    else if (ballPos.y+BALLwh == brick[changeDirWithBrick].pos.y)
                    {
                        // from top
                        ballYspeed = ballYspeed * -1;
                    }
                    else if ((ballPos.x+BALLwh) == brick[changeDirWithBrick].pos.x)
                    {
                        // from left
                        ballXspeed = ballXspeed * -1;

                    }
                    else if (ballPos.x == brick[changeDirWithBrick].pos.x+BRICKwidth)
                    {
                        // from right
                        ballXspeed = ballXspeed * -1;
                    }
                    changeDirWithBrick = 0;
                }

                // move ball
                ballPos.x += ballXspeed;
                ballPos.y += ballYspeed;
                fMoveSprite(spIdxBall, ballPos.x, ballPos.y);
            }
        }
    }

    CloseHandle(hSerial);

    return 0;
}
