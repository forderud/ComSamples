import comtypes.client
# comtypes will generate TypeLib wrappers that are cached on disk. These are unfortunately not
# regenerated if a COM interface is changed, which can lead to problems. Therefore, please eiher:
# * Delete the "comtypes.client.gen_dir" folder when changing a COM interface without updating the GUID, or
# * Set "comtypes.client.gen_dir = None" during script startup to disable caching (doesn't always work).

def TypeLibForClass(clsid):
    import winreg
    with winreg.OpenKey(winreg.HKEY_CLASSES_ROOT, "CLSID\\"+str(clsid)+"\\TypeLib", 0, winreg.KEY_READ) as key:
        typelib = winreg.EnumValue(key, 0)[1]
    return comtypes.client.GetModule([typelib, 0, 0]) # version 0.0

# MyServer ClassID (CLSID)
SERVER_CLSID = "{AF080472-F173-4D9D-8BE7-435776617347}"
MyInterfaces = TypeLibForClass(SERVER_CLSID)


class MyClientImpl(comtypes.CoClass):
    '''Non-creatable COM class that doesn't need any CLSID'''
    _com_interfaces_ = [MyInterfaces.IMyClient]

    def XmitMessage(self, message):
        msg = message.contents # access Message struct fields
        print("Received message:")
        print("  sev="+str(msg.sev))
        print("  value="+str(msg.value))
        print("  desc="+msg.desc)
        print("  color=[{}, {}, {}]".format(str(msg.color[0]), str(msg.color[1]), str(msg.color[2])))


if __name__=="__main__":
    # create or connect to server object in a separate process
    server = comtypes.client.CreateObject(SERVER_CLSID)
    # cast to IMyServer interface
    server = server.QueryInterface(MyInterfaces.IMyServer)

    # invoke COM methods
    nc = server.GetNumberCruncher()
    pi = nc.ComputePi()
    print("pi = "+str(pi))

    # subscribe to events from server
    obj = MyClientImpl()
    server.Subscribe(obj)

    # Wait a while to give the server a chance to send events.
    # Need to pump messages since we're in a STA.
    comtypes.client.PumpEvents(5)

    # unsubscribe to events
    server.Unsubscribe(obj)
