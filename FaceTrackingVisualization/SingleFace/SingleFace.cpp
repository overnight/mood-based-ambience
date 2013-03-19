//------------------------------------------------------------------------------
// <copyright file="SingleFace.cpp" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

// Defines the entry point for the application.
//

#include "stdafx.h"
#include "SingleFace.h"
#include "EggAvatar.h"
#include <FaceTrackLib.h>
#include "FTHelper.h"
#include <winsock2.h>
#include <iostream>
#pragma comment(lib,"ws2_32.lib")
#include <atlbase.h>



class IFTResultDataPacket
{
public:
	FLOAT m_pAU[6];
	UINT m_numAU;
	FLOAT m_scale;
	FLOAT m_rotationXYZ[3];
	FLOAT m_translationXYZ[3];
	bool isFailed;
	IFTResultDataPacket(): m_numAU(0) {}
	void populate(IFTResult *pResult)
	{
		if(FAILED(pResult->GetStatus()))
			isFailed = true;
		else
			isFailed = false;
		FLOAT* pAU = NULL;
		UINT numAU;
		pResult->GetAUCoefficients(&pAU, &numAU);
		if(numAU != 6)
			std::cerr << "Error: num of AUs is not 6.\n";
		for (int i=0; i<numAU; i++)
		{
			m_pAU[i] = pAU[i];
		}
		m_numAU = numAU;
		pResult->Get3DPose(&m_scale, m_rotationXYZ, m_translationXYZ);
	}
};
typedef IFTResultDataPacket SocketDataType;
class SingleFace
{
public:
    SingleFace() 
    : m_hInst(NULL)
    , m_hWnd(NULL)
    , m_hAccelTable(NULL)
    , m_pImageBuffer(NULL)
    , m_pVideoBuffer(NULL)
    , m_depthType(NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX)
    , m_colorType(NUI_IMAGE_TYPE_COLOR)
    , m_depthRes(NUI_IMAGE_RESOLUTION_320x240)
    , m_colorRes(NUI_IMAGE_RESOLUTION_640x480)
    , m_bNearMode(TRUE)
	, m_bSeatedSkeletonMode(FALSE)
    {}

    int Run(HINSTANCE hInst, PWSTR lpCmdLine, int nCmdShow);

protected:
    BOOL                        InitInstance(HINSTANCE hInst, PWSTR lpCmdLine, int nCmdShow);
    void                        ParseCmdString(PWSTR lpCmdLine);
    void                        UninitInstance();
    ATOM                        RegisterClass(PCWSTR szWindowClass);
    static LRESULT CALLBACK     WndProcStatic(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    LRESULT CALLBACK            WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    static INT_PTR CALLBACK     About(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    BOOL                        PaintWindow(HDC hdc, HWND hWnd);
    BOOL                        ShowVideo(HDC hdc, int width, int height, int originX, int originY);
    BOOL                        ShowEggAvatar(HDC hdc, int width, int height, int originX, int originY);
    static void                 FTHelperCallingBack(LPVOID lpParam);
    static int const            MaxLoadStringChars = 100;
	bool						initSocket();
	bool						start_listening();
	bool						sendSocketData(IFTResult *pResult);
    HINSTANCE                   m_hInst;
    HWND                        m_hWnd;
    HACCEL                      m_hAccelTable;
    EggAvatar                   m_eggavatar;
    FTHelper                    m_FTHelper;
    IFTImage*                   m_pImageBuffer;
    IFTImage*                   m_pVideoBuffer;

    NUI_IMAGE_TYPE              m_depthType;
    NUI_IMAGE_TYPE              m_colorType;
    NUI_IMAGE_RESOLUTION        m_depthRes;
    NUI_IMAGE_RESOLUTION        m_colorRes;
    BOOL                        m_bNearMode;
	BOOL                        m_bSeatedSkeletonMode;

	WSADATA WsaDat;
	SOCKET Socket;
	SOCKET newSocket;
	SOCKADDR_IN serverInf;
	char*						m_pBuffer;
	bool						m_isSocketRunning;
	//SocketDataType				m_data;
};

bool SingleFace::initSocket()
{
	if(m_isSocketRunning)
		return false;
	m_pBuffer = new char(sizeof(SocketDataType));
	if(WSAStartup(MAKEWORD(2,2),&WsaDat)!=0)
	{
		std::cout<<"WSA Initialization failed!\r\n";
		MessageBox(NULL, L"WSA initialization failed", L"socket error", 0);
		WSACleanup();
		system("PAUSE");
		return 0;
	}
	Socket=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if(Socket==INVALID_SOCKET)
	{
		std::cout<<"Socket creation failed.\r\n";
		MessageBox(NULL, L"Socket creation failed!", L"socket error", 0);
		WSACleanup();
		system("PAUSE");
		return 0;
	}
	serverInf.sin_family=AF_INET;
	serverInf.sin_addr.s_addr=INADDR_ANY;
	serverInf.sin_port=htons(8888);
	if(bind(Socket,(SOCKADDR*)(&serverInf),sizeof(serverInf))==SOCKET_ERROR)
	{
		std::cout<<"Unable to bind socket!\r\n";
		MessageBox(NULL, L"Unable to bind to socket", L"socket error", 0);
		WSACleanup();
		system("PAUSE");
		return 0;
	}
	return true;
}
bool SingleFace::start_listening()
{
	/*if(m_isSocketRunning)
		return false;*/
	int nError;
	if(listen(Socket,5) == SOCKET_ERROR)
	{
		nError = WSAGetLastError();
		char buf[100];
		USES_CONVERSION;
		sprintf(buf, "Listen error: %d", nError);
		LPCWSTR w = A2W(buf);
		MessageBox(NULL,w, L"socket error",0);
	}
	SOCKET TempSock=SOCKET_ERROR;
	
	while(TempSock==SOCKET_ERROR)
	{
		std::cout<<"Waiting for incoming connections...\r\n";
		//MessageBox(NULL, L"Waiting for incoming connections...", L"Socket", MB_OK);
		TempSock=accept(Socket,NULL,NULL);
		if(TempSock == SOCKET_ERROR)
		{
			nError = WSAGetLastError();
			char buf[100];
			USES_CONVERSION;
			sprintf(buf, "Accept error: %d", nError);
			LPCWSTR w = A2W(buf);
			MessageBox(NULL,w, L"socket error",0);
		}
	}
	// If iMode!=0, non-blocking mode is enabled.
	u_long iMode=0;
	ioctlsocket(Socket,FIONBIO,&iMode);
	newSocket=TempSock;
	std::cout<<"Client connected!\r\n\r\n";
	//MessageBox(NULL, L"Client Connected!", L"Socket", MB_OK);
	m_isSocketRunning = true;
	return true;
}
// Run the SingleFace application.
int SingleFace::Run(HINSTANCE hInst, PWSTR lpCmdLine, int nCmdShow)
{
    MSG msg = {static_cast<HWND>(0), static_cast<UINT>(0), static_cast<WPARAM>(-1)};
	m_isSocketRunning = false;
	initSocket();
	start_listening();
    if (InitInstance(hInst, lpCmdLine, nCmdShow))
    {
        // Main message loop:
        while (GetMessage(&msg, NULL, 0, 0))
        {
            if (!TranslateAccelerator(msg.hwnd, m_hAccelTable, &msg))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
	}
	WSACleanup();
    UninitInstance();

    return (int)msg.wParam;
}

// In this function, we save the instance handle, then create and display the main program window.
BOOL SingleFace::InitInstance(HINSTANCE hInstance, PWSTR lpCmdLine, int nCmdShow)
{
   m_hInst = hInstance; // Store instance handle in our global variable

   ParseCmdString(lpCmdLine);

   WCHAR szTitle[MaxLoadStringChars];                  // The title bar text
   LoadString(m_hInst, IDS_APP_TITLE, szTitle, ARRAYSIZE(szTitle));

   static const PCWSTR RES_MAP[] = { L"80x60", L"320x240", L"640x480", L"1280x960" };
   static const PCWSTR IMG_MAP[] = { L"PLAYERID", L"RGB", L"YUV", L"YUV_RAW", L"DEPTH" };
 
   // Add mode params in title
   WCHAR szTitleComplete[MAX_PATH];
   swprintf_s(szTitleComplete, L"%s -- Depth:%s:%s Color:%s:%s NearMode:%s, SeatedSkeleton:%s", szTitle,
       IMG_MAP[m_depthType], (m_depthRes < 0)? L"ERROR": RES_MAP[m_depthRes], IMG_MAP[m_colorType], (m_colorRes < 0)? L"ERROR": RES_MAP[m_colorRes], m_bNearMode? L"ON": L"OFF",
	   m_bSeatedSkeletonMode?L"ON": L"OFF");

   WCHAR szWindowClass[MaxLoadStringChars];            // the main window class name
   LoadString(m_hInst, IDC_SINGLEFACE, szWindowClass, ARRAYSIZE(szWindowClass));

   RegisterClass(szWindowClass);

   m_hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SINGLEFACE));

   m_pImageBuffer = FTCreateImage();
   m_pVideoBuffer = FTCreateImage();

   m_hWnd = CreateWindow(szWindowClass, szTitleComplete, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, m_hInst, this);
   if (!m_hWnd)
   {
       return FALSE;
   }
   
   ShowWindow(m_hWnd, nCmdShow);
   UpdateWindow(m_hWnd);
   
   return SUCCEEDED(m_FTHelper.Init(m_hWnd,
            FTHelperCallingBack,
            this,
            m_depthType,
            m_depthRes,
            m_bNearMode,
            TRUE, // if near mode doesn't work, fall back to default mode
            m_colorType,
            m_colorRes,
            m_bSeatedSkeletonMode));
}

void SingleFace::UninitInstance()
{
    // Clean up the memory allocated for Face Tracking and rendering.
    m_FTHelper.Stop();

    if (m_hAccelTable)
    {
        DestroyAcceleratorTable(m_hAccelTable);
        m_hAccelTable = NULL;
    }

    DestroyWindow(m_hWnd);
    m_hWnd = NULL;

    if (m_pImageBuffer)
    {
        m_pImageBuffer->Release();
        m_pImageBuffer = NULL;
    }

    if (m_pVideoBuffer)
    {
        m_pVideoBuffer->Release();
        m_pVideoBuffer = NULL;
    }
}


// Register the window class.
ATOM SingleFace::RegisterClass(PCWSTR szWindowClass)
{
    WNDCLASSEX wcex = {0};

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = &SingleFace::WndProcStatic;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = m_hInst;
    wcex.hIcon          = LoadIcon(m_hInst, MAKEINTRESOURCE(IDI_SINGLEFACE));
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCE(IDC_SINGLEFACE);
    wcex.lpszClassName  = szWindowClass;
    
    return RegisterClassEx(&wcex);
}

LRESULT CALLBACK SingleFace::WndProcStatic(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static SingleFace* pThis = NULL; // cheating, but since there is just one window now, it will suffice.
    if (WM_CREATE == message)
    {
        pThis = reinterpret_cast<SingleFace*>(reinterpret_cast<CREATESTRUCT*>(lParam)->lpCreateParams);
    }
    return pThis ? pThis->WndProc(hWnd, message, wParam, lParam) : DefWindowProc(hWnd, message, wParam, lParam);
}

//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_KEYUP    - Exit in response to ESC key
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
LRESULT CALLBACK SingleFace::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    UINT wmId, wmEvent;
    PAINTSTRUCT ps;
    HDC hdc;

    switch (message)
    {
    case WM_COMMAND:
        wmId    = LOWORD(wParam);
        wmEvent = HIWORD(wParam);
        // Parse the menu selections:
        switch (wmId)
        {
        case IDM_ABOUT:
            DialogBox(m_hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case IDM_EXIT:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;
    case WM_KEYUP:
        if (wParam == VK_ESCAPE)
        {
            PostQuitMessage(0);
        }
        break;
    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);
        // Draw the avatar window and the video window
        PaintWindow(hdc, hWnd);
        EndPaint(hWnd, &ps);
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
INT_PTR CALLBACK SingleFace::About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
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

// Drawing the video window
BOOL SingleFace::ShowVideo(HDC hdc, int width, int height, int originX, int originY)
{
    BOOL ret = TRUE;

    // Now, copy a fraction of the camera image into the screen.
    IFTImage* colorImage = m_FTHelper.GetColorImage();
    if (colorImage)
    {
        int iWidth = colorImage->GetWidth();
        int iHeight = colorImage->GetHeight();
        if (iWidth > 0 && iHeight > 0)
        {
            int iTop = 0;
            int iBottom = iHeight;
            int iLeft = 0;
            int iRight = iWidth;

            // Keep a separate buffer.
            if (m_pVideoBuffer && SUCCEEDED(m_pVideoBuffer->Allocate(iWidth, iHeight, FTIMAGEFORMAT_UINT8_B8G8R8A8)))
            {
                // Copy do the video buffer while converting bytes
                colorImage->CopyTo(m_pVideoBuffer, NULL, 0, 0);

                // Compute the best approximate copy ratio.
                float w1 = (float)iHeight * (float)width;
                float w2 = (float)iWidth * (float)height;
                if (w2 > w1 && height > 0)
                {
                    // video image too wide
                    float wx = w1/height;
                    iLeft = (int)max(0, m_FTHelper.GetXCenterFace() - wx / 2);
                    iRight = iLeft + (int)wx;
                    if (iRight > iWidth)
                    {
                        iRight = iWidth;
                        iLeft = iRight - (int)wx;
                    }
                }
                else if (w1 > w2 && width > 0)
                {
                    // video image too narrow
                    float hy = w2/width;
                    iTop = (int)max(0, m_FTHelper.GetYCenterFace() - hy / 2);
                    iBottom = iTop + (int)hy;
                    if (iBottom > iHeight)
                    {
                        iBottom = iHeight;
                        iTop = iBottom - (int)hy;
                    }
                }

                int const bmpPixSize = m_pVideoBuffer->GetBytesPerPixel();
                SetStretchBltMode(hdc, HALFTONE);
                BITMAPINFO bmi = {sizeof(BITMAPINFO), iWidth, iHeight, 1, static_cast<WORD>(bmpPixSize * CHAR_BIT), BI_RGB, m_pVideoBuffer->GetStride() * iHeight, 5000, 5000, 0, 0};
                if (0 == StretchDIBits(hdc, originX, originY, width, height,
                    iLeft, iBottom, iRight-iLeft, iTop-iBottom, m_pVideoBuffer->GetBuffer(), &bmi, DIB_RGB_COLORS, SRCCOPY))
                {
                    ret = FALSE;
                }
            }
        }
    }
    return ret;
}

// Drawing code
BOOL SingleFace::ShowEggAvatar(HDC hdc, int width, int height, int originX, int originY)
{
    static int errCount = 0;
    BOOL ret = FALSE;

    if (m_pImageBuffer && SUCCEEDED(m_pImageBuffer->Allocate(width, height, FTIMAGEFORMAT_UINT8_B8G8R8A8)))
    {
        memset(m_pImageBuffer->GetBuffer(), 0, m_pImageBuffer->GetStride() * height); // clear to black

        m_eggavatar.SetScaleAndTranslationToWindow(height, width);
        m_eggavatar.DrawImage(m_pImageBuffer);

        BITMAPINFO bmi = {sizeof(BITMAPINFO), width, height, 1, static_cast<WORD>(m_pImageBuffer->GetBytesPerPixel() * CHAR_BIT), BI_RGB, m_pImageBuffer->GetStride() * height, 5000, 5000, 0, 0};
        errCount += (0 == StretchDIBits(hdc, 0, 0, width, height, 0, 0, width, height, m_pImageBuffer->GetBuffer(), &bmi, DIB_RGB_COLORS, SRCCOPY));

        ret = TRUE;
    }

    return ret;
}

// Draw the egg head and the camera video with the mask superimposed.
BOOL SingleFace::PaintWindow(HDC hdc, HWND hWnd)
{
    static int errCount = 0;
    BOOL ret = FALSE;
    RECT rect;
    GetClientRect(hWnd, &rect);
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;
    int halfWidth = width/2;

    // Show the video on the right of the window
    errCount += !ShowVideo(hdc, width - halfWidth, height, halfWidth, 0);

    // Draw the egg avatar on the left of the window
    errCount += !ShowEggAvatar(hdc, halfWidth, height, 0, 0);
    return ret;
}

bool SingleFace::sendSocketData(IFTResult *pResult)
{
	if(!m_isSocketRunning)
		return false;
	if(!pResult)
		return false;
	SocketDataType data;
	data.populate(pResult);
	memcpy(m_pBuffer, &data, sizeof(SocketDataType));
	int nSendBytes = send(newSocket,m_pBuffer,sizeof(SocketDataType),0);
	if(nSendBytes == SOCKET_ERROR)
	{
		m_isSocketRunning = false;
		//MessageBox(NULL, L"Socket disconnected!", L"Socket Error", 0);
		start_listening();
		return true;
	}
	int nError=WSAGetLastError();
	if(nError!=WSAEWOULDBLOCK&&nError!=0)
	{
		std::cout<<"Winsock error code: "<<nError<<"\r\n";
		std::cout<<"Client disconnected!\r\n";
		MessageBox(NULL, L"Winsock error code!", L"Socket Error", 0);
		// Shutdown our socket
		shutdown(newSocket,SD_SEND);

		// Close our socket entirely
		closesocket(newSocket);
		return false;
	}
	return true;
}
/*
 * The "Face Tracker" helper class is generic. It will call back this function
 * after a face has been successfully tracked. The code in the call back passes the parameters
 * to the Egg Avatar, so it can be animated.
 */
void SingleFace::FTHelperCallingBack(PVOID pVoid)
{
    SingleFace* pApp = reinterpret_cast<SingleFace*>(pVoid);
    if (pApp)
    {
        IFTResult* pResult = pApp->m_FTHelper.GetResult();
        if (pResult && SUCCEEDED(pResult->GetStatus()))
        {
			pApp->sendSocketData(pResult);
            FLOAT* pAU = NULL;
            UINT numAU;
            pResult->GetAUCoefficients(&pAU, &numAU);
            pApp->m_eggavatar.SetCandideAU(pAU, numAU);
            FLOAT scale;
            FLOAT rotationXYZ[3];
            FLOAT translationXYZ[3];
            pResult->Get3DPose(&scale, rotationXYZ, translationXYZ);
            pApp->m_eggavatar.SetTranslations(translationXYZ[0], translationXYZ[1], translationXYZ[2]);
            pApp->m_eggavatar.SetRotations(rotationXYZ[0], rotationXYZ[1], rotationXYZ[2]);
			//Sleep(10);
        }
		else if(pResult && FAILED(pResult->GetStatus()))
		{
			pApp->sendSocketData(pResult);
		}
    }
}

void SingleFace::ParseCmdString(PWSTR lpCmdLine)
{
    const WCHAR KEY_DEPTH[]                                 = L"-Depth";
    const WCHAR KEY_COLOR[]                                 = L"-Color";
    const WCHAR KEY_NEAR_MODE[]                             = L"-NearMode";
    const WCHAR KEY_DEFAULT_DISTANCE_MODE[]                 = L"-DefaultDistanceMode";
    const WCHAR KEY_SEATED_SKELETON_MODE[]                  = L"-SeatedSkeleton";

    const WCHAR STR_NUI_IMAGE_TYPE_DEPTH[]                  = L"DEPTH";
    const WCHAR STR_NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX[] = L"PLAYERID";
    const WCHAR STR_NUI_IMAGE_TYPE_COLOR[]                  = L"RGB";
    const WCHAR STR_NUI_IMAGE_TYPE_COLOR_YUV[]              = L"YUV";

    const WCHAR STR_NUI_IMAGE_RESOLUTION_80x60[]            = L"80x60";
    const WCHAR STR_NUI_IMAGE_RESOLUTION_320x240[]          = L"320x240";
    const WCHAR STR_NUI_IMAGE_RESOLUTION_640x480[]          = L"640x480";
    const WCHAR STR_NUI_IMAGE_RESOLUTION_1280x960[]         = L"1280x960";

    enum TOKEN_ENUM
    {
        TOKEN_ERROR,
        TOKEN_DEPTH,
        TOKEN_COLOR,
        TOKEN_NEARMODE,
        TOKEN_DEFAULTDISTANCEMODE,
		TOKEN_SEATEDSKELETON
    }; 

    int argc = 0;
    LPWSTR *argv = CommandLineToArgvW(lpCmdLine, &argc);

    for(int i = 0; i < argc; i++)
    {
        NUI_IMAGE_TYPE* pType = NULL;
        NUI_IMAGE_RESOLUTION* pRes = NULL;

        TOKEN_ENUM tokenType = TOKEN_ERROR; 
        PWCHAR context = NULL;
        PWCHAR token = wcstok_s(argv[i], L":", &context);
        if(0 == wcsncmp(token, KEY_DEPTH, ARRAYSIZE(KEY_DEPTH)))
        {
            tokenType = TOKEN_DEPTH;
            pType = &m_depthType;
            pRes = &m_depthRes;
        }
        else if(0 == wcsncmp(token, KEY_COLOR, ARRAYSIZE(KEY_COLOR)))
        {
            tokenType = TOKEN_COLOR;
            pType = &m_colorType;
            pRes = &m_colorRes;
        }
        else if(0 == wcsncmp(token, KEY_NEAR_MODE, ARRAYSIZE(KEY_NEAR_MODE)))
        {
            tokenType = TOKEN_NEARMODE;
            m_bNearMode = TRUE;
        }
        else if(0 == wcsncmp(token, KEY_DEFAULT_DISTANCE_MODE, ARRAYSIZE(KEY_DEFAULT_DISTANCE_MODE)))
        {
            tokenType = TOKEN_DEFAULTDISTANCEMODE;
            m_bNearMode = FALSE;
        }
        else if(0 == wcsncmp(token, KEY_SEATED_SKELETON_MODE, ARRAYSIZE(KEY_SEATED_SKELETON_MODE)))
        {
            tokenType = TOKEN_SEATEDSKELETON;
            m_bSeatedSkeletonMode = TRUE;
        }

        if(tokenType == TOKEN_DEPTH || tokenType == TOKEN_COLOR)
        {
            _ASSERT(pType != NULL && pRes != NULL);
            
            while((token = wcstok_s(NULL, L":", &context)) != NULL)
            {
                if(0 == wcsncmp(token, STR_NUI_IMAGE_TYPE_DEPTH, ARRAYSIZE(STR_NUI_IMAGE_TYPE_DEPTH)))
                {
                    *pType = NUI_IMAGE_TYPE_DEPTH;
                }
                else if(0 == wcsncmp(token, STR_NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX, ARRAYSIZE(STR_NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX)))
                {
                    *pType = NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX;
                }
                else if(0 == wcsncmp(token, STR_NUI_IMAGE_TYPE_COLOR, ARRAYSIZE(STR_NUI_IMAGE_TYPE_COLOR)))
                {
                    *pType = NUI_IMAGE_TYPE_COLOR;
                }
                else if(0 == wcsncmp(token, STR_NUI_IMAGE_TYPE_COLOR_YUV, ARRAYSIZE(STR_NUI_IMAGE_TYPE_COLOR_YUV)))
                {
                    *pType = NUI_IMAGE_TYPE_COLOR_YUV;
                }
                else if(0 == wcsncmp(token, STR_NUI_IMAGE_RESOLUTION_80x60, ARRAYSIZE(STR_NUI_IMAGE_RESOLUTION_80x60)))
                {
                    *pRes = NUI_IMAGE_RESOLUTION_80x60;
                }
                else if(0 == wcsncmp(token, STR_NUI_IMAGE_RESOLUTION_320x240, ARRAYSIZE(STR_NUI_IMAGE_RESOLUTION_320x240)))
                {
                    *pRes = NUI_IMAGE_RESOLUTION_320x240;
                }
                else if(0 == wcsncmp(token, STR_NUI_IMAGE_RESOLUTION_640x480, ARRAYSIZE(STR_NUI_IMAGE_RESOLUTION_640x480)))
                {
                    *pRes = NUI_IMAGE_RESOLUTION_640x480;
                }
                else if(0 == wcsncmp(token, STR_NUI_IMAGE_RESOLUTION_1280x960, ARRAYSIZE(STR_NUI_IMAGE_RESOLUTION_1280x960)))
                {
                    *pRes = NUI_IMAGE_RESOLUTION_1280x960;
                }
            }
        }
    }

    if(argv) LocalFree(argv);
}


// Program's main entry point
int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nCmdShow)
{
	USES_CONVERSION;
    UNREFERENCED_PARAMETER(hPrevInstance);
    SingleFace app;

    HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

    return app.Run(hInstance, lpCmdLine, nCmdShow);
}
