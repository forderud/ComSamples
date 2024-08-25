Windows service wrapper


Example [`sc create`](https://learn.microsoft.com/en-us/windows-server/administration/windows-commands/sc-create) installation command:
```
sc.exe create MyCustomService start= delayed-auto error= normal binpath= "\"C:\<path>\ServiceWrapper.exe\" \"C:\<path>\MyExeServerCpp.exe\"" obj= "LocalSystem" displayname= "My custom service" 
```

The service configuration will be stored in the `HKEY_LOCAL_MACHINE\SYSTEM\ControlSet001\Services\MyCustomService` registry folder afterwards.