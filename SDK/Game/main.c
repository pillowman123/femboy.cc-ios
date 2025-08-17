#define UNICODE
#define _UNICODE
#include <windows.h>
#include <dwmapi.h>
#include <d2d1.h>
#include <dwrite.h>
#include <windivert.h>
#include <commctrl.h>
#include <windowsx.h>
#include <process.h>
#include <stdbool.h>
#include <math.h>
#include "packet.h"

#define APP_WIDTH 400
#define APP_HEIGHT 450
#define TITLE_BAR_HEIGHT 40
#define BUTTON_WIDTH 30
#define BUTTON_HEIGHT 25
#define MAX_LOG_MESSAGES 100
#define MAX_MESSAGE_LENGTH 256

D2D1_COLOR_F BG_COLOR = {0.05f, 0.1f, 0.15f, 1.0f};
D2D1_COLOR_F TITLE_COLOR = {0.0f, 0.4f, 0.6f, 0.95f};
D2D1_COLOR_F PANEL_COLOR = {0.08f, 0.15f, 0.2f, 0.8f};
D2D1_COLOR_F TXT_COLOR = {0.9f, 0.9f, 0.9f, 1.0f};
D2D1_COLOR_F ACCENT_COLOR = {0.0f, 0.5f, 0.7f, 1.0f};
D2D1_COLOR_F BTN_HOVER_COLOR = {0.1f, 0.6f, 0.8f, 1.0f};

HWND hGodModeCheckbox;
HWND hVanishCheckbox;
HWND hFreezeCheckbox;
HWND hFlyCheckbox;
HWND hNoClipCheckbox;
HWND hGlitchDashCheckbox;
HWND hLogBox;
HWND hClearLog;
HWND minimizeButton;
HWND closeButton;

char logMsgs[MAX_LOG_MESSAGES][MAX_MESSAGE_LENGTH];
int logMsgCount = 0;
BOOL running = TRUE;
CRITICAL_SECTION pktMutex;

ID2D1Factory* d2dFactory = NULL;
ID2D1HwndRenderTarget* renderTarget = NULL;
ID2D1SolidColorBrush* brush = NULL;
ID2D1SolidColorBrush* accentBrush = NULL;
IDWriteFactory* writeFactory = NULL;
IDWriteTextFormat* textFormat = NULL;
IDWriteTextFormat* titleFormat = NULL;
IDWriteTextFormat* subtitleFormat = NULL;
HWND mainWindow;
BOOL isDragging = FALSE;
POINT dragStart;

BOOL vanish = FALSE;
BOOL god_mode = FALSE;
BOOL full_freeze = FALSE;
BOOL fly_mode = FALSE;
BOOL noclip = FALSE;
BOOL glitch_dash = FALSE;

HBRUSH hBtnBrush = NULL;
HBRUSH hBtnHoverBrush = NULL;
int hoveredBtn = 0;

HHOOK keyboardHook = NULL;

void AddLog(const char* msg);
void InitD2D(HWND hwnd);
void MakeControls(HWND hwnd);
void Cleanup();
unsigned __stdcall packet_thread(void* data);
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0) {
        KBDLLHOOKSTRUCT* kbStruct = (KBDLLHOOKSTRUCT*)lParam;
        
        if (wParam == WM_KEYDOWN && kbStruct->vkCode == '3') {
            glitch_dash = !glitch_dash;
            AddLog(glitch_dash ? "Glitch Dash activated" : "Glitch Dash deactivated");
            SendMessage(hGlitchDashCheckbox, BM_SETCHECK, glitch_dash ? BST_CHECKED : BST_UNCHECKED, 0);
        }
    }
    return CallNextHookEx(keyboardHook, nCode, wParam, lParam);
}

void AddLog(const char* msg) {
    if (logMsgCount < MAX_LOG_MESSAGES) {
        strncpy(logMsgs[logMsgCount], msg, MAX_MESSAGE_LENGTH - 1);
        logMsgs[logMsgCount][MAX_MESSAGE_LENGTH - 1] = '\0';
        logMsgCount++;
    } else {
        for (int i = 0; i < MAX_LOG_MESSAGES - 1; i++) {
            strcpy(logMsgs[i], logMsgs[i + 1]);
        }
        strncpy(logMsgs[MAX_LOG_MESSAGES - 1], msg, MAX_MESSAGE_LENGTH - 1);
        logMsgs[MAX_LOG_MESSAGES - 1][MAX_MESSAGE_LENGTH - 1] = '\0';
    }
    
    WCHAR wMsg[MAX_MESSAGE_LENGTH];
    MultiByteToWideChar(CP_ACP, 0, msg, -1, wMsg, MAX_MESSAGE_LENGTH);
    SendMessage(hLogBox, LB_ADDSTRING, 0, (LPARAM)wMsg);
    SendMessage(hLogBox, WM_VSCROLL, SB_BOTTOM, 0);
}

void InitD2D(HWND hwnd) {
    D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &IID_ID2D1Factory, NULL, (void**)&d2dFactory);
    DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, &IID_IDWriteFactory, (IUnknown**)&writeFactory);
    
    RECT rc;
    GetClientRect(hwnd, &rc);
    
    D2D1_SIZE_U size = {rc.right, rc.bottom};
    D2D1_RENDER_TARGET_PROPERTIES rtProps = D2D1_RENDER_TARGET_PROPERTIES_DEFAULT;
    D2D1_HWND_RENDER_TARGET_PROPERTIES hwndProps = {hwnd, size, D2D1_PRESENT_OPTIONS_NONE};
    
    d2dFactory->lpVtbl->CreateHwndRenderTarget(d2dFactory, &rtProps, &hwndProps, &renderTarget);
    
    renderTarget->lpVtbl->CreateSolidColorBrush(renderTarget, &TXT_COLOR, NULL, &brush);
    renderTarget->lpVtbl->CreateSolidColorBrush(renderTarget, &ACCENT_COLOR, NULL, &accentBrush);
    
    writeFactory->lpVtbl->CreateTextFormat(writeFactory, L"Segoe UI", NULL,
        DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL, 14.0f, L"en-us", &textFormat);
    
    writeFactory->lpVtbl->CreateTextFormat(writeFactory, L"Segoe UI", NULL,
        DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL, 24.0f, L"en-us", &titleFormat);
    
    writeFactory->lpVtbl->CreateTextFormat(writeFactory, L"Segoe UI", NULL,
        DWRITE_FONT_WEIGHT_MEDIUM, DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL, 16.0f, L"en-us", &subtitleFormat);
    
    titleFormat->lpVtbl->SetTextAlignment(titleFormat, DWRITE_TEXT_ALIGNMENT_CENTER);
}

void MakeControls(HWND hwnd) {
    HFONT hFont = CreateFontW(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, FF_DONTCARE, L"Segoe UI");
    
    hGodModeCheckbox = CreateWindowW(L"BUTTON", L"God Mode",
        WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
        30, 80, 150, 30, hwnd, (HMENU)201, NULL, NULL);
    
    hVanishCheckbox = CreateWindowW(L"BUTTON", L"Vanish",
        WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
        30, 110, 150, 30, hwnd, (HMENU)202, NULL, NULL);
    
    hFreezeCheckbox = CreateWindowW(L"BUTTON", L"Full Freeze",
        WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
        30, 140, 150, 30, hwnd, (HMENU)203, NULL, NULL);
    
    hFlyCheckbox = CreateWindowW(L"BUTTON", L"Fly Mode",
        WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
        200, 80, 150, 30, hwnd, (HMENU)204, NULL, NULL);
    
    hNoClipCheckbox = CreateWindowW(L"BUTTON", L"No Clip",
        WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
        200, 110, 150, 30, hwnd, (HMENU)205, NULL, NULL);
    
    hGlitchDashCheckbox = CreateWindowW(L"BUTTON", L"Glitch Dash (3)",
        WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
        200, 140, 150, 30, hwnd, (HMENU)206, NULL, NULL);
    
    hLogBox = CreateWindowW(L"LISTBOX", NULL,
        WS_VISIBLE | WS_CHILD | WS_VSCROLL | LBS_NOTIFY | WS_BORDER,
        30, 230, 340, 150, hwnd, NULL, NULL, NULL);
    
    hClearLog = CreateWindowW(L"BUTTON", L"Clear Log",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        30, 390, 100, 30, hwnd, (HMENU)102, NULL, NULL);
    
    SendMessage(hGodModeCheckbox, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(hVanishCheckbox, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(hFreezeCheckbox, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(hFlyCheckbox, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(hNoClipCheckbox, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(hGlitchDashCheckbox, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(hLogBox, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(hClearLog, WM_SETFONT, (WPARAM)hFont, TRUE);
    
    AddLog("Time SOT External v1.1 started");
    AddLog("Press 3 to toggle Glitch Dash");
}

void Cleanup() {
    if (brush) brush->lpVtbl->Release(brush);
    if (accentBrush) accentBrush->lpVtbl->Release(accentBrush);
    if (renderTarget) renderTarget->lpVtbl->Release(renderTarget);
    if (d2dFactory) d2dFactory->lpVtbl->Release(d2dFactory);
    if (textFormat) textFormat->lpVtbl->Release(textFormat);
    if (titleFormat) titleFormat->lpVtbl->Release(titleFormat);
    if (subtitleFormat) subtitleFormat->lpVtbl->Release(subtitleFormat);
    if (writeFactory) writeFactory->lpVtbl->Release(writeFactory);
    if (hBtnBrush) DeleteObject(hBtnBrush);
    if (hBtnHoverBrush) DeleteObject(hBtnHoverBrush);
    if (keyboardHook) UnhookWindowsHookEx(keyboardHook);
    
    DeleteCriticalSection(&pktMutex);
}

unsigned __stdcall packet_thread(void* data) {
    HANDLE handle = WinDivertOpen("true", WINDIVERT_LAYER_NETWORK, 1000, 0);
    if(handle == INVALID_HANDLE_VALUE) {
        AddLog("Failed to open WinDivert handle");
        return 1;
    }
    
    AddLog("Packet interception started");
    Packet pkt;
    
    while(running) {
        EnterCriticalSection(&pktMutex);
        
        if(!WinDivertRecv(handle, pkt.data, 2000, &pkt.size, &pkt.address)) {
            LeaveCriticalSection(&pktMutex);
            continue;
        }
        
        if(pkt.address.Outbound) {
            if(vanish) {
                packet_replace(&pkt, "[Player]", "[Glider]", PACKET_REPLACE_FIRST, "Player hidden");
            }
            
            if (full_freeze) {
                char search[] = { 0x2A, 0x78, 0x00, 0x00, 0x00 };
                int i = packet_find_first(&pkt, search, 5);
                if (i != -1) pkt.data[i - 7] = '0';
            }
            
            if (fly_mode) {
                char fly_sig[] = { 0x1F, 0x8B, 0x08, 0x00 };
                int pos = packet_find_first(&pkt, fly_sig, 4);
                if (pos != -1 && pos < pkt.size - 10) {
                    pkt.data[pos + 8] = 0xC0;
                    pkt.data[pos + 9] = 0x1F;
                }
            }
            
            if (noclip) {
                char clip_sig[] = { 0x45, 0x78, 0x74, 0x65, 0x72, 0x6E, 0x61, 0x6C };
                int pos = packet_find_first(&pkt, clip_sig, 8);
                if (pos != -1 && pos < pkt.size - 12) {
                    pkt.data[pos + 10] = 0xFF;
                    pkt.data[pos + 11] = 0x01;
                }
            }
            
            if (glitch_dash) {
                static int dash_counter = 0;
                dash_counter++;
                
                if (dash_counter % 3 == 0) {
                    char dash_sig[] = { 0x56, 0x65, 0x6C, 0x6F, 0x63, 0x69, 0x74, 0x79 };
                    int pos = packet_find_first(&pkt, dash_sig, 8);
                    if (pos != -1 && pos < pkt.size - 20) {
                        pkt.data[pos + 12] = 0x3F;
                        pkt.data[pos + 13] = 0x40;
                        pkt.data[pos + 14] = 0x00;
                        pkt.data[pos + 15] = 0x00;
                    }
                }
            }
        } else {
            if(god_mode) {
                for (int i = 0; i < pkt.size - 4; i++) {
                    if (pkt.data[i] == '1' && 
                        pkt.data[i+1] == '3' && 
                        pkt.data[i+2] == '.' && 
                        pkt.data[i+3] == 'P') {
                        pkt.data[i] = '0';
                    }
                }
                
                char dmg_sig[] = { 0x44, 0x61, 0x6D, 0x61, 0x67, 0x65 };
                int pos = packet_find_first(&pkt, dmg_sig, 6);
                if (pos != -1 && pos < pkt.size - 10) {
                    pkt.data[pos + 8] = 0x00;
                    pkt.data[pos + 9] = 0x00;
                }
            }
        }
        
        WinDivertHelperCalcChecksums(pkt.data, pkt.size, &pkt.address, 0);
        WinDivertSend(handle, pkt.data, pkt.size, NULL, &pkt.address);
        
        LeaveCriticalSection(&pktMutex);
    }
    
    WinDivertClose(handle);
    AddLog("Packet interception stopped");
    return 0;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch(msg) {
        case WM_CREATE:
            InitD2D(hwnd);
            MakeControls(hwnd);
            return 0;
            
        case WM_COMMAND: {
            switch(LOWORD(wParam)) {
                case 201: 
                    god_mode = (SendMessage(hGodModeCheckbox, BM_GETCHECK, 0, 0) == BST_CHECKED);
                    AddLog(god_mode ? "God Mode enabled [0x7A4D1F60]" : "God Mode disabled");
                    break;
                    
                case 202:
                    vanish = (SendMessage(hVanishCheckbox, BM_GETCHECK, 0, 0) == BST_CHECKED);
                    AddLog(vanish ? "Vanish enabled [0x8C3E2A10]" : "Vanish disabled");
                    break;
                    
                case 203: 
                    full_freeze = (SendMessage(hFreezeCheckbox, BM_GETCHECK, 0, 0) == BST_CHECKED);
                    AddLog(full_freeze ? "Full Freeze enabled [0x9F1B3D70]" : "Full Freeze disabled");
                    break;
                
                case 204:
                    fly_mode = (SendMessage(hFlyCheckbox, BM_GETCHECK, 0, 0) == BST_CHECKED);
                    AddLog(fly_mode ? "Fly Mode enabled [0x6E2C5A80]" : "Fly Mode disabled");
                    break;
                
                case 205: 
                    noclip = (SendMessage(hNoClipCheckbox, BM_GETCHECK, 0, 0) == BST_CHECKED);
                    AddLog(noclip ? "No Clip enabled [0x5D4B8F30]" : "No Clip disabled");
                    break;
                
                case 206: 
                    glitch_dash = (SendMessage(hGlitchDashCheckbox, BM_GETCHECK, 0, 0) == BST_CHECKED);
                    AddLog(glitch_dash ? "Glitch Dash enabled [0x3A7C9E20] - Press 3 to toggle" : "Glitch Dash disabled");
                    break;
                
                case 102:
                    SendMessage(hLogBox, LB_RESETCONTENT, 0, 0);
                    logMsgCount = 0;
                    AddLog("Log cleared");
                    break;
                    
                case 1002:
                    ShowWindow(hwnd, SW_MINIMIZE);
                    break;
                    
                case 1003: 
                    DestroyWindow(hwnd);
                    break;
            }
            return 0;
        }
        
        case WM_LBUTTONDOWN: {
            POINT pt = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
            if(pt.y < TITLE_BAR_HEIGHT) {
                isDragging = TRUE;
                GetCursorPos(&dragStart);
            }
            return 0;
        }
        
        case WM_LBUTTONUP:
            isDragging = FALSE;
            return 0;
            
        case WM_MOUSEMOVE:
            if(isDragging) {
                POINT currentPos;
                GetCursorPos(&currentPos);
                RECT windowRect;
                GetWindowRect(hwnd, &windowRect);
                SetWindowPos(hwnd, NULL,
                    windowRect.left + (currentPos.x - dragStart.x),
                    windowRect.top + (currentPos.y - dragStart.y),
                    0, 0, SWP_NOSIZE | SWP_NOZORDER);
                dragStart = currentPos;
            }
            return 0;
            
        case WM_PAINT: {
            PAINTSTRUCT ps;
            BeginPaint(hwnd, &ps);
            renderTarget->lpVtbl->BeginDraw(renderTarget);
            renderTarget->lpVtbl->Clear(renderTarget, &BG_COLOR);
            
            brush->lpVtbl->SetColor(brush, &TITLE_COLOR);
            
            D2D1_ROUNDED_RECT titleBarRect = {
                {0, 0, APP_WIDTH, TITLE_BAR_HEIGHT},
                8, 8
            };
            renderTarget->lpVtbl->FillRoundedRectangle(renderTarget, &titleBarRect, brush);
            
            for (int i = 0; i < 8; i++) {
                float x = 20.0f + i * 50.0f;
                float height = 10.0f + (i % 3) * 8.0f;
                D2D1_COLOR_F waveColor = {0.0f, 0.5f, 0.7f, 0.9f};
                brush->lpVtbl->SetColor(brush, &waveColor);
                
                D2D1_ROUNDED_RECT waveRect = {
                    {x, TITLE_BAR_HEIGHT, x + 10, TITLE_BAR_HEIGHT + height},
                    5, 5
                };
                renderTarget->lpVtbl->FillRoundedRectangle(renderTarget, &waveRect, brush);
            }
            
            brush->lpVtbl->SetColor(brush, &TXT_COLOR);
            D2D1_RECT_F titleRect = {0, 8, APP_WIDTH, 35};
            renderTarget->lpVtbl->DrawText(
                renderTarget,
                L"TIME SOT EXTERNAL",
                16,
                titleFormat,
                &titleRect,
                brush,
                D2D1_DRAW_TEXT_OPTIONS_NONE
            );
            
            brush->lpVtbl->SetColor(brush, &PANEL_COLOR);
            D2D1_ROUNDED_RECT featureRect = {
                {20, 60, 380, 200},
                5, 5
            };
            renderTarget->lpVtbl->FillRoundedRectangle(renderTarget, &featureRect, brush);
            
            brush->lpVtbl->SetColor(brush, &ACCENT_COLOR);
            D2D1_RECT_F featureTitleRect = {30, 60, 200, 80};
            renderTarget->lpVtbl->DrawText(
                renderTarget,
                L"FEATURES",
                8,
                subtitleFormat,
                &featureTitleRect,
                brush,
                D2D1_DRAW_TEXT_OPTIONS_NONE
            );
            
            brush->lpVtbl->SetColor(brush, &PANEL_COLOR);
            D2D1_ROUNDED_RECT logRect = {
                {20, 210, 380, 430},
                5, 5
            };
            renderTarget->lpVtbl->FillRoundedRectangle(renderTarget, &logRect, brush);
            
            brush->lpVtbl->SetColor(brush, &ACCENT_COLOR);
            D2D1_RECT_F logTitleRect = {30, 210, 200, 230};
            renderTarget->lpVtbl->DrawText(
                renderTarget,
                L"ACTIVITY LOG",
                12,
                subtitleFormat,
                &logTitleRect,
                brush,
                D2D1_DRAW_TEXT_OPTIONS_NONE
            );
            
            D2D1_COLOR_F logoColor = {0.0f, 0.6f, 0.8f, 0.5f};
            brush->lpVtbl->SetColor(brush, &logoColor);
            float logoX = 320;
            float logoY = 110;
            float logoSize = 40;
            
            D2D1_ELLIPSE logo = {
                {logoX, logoY},
                logoSize/2,
                logoSize/2
            };
            renderTarget->lpVtbl->FillEllipse(renderTarget, &logo, brush);
            
            D2D1_COLOR_F innerColor = {0.0f, 0.0f, 0.0f, 0.7f};
            brush->lpVtbl->SetColor(brush, &innerColor);
            
            D2D1_ELLIPSE inner = {
                {logoX, logoY},
                logoSize/3,
                logoSize/3
            };
            renderTarget->lpVtbl->FillEllipse(renderTarget, &inner, brush);
            
            D2D1_COLOR_F centerColor = {0.0f, 0.8f, 1.0f, 0.9f};
            brush->lpVtbl->SetColor(brush, &centerColor);
            
            D2D1_ELLIPSE center = {
                {logoX, logoY},
                logoSize/6,
                logoSize/6
            };
            renderTarget->lpVtbl->FillEllipse(renderTarget, &center, brush);
            
            renderTarget->lpVtbl->EndDraw(renderTarget, NULL, NULL);
            EndPaint(hwnd, &ps);
            return 0;
        }
        
        case WM_CTLCOLORSTATIC:
        case WM_CTLCOLORBTN: {
            HDC hdcStatic = (HDC)wParam;
            SetTextColor(hdcStatic, RGB(230, 230, 230));
            SetBkMode(hdcStatic, TRANSPARENT);
            
            if (!hBtnBrush)
                hBtnBrush = CreateSolidBrush(RGB(15, 30, 40));
                
            return (INT_PTR)hBtnBrush;
        }
        
        case WM_DESTROY:
            running = FALSE;
            Cleanup();
            PostQuitMessage(0);
            return 0;
    }
    
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    INITCOMMONCONTROLSEX icc;
    icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icc.dwICC = ICC_STANDARD_CLASSES;
    InitCommonControlsEx(&icc);
    
    InitializeCriticalSection(&pktMutex);
    
    WNDCLASSW wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"TimeSOTExternalClass";
    wc.hbrBackground = CreateSolidBrush(RGB(10, 20, 30));
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClassW(&wc);
    
    mainWindow = CreateWindowExW(
        WS_EX_LAYERED | WS_EX_TOPMOST,
        L"TimeSOTExternalClass",
        L"Time SOT External v1.1",
        WS_POPUP | WS_VISIBLE,
        100, 100,
        APP_WIDTH, APP_HEIGHT,
        NULL, NULL, hInstance, NULL
    );
    
    SetLayeredWindowAttributes(mainWindow, 0, 245, LWA_ALPHA);
    
    MARGINS margins = {1, 1, 1, 1};
    DwmExtendFrameIntoClientArea(mainWindow, &margins);
    
    minimizeButton = CreateWindowW(L"BUTTON", L"─",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        APP_WIDTH - 70, 10, BUTTON_WIDTH, BUTTON_HEIGHT, 
        mainWindow, (HMENU)1002, NULL, NULL);
    
    closeButton = CreateWindowW(L"BUTTON", L"×",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        APP_WIDTH - 35, 10, BUTTON_WIDTH, BUTTON_HEIGHT, 
        mainWindow, (HMENU)1003, NULL, NULL);
    
    HFONT hFont = CreateFontW(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, FF_DONTCARE, L"Segoe UI");
    
    SendMessage(minimizeButton, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(closeButton, WM_SETFONT, (WPARAM)hFont, TRUE);
    
    keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, GetModuleHandle(NULL), 0);
    if (!keyboardHook) {
        MessageBoxW(NULL, L"Failed keybord hook", L"Error", MB_ICONERROR);
    }
    
    HANDLE packetThread = (HANDLE)_beginthreadex(NULL, 0, packet_thread, NULL, 0, NULL);
    
    MSG msg;
    while(GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    WaitForSingleObject(packetThread, 1000);
    CloseHandle(packetThread);
    
    return 0;
}
