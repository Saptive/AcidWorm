

//This is the source of the payload which will be downloaded by the exploit payload to the target hosts.
//It has to be compiled with visual c++ 6.0



#include <Windows.h>
#include <stdio.h>


const char* frame1 =
"\n\n ___________________ \n"
"(o_______________))_)\n";


const char* frame2 =
"\n     _________       \n"
"  __/  ______ \\_____ \n"
"(o____/      \\ __))_)\n";


const char* frame3 =
"      ______        \n"
"     / ____ \\       \n"
"  __/ .    . \\_____ \n"
"(o__.'      '.__))_)\n";




void PrintShifted(const char* frame, int offset)
{
    int i, spaces;
    char c;

    for (spaces = 0; spaces < offset; ++spaces)
        putchar(' ');

    i = 0;
    while (frame[i] != '\0')
    {
        c = frame[i];
        putchar(c);
        if (c == '\n')
        {
            for (spaces = 0; spaces < offset; ++spaces)
                putchar(' ');
        }
        ++i;
    }
    fflush(stdout);
}

void PrintCentered(const char* text, int consoleWidth, int row)
{
    COORD pos;
    HANDLE hConsole;
    int len;
    int startX;

    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    len = (int)strlen(text);
    startX = (consoleWidth - len) / 2;
    if (startX < 0) startX = 0;

    pos.X = (SHORT)startX;
    pos.Y = (SHORT)row;
    SetConsoleCursorPosition(hConsole, pos);
    printf("%s", text);
    fflush(stdout);
}


int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
{
    const char* frames[5];
    int frameCount;
    int frameIndex;
    HANDLE hConsole;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    int consoleWidth;
    int wormWidth;
    int startPos;
    int pos;
    int doMove;
    COORD cursorPos;

    AllocConsole();
    freopen("CONOUT$", "w", stdout);
    freopen("CONIN$", "r", stdin);
    freopen("CONOUT$", "w", stderr);

    system("color 06");


    frames[0] = frame1;
    frames[1] = frame2;
    frames[2] = frame3;
    frames[3] = frame2;
    frames[4] = frame1;

    frameCount = 5;

    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole != INVALID_HANDLE_VALUE && GetConsoleScreenBufferInfo(hConsole, &csbi))
    {
        consoleWidth = (int)csbi.dwSize.X;
    }


    wormWidth = 25;
    startPos = consoleWidth - wormWidth;
    if (startPos < 0)
    {
        startPos = 0;
    }


    frameIndex = 0;

    while (1)
    {
        pos = startPos;
        while (pos >= 0)
        {
            system("cls");

            PrintCentered("AcidWorm 2025", consoleWidth, 10);

            cursorPos.X = 0;
            cursorPos.Y = 1;
            SetConsoleCursorPosition(hConsole, cursorPos);

            PrintShifted(frames[frameIndex], pos);

            Sleep(120);

            doMove = (frameIndex == 3);

            frameIndex = (frameIndex + 1) % frameCount;

            if (doMove)
                pos = pos - 1;
        }
    }

    return 0;
}