// FitApp.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "FitApp.h"
#include <string>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include "Core.h"


#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    SignUpDlgProc(HWND, UINT, WPARAM, LPARAM);
std::string GenerateSaltFunction();
std::string HashPasswordFunction(const WCHAR* password, const std::string& salt);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_FITAPP, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_FITAPP));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_FITAPP));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_FITAPP);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        {
            CreateWindowW(L"BUTTON", L"Sign In",
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                50, 100, 100, 30, hWnd, (HMENU)1, hInst, NULL);

            CreateWindowW(L"BUTTON", L"Sign Up",
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                200, 100, 100, 30, hWnd, (HMENU)2, hInst, NULL);
        }
        break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            switch (wmId)
            {
            case 1: // Sign In button
                // Handle sign in logic here
                break;
            case 2: // Sign Up button
                // 1. Show dialog to get username/password
                DialogBox(hInst, MAKEINTRESOURCE(IDD_SIGNUP), hWnd, SignUpDlgProc);
                // 2. Create user object -> next window will be user stats
                // 3. Insert user into SQLite database
                break;
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
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

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

// Function to generate a salt
std::string GenerateSaltFunction()
{
    unsigned char salt[16];
    if (RAND_bytes(salt, sizeof(salt)) != 1) {
        // Handle error
        return "";
    }
    // Convert to hex string
    std::string saltStr;
    char buf[3];
    for (int i = 0; i < sizeof(salt); ++i) {
        sprintf(buf, "%02x", salt[i]);
        saltStr += buf;
    }
    return saltStr;
}

// Function to hash a password with a salt
std::string HashPasswordFunction(const WCHAR* password, const std::string& salt)
{
    // Convert WCHAR* password to UTF-8 std::string
    int len = WideCharToMultiByte(CP_UTF8, 0, password, -1, nullptr, 0, nullptr, nullptr);
    std::string pwdUtf8(len, 0);
    WideCharToMultiByte(CP_UTF8, 0, password, -1, &pwdUtf8[0], len, nullptr, nullptr);

    std::string saltedPwd = pwdUtf8 + salt;

    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hashLen;

    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr);
    EVP_DigestUpdate(ctx, saltedPwd.c_str(), saltedPwd.size());
    EVP_DigestFinal_ex(ctx, hash, &hashLen);
    EVP_MD_CTX_free(ctx);

    // Convert hash to hex string
    std::string hashStr;
    char buf[3];
    for (unsigned int i = 0; i < hashLen; ++i) {
        sprintf(buf, "%02x", hash[i]);
        hashStr += buf;
    }
    return hashStr;
}

// Dialog procedure for signup
INT_PTR CALLBACK SignUpDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK)
        {
            WCHAR username[100], password[100];
            GetDlgItemText(hDlg, IDC_USERNAME_EDIT, username, 100);
            GetDlgItemText(hDlg, IDC_PASSWORD_EDIT, password, 100);

            // Convert username to std::wstring
            std::wstring userNameW(username);

            // Generate salt and hash password
            std::string salt = GenerateSaltFunction();
            std::string pwdHash = HashPasswordFunction(password, salt);

            // Create User object
            Core::User newUser(userNameW, pwdHash, salt);

            // Insert into SQLite (implement this function in Core)
            InsertUserToDatabase(newUser);

            EndDialog(hDlg, IDOK);
            return (INT_PTR)TRUE;
        }
        else if (LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, IDCANCEL);
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}



