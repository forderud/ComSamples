Windows service wrapper that allows "regular" executables to be started as a Windows service. Replacement for [srvany](https://learn.microsoft.com/en-us/troubleshoot/windows-client/setup-upgrade-and-drivers/create-user-defined-service) that is no longer distributed with the Windows SDK.


Example [`sc create`](https://learn.microsoft.com/en-us/windows-server/administration/windows-commands/sc-create) installation command:
```
sc.exe create MyExeServerCpp start= delayed-auto error= normal binpath= "\"C:\<path>\ServiceWrapper.exe\" \"C:\<path>\MyExeServerCpp.exe\"" obj= "nt authority\localservice" displayname= "MyExeServerCpp service"
```

The service configuration will be stored in the `HKEY_LOCAL_MACHINE\SYSTEM\ControlSet001\Services\MyExeServerCpp` registry folder afterwards.