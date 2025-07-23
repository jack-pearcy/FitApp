// FitApp.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "FitApp.h"
#include <string>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include "Core.h"
#include <sqlite3.h> // Ensure SQLite header is included
#include <codecvt>
#include <locale>
#include <Windows.h>
#include <commctrl.h> // Needed for slider/trackbar
#include <cmath> // For round function
#include <cstdio> // For printf

#define MAX_LOADSTRING 100

// Use inches for everything in the UI
#define MIN_HEIGHT_INCHES 39   // 3 ft 3 in
#define MAX_HEIGHT_INCHES 84   // 7 ft 0 in

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
std::string g_userNameUtf8;                     // Global variable for username in UTF-8

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    SignInDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK    SignUpDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK UserStatsDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
std::string GenerateSaltFunction();
std::string HashPasswordFunction(const WCHAR* password, const std::string& salt);
void DisplayHeightFeetInches(HWND hDlg, int inches);
void SaveUserStatsToDB(const std::string& username, int height, int weight);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    fnCore();

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

    // Define desired window size
    int windowWidth = 400;  // Width of the window
    int windowHeight = 300; // Height of the window

    // Get screen dimensions
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    // Calculate top-left corner to center the window
    int xPos = (screenWidth - windowWidth) / 2;
    int yPos = (screenHeight - windowHeight) / 2;

    // Create the window at the calculated position
    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
                              xPos, yPos, windowWidth, windowHeight, nullptr, nullptr, hInstance, nullptr);

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
                // Show sign-in dialog
                DialogBox(hInst, MAKEINTRESOURCE(IDD_SIGNIN), hWnd, SignInDlgProc);
                break;
            case 2: // Sign Up button
            {
                // Show sign-up dialog
                INT_PTR result = DialogBox(hInst, MAKEINTRESOURCE(IDD_SIGNUP), hWnd, SignUpDlgProc);
                if (result == IDOK) {
                    // If sign-up was successful, show user stats dialog
                    DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_USERSTATS), hWnd, UserStatsDlgProc, (LPARAM)g_userNameUtf8.c_str());
                }
                // User cancelled or closed dialog: do nothing
                break;
            }
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
        sprintf_s(buf, "%02x", salt[i]);
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
        sprintf_s(buf, "%02x", hash[i]);
        hashStr += buf;
    }
    return hashStr;
}

// Helper function to convert inches to feet/inches
void DisplayHeightFeetInches(HWND hDlg, int inches)
{
    int feet = inches / 12;
    int remInches = inches % 12;
    WCHAR buf[32];
    swprintf(buf, 32, L"%d' %d\''", feet, remInches);
    SetDlgItemText(hDlg, IDC_HEIGHT_DISPLAY, buf);
}

// Function to save user stats to the database
void SaveUserStatsToDB(const std::string& username, int height, int weight)
{
    sqlite3* db;
    sqlite3_stmt* stmt;
    int rc = sqlite3_open("FitApp.db", &db);
    
    if (rc != SQLITE_OK) {
        OutputDebugStringA("Failed to open database\n");
        return;
    }
    
    // Insert into userstats table with proper error handling
    const char* sqlInsert = "INSERT INTO Userstats (Username, Height, Weight) VALUES (?, ?, ?);";
    rc = sqlite3_prepare_v2(db, sqlInsert, -1, &stmt, nullptr);
    
    if (rc != SQLITE_OK) {
        char errMsg[256];
        sprintf_s(errMsg, "SQL prepare error: %s\n", sqlite3_errmsg(db));
        OutputDebugStringA(errMsg);
        sqlite3_close(db);
        return;
    }
    
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, height);
    sqlite3_bind_int(stmt, 3, weight);
    
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        char errMsg[256];
        sprintf_s(errMsg, "SQL insert error: %s\n", sqlite3_errmsg(db));
        OutputDebugStringA(errMsg);
    } else {
        OutputDebugStringA("Successfully inserted into Userstats table\n");
    }
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

// Dialog procedure for sign in
INT_PTR CALLBACK SignInDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_INITDIALOG:
    {
        // Debug: Dialog initialization
        OutputDebugStringA("SignInDlgProc: WM_INITDIALOG triggered\n");

        // Set focus to the username field so the user can start typing immediately
        SetFocus(GetDlgItem(hDlg, IDC_SIGNINUNAME));
        return (INT_PTR)FALSE; // Return TRUE to indicate successful initialization
    }

    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam); // Extract the control ID from wParam
        switch (wmId)
        {
        case IDC_SIGNINSUBMIT: // Handle "OK" button click
        {
            // Debug: OK button clicked
            OutputDebugStringA("SignInDlgProc: IDOK button clicked\n");

            WCHAR username[100], password[100]; // Declare buffers for username and password
            GetDlgItemText(hDlg, IDC_SIGNINUNAME, username, 100); // Retrieve text from the username field
            GetDlgItemText(hDlg, IDC_SIGNINPASS, password, 100); // Retrieve text from the password field

            // Debug: Retrieved username and password
            OutputDebugStringA(("SignInDlgProc: Username: " + std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(username) + "\n").c_str());
            OutputDebugStringA("SignInDlgProc: Password retrieved\n");

            // Validate input: Check if username or password fields are empty
            if (wcslen(username) == 0 || wcslen(password) == 0)
            {
                // Debug: Empty fields detected
                OutputDebugStringA("SignInDlgProc: Empty username or password detected\n");

                // Show an error message if fields are empty
                MessageBox(hDlg, L"Please enter both username and password.", L"Error", MB_OK | MB_ICONERROR);
                return (INT_PTR)TRUE; // Return TRUE to indicate the message was handled
            }

            // Convert username from wide string to UTF-8 for database query
            std::wstring_convert<std::codecvt_utf8<wchar_t>> conv; // Create a converter for wide strings
            std::string usernameUtf8 = conv.to_bytes(username); // Convert the wide string to UTF-8

            // Debug: Converted username to UTF-8
            OutputDebugStringA(("SignInDlgProc: Username UTF-8: " + usernameUtf8 + "\n").c_str());

            // Open the SQLite database
            sqlite3* db; // Declare a pointer for the database connection
            sqlite3_stmt* stmt; // Declare a pointer for the prepared statement
            int rc = sqlite3_open("FitApp.db", &db); // Open the database file
            if (rc != SQLITE_OK) // Check if the database connection failed
            {
                // Debug: Database connection failed
                OutputDebugStringA("SignInDlgProc: Failed to connect to the database\n");

                // Show an error message if the database connection failed
                MessageBox(hDlg, L"Failed to connect to the database.", L"Error", MB_OK | MB_ICONERROR);
                EndDialog(hDlg, IDCANCEL); // Close the dialog
                return (INT_PTR)TRUE; // Return TRUE to indicate the message was handled
            }

            // Debug: Database connection successful
            OutputDebugStringA("SignInDlgProc: Database connection successful\n");

            // Prepare the SQL query to fetch the password hash and salt for the given username
            const char* sql = "SELECT Password, Salt FROM Users WHERE Username = ?;";
            rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr); // Prepare the SQL query
            if (rc != SQLITE_OK) // Check if the query preparation failed
            {
                sqlite3_close(db); // Close the database connection

                // Debug: Query preparation failed
                OutputDebugStringA("SignInDlgProc: Failed to prepare database query\n");

                // Show an error message if the query preparation failed
                MessageBox(hDlg, L"Failed to prepare database query.", L"Error", MB_OK | MB_ICONERROR);
                EndDialog(hDlg, IDCANCEL); // Close the dialog
                return (INT_PTR)TRUE; // Return TRUE to indicate the message was handled
            }

            // Debug: Query preparation successful
            OutputDebugStringA("SignInDlgProc: Query prepared successfully\n");

            // Bind the username parameter to the SQL query
            sqlite3_bind_text(stmt, 1, usernameUtf8.c_str(), -1, SQLITE_TRANSIENT);

            std::string storedHash, storedSalt; // Declare variables to store the retrieved hash and salt
            if (sqlite3_step(stmt) == SQLITE_ROW) // Execute the query and check if a row was returned
            {
                // Retrieve the password hash and salt from the query result
                storedHash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
                storedSalt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));

                // Debug: Retrieved hash and salt
                OutputDebugStringA(("SignInDlgProc: Retrieved Hash: " + storedHash + "\n").c_str());
                OutputDebugStringA(("SignInDlgProc: Retrieved Salt: " + storedSalt + "\n").c_str());
            }
            else // If no row was returned, the username does not exist
            {
                sqlite3_finalize(stmt); // Finalize the prepared statement
                sqlite3_close(db); // Close the database connection

                // Debug: Username not found
                OutputDebugStringA("SignInDlgProc: Username not found in database\n");

                // Show an error message if the username does not exist
                MessageBox(hDlg, L"Invalid username or password.", L"Error", MB_OK | MB_ICONERROR);
                return (INT_PTR)TRUE; // Return TRUE to indicate the message was handled
            }

            sqlite3_finalize(stmt); // Finalize the prepared statement
            sqlite3_close(db); // Close the database connection

            // Hash the entered password with the retrieved salt
            std::string enteredHash = HashPasswordFunction(password, storedSalt);

            // Debug: Computed entered hash
            OutputDebugStringA(("SignInDlgProc: Computed Entered Hash: " + enteredHash + "\n").c_str());

            // Compare the hashed password with the stored hash
            if (enteredHash == storedHash) // If the hashes match, the credentials are valid
            {
                // Debug: Password verified successfully
                OutputDebugStringA("SignInDlgProc: Password verified successfully\n");

                // Show a success message
                MessageBox(hDlg, L"Sign-in successful!", L"Success", MB_OK | MB_ICONINFORMATION);
                EndDialog(hDlg, IDOK); // Close the dialog and return IDOK
            }
            else // If the hashes do not match, the credentials are invalid
            {
                // Debug: Password verification failed
                OutputDebugStringA("SignInDlgProc: Password verification failed\n");

                // Show an error message
                MessageBox(hDlg, L"Invalid username or password.", L"Error", MB_OK | MB_ICONERROR);
            }

            return (INT_PTR)TRUE; // Return TRUE to indicate the message was handled
        }

        case IDCANCEL: // Handle "Cancel" button click
        {
            // Debug: Cancel button clicked
            OutputDebugStringA("SignInDlgProc: IDCANCEL button clicked\n");

            EndDialog(hDlg, IDCANCEL); // Close the dialog and return IDCANCEL
            return (INT_PTR)TRUE; // Return TRUE to indicate the message was handled
        }
        }
    }
    break;
    }
    return (INT_PTR)FALSE; // Return FALSE for unhandled messages
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

            // Convert username to UTF-8 std::string
            std::wstring userNameW(username);
            std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
            g_userNameUtf8 = conv.to_bytes(userNameW);

            // Generate salt and hash password
            std::string salt = GenerateSaltFunction();
            std::string pwdHash = HashPasswordFunction(password, salt);

            // Create User object
            Core::User newUser(userNameW, pwdHash, salt);

            // Insert into SQLite database
            sqlite3* db;
            sqlite3_stmt* stmt; // Declare stmt
            int rc = sqlite3_open("FitApp.db", &db);
            if (rc == SQLITE_OK)
            {
                const char* sql = "INSERT INTO Users (Username, Password, Salt) VALUES (?, ?, ?);";
                rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
                if (rc == SQLITE_OK)
                {
                    sqlite3_bind_text(stmt, 1, g_userNameUtf8.c_str(), -1, SQLITE_TRANSIENT);
                    sqlite3_bind_text(stmt, 2, pwdHash.c_str(), -1, SQLITE_TRANSIENT);
                    sqlite3_bind_text(stmt, 3, salt.c_str(), -1, SQLITE_TRANSIENT);

                    sqlite3_step(stmt);
                    sqlite3_finalize(stmt);
                }
                sqlite3_close(db);
            }

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

// Dialog procedure for user stats
INT_PTR CALLBACK UserStatsDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_INITDIALOG:
        {
            HWND hSlider = GetDlgItem(hDlg, IDC_HEIGHT_SLIDER);
            SendMessage(hSlider, TBM_SETRANGE, TRUE, MAKELPARAM(MIN_HEIGHT_INCHES, MAX_HEIGHT_INCHES));
            SendMessage(hSlider, TBM_SETPOS, TRUE, 66); // Default to 5'6"
            DisplayHeightFeetInches(hDlg, 66); // Initial display
        }
        return (INT_PTR)TRUE;

    case WM_HSCROLL:
        {
            HWND hSlider = GetDlgItem(hDlg, IDC_HEIGHT_SLIDER);
            if ((HWND)lParam == hSlider)
            {
                int inches = (int)SendMessage(hSlider, TBM_GETPOS, 0, 0);
                DisplayHeightFeetInches(hDlg, inches);
            }
        }
        break;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK)
        {
            // Get the height value from the slider
            HWND hSlider = GetDlgItem(hDlg, IDC_HEIGHT_SLIDER);
            int heightInches = (int)SendMessage(hSlider, TBM_GETPOS, 0, 0);

            // Get the weight value from the edit box
            int weight = GetDlgItemInt(hDlg, IDC_WEIGHTINPUT, NULL, FALSE);

            // Get the username from global variable
            std::string username = g_userNameUtf8;

            // Save stats to database
            SaveUserStatsToDB(username, heightInches, weight);

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





