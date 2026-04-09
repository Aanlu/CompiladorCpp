#include "framework.h"
#include "Compilador2.h"
#include "Lexer.h"
#include "Tokens.h"
#include <string>
#include <vector>
#include <sstream>
#include <stdexcept>

#define MAX_LOADSTRING 100

#define IDC_TEXT_INPUT   101
#define IDC_TEXT_OUTPUT  102
#define IDC_BTN_ANALIZAR 103

HINSTANCE hInst;
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];

HWND hwndInput;
HWND hwndOutput;
HWND hwndBoton;

// Convierte wstring (Unicode) a string UTF-8 para el lexer
std::string ConvertirHaciaUTF8(const std::wstring& wstr) {
    if (wstr.empty()) return std::string();
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}

// Convierte string UTF-8 a wstring para mostrar en controles Win32
std::wstring ConvertirHaciaWide(const std::string& str) {
    if (str.empty()) return std::wstring();
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}

// Función auxiliar para hacer visibles los saltos de línea y tabulaciones en el log
std::string LimpiarLexemaParaDisplay(const std::string& lexema) {
    std::string resultado = "";
    for (char c : lexema) {
        if (c == '\n') {
            resultado += "\\n"; // Muestra explícitamente el salto de línea
        }
        else if (c == '\r') {
            resultado += "\\r"; // Muestra el retorno de carro (típico en Windows)
        }
        else if (c == '\t') {
            resultado += "\\t"; // Muestra las tabulaciones
        }
        else {
            resultado += c;     // Deja los caracteres normales intactos
        }
    }
    return resultado;
}

ATOM             MyRegisterClass(HINSTANCE hInstance);
BOOL             InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
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
            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
            FIXED_PITCH | FF_MODERN, L"Consolas");

        hwndInput = CreateWindowEx(0, L"EDIT", L"",
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_BORDER |
            ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN,
            20, 20, 740, 200, hWnd, (HMENU)IDC_TEXT_INPUT, hInst, NULL);
        SendMessage(hwndInput, WM_SETFONT, (WPARAM)hFont, TRUE);

        hwndBoton = CreateWindowEx(0, L"BUTTON", L"Ejecutar Analizador",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            300, 230, 200, 40, hWnd, (HMENU)IDC_BTN_ANALIZAR, hInst, NULL);
        SendMessage(hwndBoton, WM_SETFONT, (WPARAM)hFont, TRUE);

        hwndOutput = CreateWindowEx(0, L"EDIT", L"",
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_BORDER |
            ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
            20, 280, 740, 260, hWnd, (HMENU)IDC_TEXT_OUTPUT, hInst, NULL);
        SendMessage(hwndOutput, WM_SETFONT, (WPARAM)hFont, TRUE);
    }
    break;

    case WM_CTLCOLORSTATIC:
    {
        HDC hdcStatic = (HDC)wParam;
        HWND hwndStatic = (HWND)lParam;

        // Validamos que le vamos a cambiar el color SOLO a la caja de salida
        if (hwndStatic == hwndOutput) {
            // Color del texto (RGB: Rojo, Verde, Azul) - Aqui te puse un Azul Rey
            SetTextColor(hdcStatic, RGB(0, 50, 200));

            // Fondo del texto (debe coincidir con el fondo de la ventana)
            SetBkColor(hdcStatic, GetSysColor(COLOR_WINDOW));

            // Retornamos la brocha del fondo
            return (INT_PTR)GetSysColorBrush(COLOR_WINDOW);
        }
    }
    break;

    case WM_COMMAND:
    {
        if (LOWORD(wParam) == IDC_BTN_ANALIZAR)
        {
            int len = GetWindowTextLength(hwndInput);
            if (len > 0)
            {
                std::vector<wchar_t> buffer(len + 1);
                GetWindowText(hwndInput, &buffer[0], len + 1);
                std::string codigoFuente = ConvertirHaciaUTF8(std::wstring(&buffer[0]));

                try {
                    Lexer miAnalizador(
                        codigoFuente,
                        "matriz_transiciones.csv",
                        "nombres_tokens.csv",
                        "estados_tokens.csv",
                        "char_columnas.csv",
                        "keywords.csv"
                    );

                    // Cambiamos a std::vector
                    std::vector<Token> listaTokens = miAnalizador.generarListaTokens();

                    std::stringstream salida;
                    for (const auto& token : listaTokens) {
                        const std::string& nombre = NOMBRES_TOKENS[token.tipo];
                        salida << "Línea: " << token.linea << " \t| Col: " << token.columna << " \t| ";
                        salida << "Tipo: [" << nombre << "]";
                        salida << (nombre.length() < 7 ? "\t\t" : "\t");
                        salida << "| Lexema: [" << LimpiarLexemaParaDisplay(token.lexema) << "]\r\n";
                    }

                    SetWindowText(hwndOutput, ConvertirHaciaWide(salida.str()).c_str());
                }
                catch (const std::exception& e) {
                    SetWindowText(hwndOutput, ConvertirHaciaWide(e.what()).c_str());
                }
            }
            else {
                SetWindowText(hwndOutput, L"Error: El campo de entrada se encuentra vacio.");
            }
        }
    }
    break;

    case WM_SIZE:
    {
        int w = LOWORD(lParam), h = HIWORD(lParam);
        MoveWindow(hwndInput, 20, 20, w - 40, (h / 2) - 60, TRUE);
        MoveWindow(hwndBoton, (w / 2) - 100, (h / 2) - 30, 200, 40, TRUE);
        MoveWindow(hwndOutput, 20, (h / 2) + 20, w - 40, (h / 2) - 40, TRUE);
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