import comtypes.client

# Disable disk caching of TypeLib wrappers. Avoids failures on interface changes.
# Folder might need to be deleted if already containing TypeLibs.
comtypes.client.gen_dir = None

def TypeLibForClass (clsid):
    import winreg
    with winreg.OpenKey(winreg.HKEY_CLASSES_ROOT, "CLSID\\"+str(clsid)+"\\TypeLib", 0, winreg.KEY_READ) as key:
        typelib = winreg.EnumValue(key, 0)[1]
    return comtypes.client.GetModule([typelib, 0, 0]) # version 0.0


if __name__=="__main__":
    # MyServer ClassID (CLSID)
    clsid = "{AF080472-F173-4D9D-8BE7-435776617347}"
    MyInterfaces = TypeLibForClass(clsid)

    # create or connect to server object in a separate process
    server = comtypes.client.CreateObject(clsid)
    # cast to IMyServer interface
    server = server.QueryInterface(MyInterfaces.IMyServer)

    # invoke COM methods
    nc = server.GetNumberCruncher()
    pi = nc.ComputePi()
    print("pi: "+str(pi))