# Tips for debugging of COM marshaling problems

### Symptoms
* Unable to pass COM pointers across process boundaries
* Unable to pass COM pointers between programming languages

### Associated COM error codes
* `E_NOINTERFACE`
* `REGDB_E_IIDNOTREG`
* Probably more...

## Suggested steps
Open `regedit.exe` to check that the required registry entries are present:
1. Ensure that the problematic COM interface is found on `HKCR\Interface\{INTERFACE-GUID}`
1. Locate the associated TypeLib GUID in `HKCR\Interface\{INTERFACE-GUID}\TypeLib`
1. Ensure that the TypeLib can be found on `HKCR\TypeLib\{TYPELIB-GUID}`
1. Ensure that the TypeLib file specified in `HKCR\TypeLib\{TYPELIB-GUID}\<VERSION>\0\win64` is present on the filesystem.
1. Open the IDL file used to generate the TypeLib and ensure that the problematic interface is either mentioned explicitly in the `library` block, or indirectly accessible through interface(s) mentioned in the `library` block.

### `IMyServer` example
![com-interface-reg](https://github.com/user-attachments/assets/dbc4edd0-12b4-4eeb-a046-974f3cb7bf75)  

![typelib-reg](https://github.com/user-attachments/assets/8e86665e-92e0-4366-bd97-8cf8b652f5b6)  

![image](https://github.com/user-attachments/assets/786fe02b-7d20-4dda-83d2-7e896b8cd16b)  
