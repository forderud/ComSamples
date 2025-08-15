#pragma once
#include <stdexcept>

#pragma comment(lib, "Rpcrt4.lib") // for RpcServerInqCallAttributesW


/** Get client process ID (PID) during handling of incoming RPC call.
    Only supported for the "ncalrpc" local RPC protocol. */
inline DWORD GetClientProcessID(RPC_BINDING_HANDLE binding = nullptr) {
    RPC_CALL_ATTRIBUTES attribs{};
    attribs.Version = RPC_CALL_ATTRIBUTES_VERSION; // 3
    attribs.Flags = RPC_QUERY_CLIENT_PID;
    RPC_STATUS status = RpcServerInqCallAttributesW(binding, &attribs);
    if (status != RPC_S_OK)
        throw std::runtime_error("RPC_QUERY_CLIENT_PID failed");

    return (DWORD)(size_t)(attribs.ClientPID); // truncating cast should be safe, since GetCurrentProcessId() returns a DWORD
}
