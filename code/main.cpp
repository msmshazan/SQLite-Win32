/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Mohamed Shazan $
   $Notice: All Rights Reserved. $
   ======================================================================== */

   
   
   
//////////////////////////////////////////////////////////
//            WIN32 ASYCHRONOUS FILE IO                 //
//////////////////////////////////////////////////////////


#include "main.h"

// WINDOWS CONSOLE vs GUI
#if 1
#define WINDOWS_ENTRY_POINT int main(int argc,char* argv)
#else
#define WINDOWS_ENTRY_POINT int                 \
    CALLBACK WinMain(HINSTANCE Instance,        \
                     HINSTANCE hPrevInstance,   \
                     LPSTR     lpCmdLine,       \
                     int       nCmdShow)

#endif

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600


global_variable bool32 GlobalRunning = true;
global_variable win32_offscreen_buffer GlobalBackBuffer;

internal win32_window_dimension
Win32GetWindowDimension(HWND Window)
{
    win32_window_dimension Result;
    
    RECT ClientRect;
    GetClientRect(Window, &ClientRect);
    Result.Width = ClientRect.right - ClientRect.left;
    Result.Height = ClientRect.bottom - ClientRect.top;

    return Result;
}


internal void
Win32ResizeDIBSection(win32_offscreen_buffer* Buffer, int Width, int Height)
{
    // TODO: Bulletproof this.
    // Maybe don't free first, free after, then free first if that fails.

    if (Buffer->Memory) {
        VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
    }

    Buffer->Width = Width;
    Buffer->Height = Height;

    int BytesPerPixel = 4;
    Buffer->BytesPerPixel = BytesPerPixel;

    // NOTE: When the biHeight field is negative, this is the clue to
    // Windows to treat this bitmap as top-down, not bottom-up, meaning that
    // the first tree bytes of the image are the color for the top left pixel
    // in the bitmap, not the bottom left.
    Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
    Buffer->Info.bmiHeader.biWidth = Buffer->Width;
    Buffer->Info.bmiHeader.biHeight = Buffer->Height;
    Buffer->Info.bmiHeader.biPlanes = 1;
    Buffer->Info.bmiHeader.biBitCount = 32;
    Buffer->Info.bmiHeader.biCompression = BI_RGB;

    Buffer->Pitch = (((Width * BytesPerPixel)+15) & ~15);//Align16
    int BitmapMemorySize = Buffer->Pitch * Buffer->Height;
    Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    // TODO: Probably clear this to black
}

internal void
Win32DisplayBufferInWindow(win32_offscreen_buffer *Buffer, HDC DeviceContext,
                           int WindowWidth, int WindowHeight)
{
    // TODO: Centering / black bars?
    if ((WindowWidth >= Buffer->Width * 2) &&
        (WindowHeight >= Buffer->Height * 2)) {
        StretchDIBits(DeviceContext,
                      0, 0, 2 * Buffer->Width, 2 * Buffer->Height,
                      0, 0, Buffer->Width, Buffer->Height,
                      Buffer->Memory, &Buffer->Info,
                      DIB_RGB_COLORS, SRCCOPY);
    } else {
#if 1
        int OffsetX = 0;
        int OffsetY = 0;

        PatBlt(DeviceContext, 0, 0, WindowWidth, OffsetY, BLACKNESS);
        PatBlt(DeviceContext, 0, OffsetY + Buffer->Height, WindowWidth, WindowHeight, BLACKNESS);
        PatBlt(DeviceContext, 0, 0, OffsetX, WindowHeight, BLACKNESS);
        PatBlt(DeviceContext, OffsetX + Buffer->Width, 0, WindowWidth, WindowHeight, BLACKNESS);

        // NOTE: For prototyping purposes, we're going to always blit
        // 1-to-1 pixels to make sure we don't introduce artifacts with
        // stretching while we are learning to code the renderer!
        StretchDIBits(DeviceContext,
                      /*
                        X, Y, Width, Height,
                        X, Y, Width, Height,
                      */
                      OffsetX, OffsetY, Buffer->Width, Buffer->Height,
                      0, 0, Buffer->Width, Buffer->Height,
                      Buffer->Memory, &Buffer->Info,
                      DIB_RGB_COLORS, SRCCOPY);
#else
        r32 HeightOverWidth = (r32)Buffer->Height / (r32)Buffer->Width;
        s32 Width = WindowWidth;
        s32 Height = (s32)(Width * HeightOverWidth);
        StretchDIBits(DeviceContext,
                      0, 0, Width, Height,
                      0, 0, Buffer->Width, Buffer->Height,
                      Buffer->Memory, &Buffer->Info,
                      DIB_RGB_COLORS, SRCCOPY);
#endif
    }
}

internal void
Win32ProcessPendingMessage() {
    MSG Message;

    while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE)) {
        if (Message.message == WM_QUIT) {
            GlobalRunning = false;
        }

        switch (Message.message) {
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_KEYDOWN:
            case WM_KEYUP: {
                
            }break;
            default: {
                TranslateMessage(&Message);
                DispatchMessage(&Message);
            }
        }
    }

}

LRESULT CALLBACK
Win32MainWindowCallback(HWND Window,
                        UINT Message,
                        WPARAM WParam,
                        LPARAM LParam)
{
    LRESULT Result = 0;

    switch (Message) {
        case WM_CLOSE:
        {
            // TODO: Handle this with a message to the user?
            GlobalRunning = false;
            printf("WM_CLOSE\n");
        } break;

        case WM_ACTIVATEAPP:
        {
            printf("WM_ACTIVATEAPP\n");
#if 0
            if (WParam == TRUE) {
                SetLayeredWindowAttributes(Window, RGB(0, 0, 0), 255, LWA_ALPHA);
            } else {
                SetLayeredWindowAttributes(Window, RGB(0, 0, 0), 64, LWA_ALPHA);
            }
#endif
        } break;

        case WM_DESTROY:
        {
            // TODO: Handle this as an error - recreate window?
            GlobalRunning = false;
            printf("WM_DESTROY\n");
        } break;
        case WM_PAINT:
        {
            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(Window, &Paint);
            int X = Paint.rcPaint.left;
            int Y = Paint.rcPaint.top;
            int Width = Paint.rcPaint.right - Paint.rcPaint.left;
            int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;

            win32_window_dimension Dimension = Win32GetWindowDimension(Window);

            Win32DisplayBufferInWindow(&GlobalBackBuffer, DeviceContext, Dimension.Width, Dimension.Height);
            EndPaint(Window, &Paint);
        } break;
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            assert(!"Keybord input came in through a non-dispatch message");
        } break;
        default:
        {
            //printf("default\n");
            Result = DefWindowProc(Window, Message, WParam, LParam);
        }
    }

    return Result;
}
internal void *Win32AllocateMemory(size_t Size) {
    void *Result = VirtualAlloc(0, Size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    return Result;
}

internal void Win32DeallocateMemory(void* Memory){
    if(Memory){
        VirtualFree(Memory,0,MEM_RELEASE);
    }
}

struct read_file_result{
    void* Contents;
    uint32_t ContentsSize;
};

internal read_file_result Win32ReadEntireFile(const char* Filename){
    read_file_result Result = {};

    HANDLE FileHandle = CreateFileA(Filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if (FileHandle != INVALID_HANDLE_VALUE) {
        LARGE_INTEGER FileSize;
        if (GetFileSizeEx(FileHandle, &FileSize)) {
            uint32_t FileSize32 = (uint32_t)(FileSize.QuadPart);
            Result.Contents = VirtualAlloc(0, FileSize32, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
            if (Result.Contents) {
                DWORD BytesRead;
                if (ReadFile(FileHandle, Result.Contents, FileSize32, &BytesRead, 0)
                    && (FileSize32 == BytesRead)) {
                    // NOTE: File read successfully
                    Result.ContentsSize = FileSize32;
                } else {
                    // TODO: Logging
                    Win32DeallocateMemory(Result.Contents);
                    Result.Contents = 0;
                }
            } else {
                // TODO: Logging
            }
        } else {
            // TODO: Logging
        }

        CloseHandle(FileHandle);
    } else {
        // TODO: Logging
    }

    return Result;
}

internal HANDLE Win32ReadDataFromFile(const char* FileName,void *Dest,uint32_t ContentsSize){

// Create File
    HANDLE FileHandle = CreateFile(FileName,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_FLAG_OVERLAPPED,0);
     if(FileHandle){
     LARGE_INTEGER FileSize;
     if(GetFileSizeEx(FileHandle,&FileSize)){
         DWORD BytesRead;
         uint32_t FileSize32 = (uint32_t)(FileSize.QuadPart);
         ContentsSize = FileSize32;
//Read file
     OVERLAPPED Overlapped = {};
     ReadFile(FileHandle,Dest,FileSize32,&BytesRead,&Overlapped);
     }else{
         //TODO: Logging
     }
     }else{
         //TODO: Logging
     }
     return FileHandle;    

}

internal int Win32WriteEntireFile(const char* Filename,void *Memory,size_t MemorySize){

    int Result = false;
    // Create File Handle
    HANDLE FileHandle = CreateFile(Filename,GENERIC_WRITE,0,0,CREATE_ALWAYS,0,0);
    if(FileHandle){
        DWORD BytesWritten;
    //Write file
        if(WriteFile(FileHandle,Memory,MemorySize,&BytesWritten,0)){
            Result = (BytesWritten == MemorySize);
            }else{
                // TODO: Logging
            }
        CloseHandle(FileHandle);
    }else{
        // TODO: Logging
    }
    return Result;
}

WINDOWS_ENTRY_POINT
{

    WNDCLASS wc;
    ATOM atom;
    RECT rect = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
    DWORD style = WS_OVERLAPPEDWINDOW;
    DWORD exstyle = WS_EX_APPWINDOW;
    HWND window;
    HDC dc;
    /* NOTE: 1080p display mode is 1920x1080 -> Half of that is 960x540
       1920 -> 2048 = 2048 - 1920 -> 128 pixels
       1080 -> 2048 = 2048 - 1080 -> 968 pixels
       1024 + 128 = 1152
    */
    //Win32ResizeDIBSection(&GlobalBackBuffer, 960, 540);
    Win32ResizeDIBSection(&GlobalBackBuffer, 1920, 1080);

    /* Win32 */
    memset(&wc, 0, sizeof(wc));
    wc.lpfnWndProc = Win32MainWindowCallback;
    wc.hInstance = GetModuleHandle(NULL);
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = "WindowClass";
    atom = RegisterClass(&wc);

    window = CreateWindowEx(0, // WS_EX_TOPMOST | WS_EX_LAYERED
                            wc.lpszClassName,
                            "Windows Network Programming",
                            style | WS_VISIBLE,
                            CW_USEDEFAULT,
                            CW_USEDEFAULT,
                            CW_USEDEFAULT,
                            CW_USEDEFAULT,
                            NULL, NULL, wc.hInstance, NULL);
     dc = GetDC(window);

     void *Memory =Win32AllocateMemory(100*1024*1024);
     sqlite3 *db;
    
     struct Data{
         uint32 MagicNumber;
         uint32 FileID;
     };

    int rc = sqlite3_open("database.db", &db);
if(db == NULL){
      fprintf(stderr, "Can't allocate memory for database: %s\n", sqlite3_errmsg(db));
      sqlite3_close(db);
      return(1);
    }
    if( rc ){
      fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
      sqlite3_close(db);
      return(1);
    }else{
        fprintf(stdout,"Sucessfully created database.");
    }
     if(window){
        GlobalRunning = true;
        while (GlobalRunning)
        {
            Win32ProcessPendingMessage();
            
        }

        ReleaseDC(window, dc);
    }
     sqlite3_close(db);
    UnregisterClass(wc.lpszClassName, wc.hInstance);
    return 0;
}
