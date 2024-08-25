// Official sample: https://learn.microsoft.com/en-us/windows/win32/services/svc-cpp
#include <Windows.h>
#include <memory>
#include <string>

SERVICE_STATUS_HANDLE g_StatusHandle = NULL;
HANDLE                g_ServiceStopEvent = INVALID_HANDLE_VALUE; // weak-ref to ServiceMain variable


/* Set the current state of the service. */
void ServiceSetState(DWORD acceptedControls, DWORD newState, DWORD exitCode) {
    SERVICE_STATUS serviceStatus = {};
    serviceStatus.dwCheckPoint = 0;
    serviceStatus.dwControlsAccepted = acceptedControls;
    serviceStatus.dwCurrentState = newState;
    serviceStatus.dwServiceSpecificExitCode = 0;
    serviceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    serviceStatus.dwWaitHint = 0;
    serviceStatus.dwWin32ExitCode = exitCode;

    if (!SetServiceStatus(g_StatusHandle, &serviceStatus)) {
        OutputDebugStringW(L"SetServiceStatus() failed\n");
    }
}


/* Handle service control requests, like STOP, PAUSE and CONTINUE. */
void WINAPI ServiceCtrlHandler(DWORD CtrlCode) {
    switch (CtrlCode) {
    case SERVICE_CONTROL_STOP:
        SetEvent(g_ServiceStopEvent); // signal service stop
        ServiceSetState(0, SERVICE_STOPPED, 0);
        break;

    case SERVICE_CONTROL_PAUSE:
        ServiceSetState(0, SERVICE_PAUSED, 0);
        break;

    case SERVICE_CONTROL_CONTINUE:
        ServiceSetState(0, SERVICE_RUNNING, 0);
        break;

    case SERVICE_CONTROL_INTERROGATE:
        break;

    default:
        break;
    }
}


/* Main entry point for the service.Acts in a similar fasion to main().
 * NOTE: svc_argv[0] will always contain the service name when invoked by the SCM. Command-line arguments are NOT forwarded here. */
void WINAPI ServiceMain(DWORD svc_argc, WCHAR* svc_argv[]) {
    UNREFERENCED_PARAMETER(svc_argc);
    const WCHAR* ServiceName = svc_argv[0];

    // retrieve command-line arguments programatically
    WCHAR* args = GetCommandLineW();
    while (*args != L' ') // skip argv[0] which contain this executable name
        args++;
    args++;

    g_StatusHandle = RegisterServiceCtrlHandlerW(ServiceName, ServiceCtrlHandler);
    if (!g_StatusHandle) {
        OutputDebugStringW(L"RegisterServiceCtrlHandler() failed\n");
        ServiceSetState(0, SERVICE_STOPPED, GetLastError());
        return;
    }

    std::unique_ptr<std::remove_pointer<HANDLE>::type, BOOL(*)(HANDLE)> ServiceStopEvent(CreateEventW(NULL, TRUE, FALSE, NULL), CloseHandle);
    g_ServiceStopEvent = ServiceStopEvent.get(); // allow access from other functions

    STARTUPINFOW startupInfo = {};
    startupInfo.cb = sizeof(STARTUPINFOW);
    startupInfo.wShowWindow = 0;
    startupInfo.lpReserved = NULL;
    startupInfo.cbReserved2 = 0;
    startupInfo.lpReserved2 = NULL;

    // try to create target process
    const WCHAR* procEnv = nullptr;
    const WCHAR* procCurDir = nullptr;
    PROCESS_INFORMATION process = {};
    if (!CreateProcessW(NULL, args, NULL, NULL, FALSE, CREATE_NO_WINDOW | CREATE_UNICODE_ENVIRONMENT, (void*)procEnv, procCurDir, &startupInfo, &process)) {
        ServiceSetState(0, SERVICE_STOPPED, 0);
        return;
    }
        
    // process created
    ServiceSetState(SERVICE_ACCEPT_STOP, SERVICE_RUNNING, 0);

    // Keep the service alive until stopped or the target application exits
    while (WaitForSingleObject(ServiceStopEvent.get(), 0) != WAIT_OBJECT_0) {
        // signal stop-event if the target process has exited
        if (WaitForSingleObject(process.hProcess, 0) == WAIT_OBJECT_0)
            SetEvent(ServiceStopEvent.get());

        Sleep(1000);
    }

    TerminateProcess(process.hProcess, 0); // kill the target process if it's still running

    ServiceSetState(0, SERVICE_STOPPED, 0);
}


/* Main entry point for the application.
 * NOTE: The SCM calls this, just like any other application. */
int wmain(int argc, WCHAR* argv[]) {
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    const SERVICE_TABLE_ENTRYW DispatchTable[] = {
        { (WCHAR*)L"ServiceWraper", (LPSERVICE_MAIN_FUNCTION)ServiceMain}, // name ignored for SERVICE_WIN32_OWN_PROCESS services
        { NULL, NULL }
    };

    if (StartServiceCtrlDispatcherW(DispatchTable) == FALSE) {
        return GetLastError();
    }

    return 0;
}
