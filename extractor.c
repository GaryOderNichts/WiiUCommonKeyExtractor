#include <windows.h>
#include <shlwapi.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "resource.h"

HWND openedFileLabel;
HWND hwndEdit;
HWND copyButton;

char* getCommonKey(char* path)
{
    char* ret = malloc(128); // Support 127 chars for returned messages

    FILE* f = fopen(path, "rb");
    if (f)
    {
        fseek(f, 0xE0, SEEK_SET);

        uint8_t* buf = malloc(16);
        fread(buf, 1, 16, f);

        // If this is the common key
        uint8_t firstCommonKeyByte = 0xd7;
        if (memcmp(buf, &firstCommonKeyByte, 1) != 0)
        {
            free(buf);
            goto otp_error;
        }

        ret = malloc(16 * 2 + 1);
        for (size_t i = 0; i < 16; i++)
            sprintf(ret + i * 2, "%02x", buf[i]);

        ShowWindow(copyButton, 1);

        free(buf);
        return ret;
    }

otp_error:
    ShowWindow(copyButton, 0);
    strcpy(ret, "Invalid otp.bin");
    return ret;
}

void handleOtpOpen(char* filePath)
{
    char fileName[MAX_PATH] = "";
    strcpy(fileName, filePath);
    PathStripPath(fileName);

    SetWindowText(openedFileLabel, fileName);

    char* commonKey = getCommonKey(filePath);
    SetWindowText(hwndEdit, commonKey);
    free(commonKey);
}

void otpOpen(HWND hwnd)
{
    OPENFILENAME ofn;
    char* filePath = malloc(MAX_PATH);
    filePath[0] = '\0';

    ZeroMemory(&ofn, sizeof(ofn));

    ofn.lStructSize = sizeof(ofn); 
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = "Binary Files (*.bin)\0*.bin\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = filePath;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    ofn.lpstrDefExt = "bin";

    if(GetOpenFileName(&ofn))
    {
        handleOtpOpen(filePath);
    }

    free(filePath);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
        case WM_COMMAND:
        if (wParam == IDC_OPEN_OTP)
        {
            otpOpen(hwnd);
        }
        else if (wParam == IDC_COPY)
        {
            char output[128] = ""; 
            GetWindowText(hwndEdit, output, 127);
            const size_t len = strlen(output) + 1;
            HGLOBAL hMem =  GlobalAlloc(GMEM_MOVEABLE, len);
            memcpy(GlobalLock(hMem), output, len);
            GlobalUnlock(hMem);
            OpenClipboard(0);
            EmptyClipboard();
            SetClipboardData(CF_TEXT, hMem);
            CloseClipboard();
        }
        
        break;
        case WM_CLOSE:
            DestroyWindow(hwnd);
        break;
        case WM_DESTROY:
            PostQuitMessage(0);
        break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    WNDCLASSEX wc;
    HWND hwnd;
    MSG Msg;

    wc.cbSize        = sizeof(WNDCLASSEX);
    wc.style         = 0;
    wc.lpfnWndProc   = WndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInstance;
    wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH) (COLOR_WINDOW);
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = "extractorWindowClass";
    wc.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);

    if(!RegisterClassEx(&wc))
    {
        MessageBox(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return -1;
    }

    hwnd = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        "extractorWindowClass",
        "Wii U Common Key Extractor",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, 
        CW_USEDEFAULT, 
        392, 
        92,
        NULL, 
        NULL, 
        hInstance, 
        NULL);

    if(hwnd == NULL)
    {
        MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return -1;
    }

    HWND selectLabel = CreateWindow(
        "STATIC", 
        "Select otp.bin:", 
        WS_CHILD | WS_VISIBLE, 
        8,
        8,
        100,
        25,
        hwnd,
        NULL,
        GetModuleHandle(NULL), 
        NULL);
    SendMessage(selectLabel, WM_SETFONT, (LPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);


    HWND hwndOpenButton = CreateWindow(
        "BUTTON",
        "Open",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        92,
        4,
        100,
        24,
        hwnd,
        (HMENU) IDC_OPEN_OTP,
        (HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE), 
        NULL);
    SendMessage(hwndOpenButton, WM_SETFONT, (LPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);

    openedFileLabel = CreateWindow(
        "STATIC", 
        "",
        WS_CHILD | WS_VISIBLE, 
        194,
        8,
        125,
        25,
        hwnd,
        NULL,
        GetModuleHandle(NULL), 
        NULL);
    SendMessage(openedFileLabel, WM_SETFONT, (LPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);

    HWND commonKeyLabel = CreateWindow(
        "STATIC", "Common Key:", 
        WS_CHILD | WS_VISIBLE, 
        8,
        32,
        100,
        25,
        hwnd,
        NULL,
        GetModuleHandle(NULL), 
        NULL);
    SendMessage(commonKeyLabel, WM_SETFONT, (LPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);

    hwndEdit = CreateWindow(
        "EDIT",
        "No otp.bin",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | ES_READONLY,
        92,
        32,
        225,
        24,
        hwnd,
        NULL,
        (HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE), 
        NULL);
    SendMessage(hwndEdit, WM_SETFONT, (LPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);

    copyButton = CreateWindow(
        "BUTTON",
        "Copy",
        WS_TABSTOP | SW_HIDE | WS_CHILD | BS_DEFPUSHBUTTON,
        320,
        26,
        50,
        24,
        hwnd,
        (HMENU) IDC_COPY,
        (HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE), 
        NULL);
    SendMessage(copyButton, WM_SETFONT, (LPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    if (_argc > 1)
    {
        handleOtpOpen(_argv[1]);
    }

    while(GetMessage(&Msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }
    return Msg.wParam;
}