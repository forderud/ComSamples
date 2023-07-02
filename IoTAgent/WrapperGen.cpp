// Generate C++ convenience wrapper headers
#ifdef _WIN64
  #ifdef _DEBUG
    #import "x64/Debug/IoTAgent.tlb"
  #else
    #import "x64/Release/IoTAgent.tlb"
  #endif
#endif
