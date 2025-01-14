#include <Windows.h>

// DLL Entry Point
extern "C"
BOOL DllMain(HINSTANCE /*hInstance*/, DWORD reason, LPVOID reserved) {
    // Perform actions based on the reason for calling.
    switch (reason)
    {
    case DLL_PROCESS_ATTACH:
        // Initialize once for each new process.
        break; // Return FALSE to fail DLL load.

    case DLL_THREAD_ATTACH:
        // thread-specific initialization.
        break;

    case DLL_THREAD_DETACH:
        // thread-specific cleanup.
        break;

    case DLL_PROCESS_DETACH:
        if (reserved != nullptr) {
            // process termination
            // skip cleanup
            break;
        } else {
            // FreeLibrary has been called
            // perform cleanup.
            break;
        }
    }

    return TRUE; // Success
}
