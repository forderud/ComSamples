# Tips for debugging of COM marshaling problems

### Symptoms
* Unable to pass COM pointers across process boundaries
* Unable to pass COM pointers between programming languages

### Related COM error codes
* `E_NOINTERFACE`
* `REGDB_E_IIDNOTREG`
* Probably more...

## Suggested steps
Open `regedit.exe` to check that the required registry entries are present:
1. Ensure that the associated COM interface is found on `HKCR\Interface\{INTERFACE-GUID}`
1. Locate the associated TypeLib GUID in HKCR\Interface\{BE3FF6C1-94F5-4974-913C-237C9AB29679}\TypeLib`
1. Ensure that the TypeLib can be found on `HKCR\TypeLib\{TYPELIB-GUID}`
1. Ensure that thje TypeLib file specified in `HKCR\TypeLib\{TYPELIB-GUID}\<VERSION>\0\win64` is present on the filesystem.
