// Generate C++ convenience wrapper headers
#ifdef _WIN64
  #ifdef _DEBUG
    #import "x64/Debug/MyInterfaces.tlb"
  #else
    #import "x64/Release/MyInterfaces.tlb"
  #endif
#endif
