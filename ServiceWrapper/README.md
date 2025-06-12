Windows service wrapper that allows "regular" executables to be started as a [Windows service](https://learn.microsoft.com/en-us/windows/win32/services/using-services). Replacement for [srvany](https://learn.microsoft.com/en-us/troubleshoot/windows-client/setup-upgrade-and-drivers/create-user-defined-service) that is no longer distributed with the Windows SDK.


Example [`sc create`](https://learn.microsoft.com/en-us/windows-server/administration/windows-commands/sc-create) installation command:
```
sc.exe create MyExeServerAtl start= delayed-auto error= normal binpath= "\"C:\<path>\ServiceWrapper.exe\" \"C:\<path>\MyExeServerAtl.exe\"" obj= "nt authority\localservice" displayname= "MyExeServerAtl service"
```

The service configuration will be stored in the `HKEY_LOCAL_MACHINE\SYSTEM\ControlSet001\Services\MyExeServerAtl` registry folder afterwards.

If launching a COM server, then the COM server needs to have a [`RunAs`](https://learn.microsoft.com/en-us/windows/win32/com/runas) registry parameter with user account matching the `obj= ` account. 
