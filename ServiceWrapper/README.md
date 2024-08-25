Windows service wrapper


Example [`sc create`](https://learn.microsoft.com/en-us/windows-server/administration/windows-commands/sc-create) installation command:
```
sc.exe create MyExeServerCpp start= delayed-auto error= normal binpath= "\"C:\Users\User\Desktop\x64\Debug\ServiceWrapper.exe\" \"C:\Users\User\Desktop\x64\Debug\MyExeServerCpp.exe\"" obj= "LocalSystem" displayname= "MyExeServerCpp service"
```

The service configuration will be stored in the `HKEY_LOCAL_MACHINE\SYSTEM\ControlSet001\Services\MyExeServerCpp` registry folder afterwards.