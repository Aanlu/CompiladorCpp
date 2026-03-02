#include "framework.h"
#include "Compilador2.h"
#include "Lexer.h" // Importación crítica de nuestro módulo modularizado
#include "Tokens.h" // Importación para el acceso al arreglo NOMBRES_TOKENS
#include <string>
#include <vector>
#include <sstream>
#include <stdexcept>

#define MAX_LOADSTRING 100

// Identificadores de los controles de la interfaz
#define IDC_TEXT_INPUT 101
#define IDC_TEXT_OUTPUT 102
#define IDC_BTN_ANALIZAR 103

// Variables globales de la ventana principal
HINSTANCE hInst;
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];

HWND hwndInput;
HWND hwndOutput;
HWND hwndBoton;

// =================================================================================================
// FUNCIONES DE CONVERSIÓN DE CODIFICACIÓN (WSTRING <-> STRING)
// =================================================================================================

std::string ConvertirHaciaUTF8(const std::wstring& wstr) {
    if (wstr.empty()) return std::string();
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}

std::wstring ConvertirHaciaWide(const std::string& str) {
    if (str.empty()) return std::wstring();
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}

// =================================================================================================
// LÓGICA DE LA INTERFAZ GRÁFICA (Win32 API)
// =================================================================================================

ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_COMPILADOR2, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    if (!InitInstance(hInstance, nCmdShow)) return FALSE;

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_COMPILADOR2));
    MSG msg;

    while (GetMessage(&msg, nullptr, 0, 0)) {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    return (int)msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_COMPILADOR2));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance;
    HWND hWnd = CreateWindowW(szWindowClass, L"Analizador Lexico FIM", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, 800, 600, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd) return FALSE;

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
    {
        HFONT hFont = CreateFont(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET,
            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, L"Consolas");

        hwndInput = CreateWindowEx(0, L"EDIT", L"",
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN,
            20, 20, 740, 200, hWnd, (HMENU)IDC_TEXT_INPUT, hInst, NULL);
        SendMessage(hwndInput, WM_SETFONT, (WPARAM)hFont, TRUE);

        hwndBoton = CreateWindowEx(0, L"BUTTON", L"Ejecutar Analizador",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            300, 230, 200, 40, hWnd, (HMENU)IDC_BTN_ANALIZAR, hInst, NULL);
        SendMessage(hwndBoton, WM_SETFONT, (WPARAM)hFont, TRUE);

        hwndOutput = CreateWindowEx(0, L"EDIT", L"",
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
            20, 280, 740, 260, hWnd, (HMENU)IDC_TEXT_OUTPUT, hInst, NULL);
        SendMessage(hwndOutput, WM_SETFONT, (WPARAM)hFont, TRUE);
    }
    break;

    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);

        if (wmId == IDC_BTN_ANALIZAR)
        {
            int len = GetWindowTextLength(hwndInput);
            if (len > 0)
            {
                std::vector<wchar_t> buffer(len + 1);
                GetWindowText(hwndInput, &buffer[0], len + 1);
                std::wstring entradaWide(&buffer[0]);

                std::string codigoFuente = ConvertirHaciaUTF8(entradaWide);

                try {
                    // Instanciación dinámica del motor inyectando el código y la matriz externa
                    Lexer miAnalizador(codigoFuente, "matriz_transiciones.csv");
                    Token tokenActual;
                    std::stringstream salida;

                    do {
                        tokenActual = miAnalizador.siguienteToken();

                        salida << "Tipo: [" << NOMBRES_TOKENS[tokenActual.tipo] << "]";

                        // Lógica de tabulación para alinear columnas visualmente
                        if (NOMBRES_TOKENS[tokenActual.tipo].length() < 7) salida << "\t\t";
                        else salida << "\t";

                        salida << "| Lexema: '" << tokenActual.lexema << "'\r\n";
                    } while (tokenActual.tipo != FIN);

                    std::wstring salidaWide = ConvertirHaciaWide(salida.str());
                    SetWindowText(hwndOutput, salidaWide.c_str());

                }
                catch (const std::exception& e) {
                    // Intercepción de excepciones (ej. Archivo CSV no encontrado)
                    std::string error(e.what());
                    SetWindowText(hwndOutput, ConvertirHaciaWide(error).c_str());
                }
            }
            else {
                SetWindowText(hwndOutput, L"Error: El campo de entrada se encuentra vacío.");
            }
        }
    }
    break;

    case WM_SIZE:
    {
        int width = LOWORD(lParam);
        int height = HIWORD(lParam);
        MoveWindow(hwndInput, 20, 20, width - 40, (height / 2) - 60, TRUE);
        MoveWindow(hwndBoton, (width / 2) - 100, (height / 2) - 30, 200, 40, TRUE);
        MoveWindow(hwndOutput, 20, (height / 2) + 20, width - 40, (height / 2) - 40, TRUE);
    }
    break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}