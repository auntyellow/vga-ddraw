#include <windows.h>
#include <stdio.h>

// Global handles for resource cleanup
HANDLE hEventBufferReady = NULL;
HANDLE hEventDataReady   = NULL;
HANDLE hSharedFile       = NULL;
char*  pBuffer           = NULL;
BOOL   bRunning          = TRUE;

// Control handler to catch Ctrl+C or Close window signals
BOOL WINAPI ConsoleHandler(DWORD dwType) {
    if (dwType == CTRL_C_EVENT || dwType == CTRL_CLOSE_EVENT) {
        printf("\nStopping monitor and cleaning up resources...\n");
        bRunning = FALSE;
        // Returning TRUE tells the OS we've handled the signal
        return TRUE; 
    }
    return FALSE;
}

int main() {
    DWORD waitResult;
    // 1. Register the console control handler
    if (!SetConsoleCtrlHandler(ConsoleHandler, TRUE)) {
        printf("Error: Could not register console control handler.\n");
        return 1;
    }

    // 2. Initialize Synchronization Events
    // DBWIN_BUFFER_READY: Signal that the monitor is ready to receive
    // DBWIN_DATA_READY:   Signal from the application that data is written
    hEventBufferReady = CreateEventA(NULL, FALSE, FALSE, "DBWIN_BUFFER_READY");
    hEventDataReady   = CreateEventA(NULL, FALSE, FALSE, "DBWIN_DATA_READY");
    
    // 3. Create the shared memory buffer (standard 4KB for NT)
    hSharedFile = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 4096, "DBWIN_BUFFER");

    if (!hEventBufferReady || !hEventDataReady || !hSharedFile) {
        printf("Initialization failed.\n");
        printf("Ensure no other debugger (like DebugView) is currently running.\n");
        return 1;
    }

    // Map the shared memory into our process address space
    pBuffer = (char*)MapViewOfFile(hSharedFile, FILE_MAP_READ, 0, 0, 4096);
    
    printf("--- NT Debug Monitor Started ---\n");
    printf("Listening for OutputDebugStringA calls...\n");
    printf("Press Ctrl+C to exit safely.\n\n");

    while (bRunning) {
        // Signal the system that we are ready for the next message
        SetEvent(hEventBufferReady);

        // Wait for data with a timeout (500ms) to keep the loop responsive to bRunning
        waitResult = WaitForSingleObject(hEventDataReady, 500);

        if (waitResult == WAIT_OBJECT_0) {
            // First 4 bytes are the Process ID
            DWORD pid = *(DWORD*)pBuffer;
            // The actual message starts after the DWORD
            const char* message = pBuffer + sizeof(DWORD);
            
            printf("[PID %lu]: %s\n", pid, message);
        }
        else if (waitResult == WAIT_TIMEOUT) {
            // No message yet, check bRunning and loop again
            continue;
        }
    }

    // 4. Clean up resources
    if (pBuffer)           UnmapViewOfFile(pBuffer);
    if (hSharedFile)       CloseHandle(hSharedFile);
    if (hEventBufferReady) CloseHandle(hEventBufferReady);
    if (hEventDataReady)   CloseHandle(hEventDataReady);

    printf("Monitor terminated successfully.\n");
    return 0;
}