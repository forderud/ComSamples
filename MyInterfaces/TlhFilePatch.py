# Script for patching the generated TLH header
# Changes applied:
# * Change TLI include from absolute to relative path
# * Replace C-style structs with C++ RAII structs from cpp_quote lines in the IDL files
# * Insert free functions from cpp_quote lines in the IDL files
import fileinput
import os
import sys


def ExtractCppQuoteFromIDLs(folder):
    '''Extract cpp_quote lines from all IDL files in the given folder''' 
    cpp_content = []
    
    for file in os.listdir(folder):
        if file[-4:] != '.idl':
            continue
        
        #print("Parsing "+file+"...")
        with open(folder+"\\"+file, 'r') as f:
            for line in f.readlines():
                if (line[:11] != 'cpp_quote("'):
                    continue
                
                line = line[11:-3]+"\n"
                line = line.replace('\\"', '"') # remove escaped quotes
                cpp_content.append(line)
    
    return cpp_content


def GetCppStruct(name, cpp_content):
    '''Get the C++ struct body from cpp_quote lines in the IDL files'''
    for i in range(len(cpp_content)):
        if cpp_content[i] != ("struct "+name+" {\n"):
            continue
        
        for j in range(i+1, len(cpp_content)):
            if cpp_content[j] != "};\n": # end of struct check
                continue
        
            # Found struct on lines [i:j+1]
            return "".join(cpp_content[i:j+1])
    
    print("  Could not find cpp_quote for "+name)
    return ""

def ReplaceStructs(source, cpp_content):
    '''Replace C-style structs with cpp_quote C++ RAII structs from the IDL files'''
    for i in range(len(source)):
        line = source[i]
        if line[:7] != "struct ":
            continue
            
        if line[-2:] == ";\n":
            continue
        
        if line[7:7+10] == "__declspec":
            continue # ignore declspec lines
        
        # struct identified in line #i
        name = line.split()[1]
        
        for j in range(i+1, len(source)):
            if source[j] != "};\n": # end of struct check
                continue
                
            # try to replace struct defined on lines [i:j+1]
            print("Replacing "+name)
            cpp_struct = GetCppStruct(name, cpp_content)
            if len(cpp_struct) > 0:
                source[i] = cpp_struct
                for k in range(i+1,j+1):
                    source[k] = "" # blank out existing body
            break
    
    return source


def GetCppFunctions(cpp_content):
    '''Extract inline functions from the C++ content'''
    functions = {}
    
    current_function = []
    current_function_key = ""
    for i in range(len(cpp_content)):
        line = cpp_content[i]
        
        if line[:7] == "inline ":
            # start of function
            prev_line = cpp_content[i-1]
            if prev_line[:2] == "/*":
                current_function.append(prev_line) # Also extract function comment on line above
            current_function.append(line)
            
            # use first function argument type as key
            # WARNING: This is a hack to ensure that the ImageFormatSize & UnitTypeToSR functions
            #          are inserted directly below the ImageFormat and UnitType enum declarations
            current_function_key = line[line.find("(")+1:].split()[0]
        elif current_function:
            # inside function
            current_function.append(line)

        if line == "}\n":
            # end of function
            functions[current_function_key] = current_function
            current_function = []
    
    return functions


def AddFunctionsToSource(source, functions):
    '''Add C++ functions directly below the corresponding enum declarations'''
    for key in functions:
        function_inserted = False
        for line_idx in range(len(source)):
            if source[line_idx] != "enum "+key+"\n":
                continue
            
            for term_idx in range(line_idx+1,len(source)):
                if source[term_idx] != "};\n":
                    continue
                
                print("Inserting function correspoding to enum "+key)
                source = source[:term_idx+1] + functions[key] + source[term_idx+1:]
                function_inserted = True
                break
        
        if not function_inserted:
            raise Exception("Unable to find enum corresponding to "+key)
    
    return source


def MakeTliIncludeRelative(source, tli_file):
    '''Replace #include "<absolute-path>\\<filename>.tli" with #include "<filename>.tli"'''
    # remove path prefix
    idx = tli_file.rfind('\\')
    if idx:
        tli_file = tli_file[idx+1:]
    
    for i in range(len(source)):
        # parse line
        if source[i][:10] != '#include "':
            continue
        
        if '\\'+tli_file.lower()+'"\n' not in source[i]:
            continue
        
        source[i] = '#include "'+tli_file+'"\n'
        print("TLI include patched.")
    
    return source


def ParseUuidString (uuid):
    '''Return uuid string on {0xFFFFFFFF,0xFFFF,0xFFFF,{0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF}} form'''
    # identify UUID
    uuid = ''.join(uuid.split('-'))
    uuid = '{0x'+uuid[:8]+',0x'+uuid[8:12]+',0x'+uuid[12:16]+',{0x'+uuid[16:18]+',0x'+uuid[18:20]+',0x'+uuid[20:22]+',0x'+uuid[22:24]+',0x'+uuid[24:26]+',0x'+uuid[26:28]+',0x'+uuid[28:30]+',0x'+uuid[30:32]+'}}'
    return uuid


def ExtractIncludes(cpp_content):
    '''Extract #include statements from the cpp_quote lines.'''
    includes = set() # set to avoid duplicates
    
    for line in cpp_content:
        if "#include" not in line:
            continue
        
        includes.add(line)
    
    return list(sorted(includes))


def PatchTlhFile(tlh_file_in, tlh_file_out, tli_file_in, remove_header):
    this_script_dir = os.path.dirname(os.path.abspath(__file__))
    cpp_content = ExtractCppQuoteFromIDLs(this_script_dir)
    
    with open(tlh_file_in, 'r') as f:
        source = f.readlines()
    
    if remove_header:
        # remove first 6 lines with comments (first line also contain UTF BOM header)
        # done to make the content deterministic and suitable for versioning
        source = source[6:]
    
    source = MakeTliIncludeRelative(source, tli_file_in)
    source = ReplaceStructs(source, cpp_content)
    
    functions = GetCppFunctions(cpp_content)
    source = AddFunctionsToSource(source, functions)
    
    # add includes at the top of the file
    source = ExtractIncludes(cpp_content) + source
    
    with open(tlh_file_out, 'w', newline='\r\n') as f:
        for line in source:
            f.write(line)


def PatchTliFile(tli_file_in, tli_file_out, remove_header):
    with open(tli_file_in, 'r') as f:
        source = f.readlines()
    
    if remove_header:
        # remove first 6 lines with comments
        # done to make the content deterministic and suitable for versioning
        source = source[6:]
        
    with open(tli_file_out, 'w', newline='\r\n') as f:
        for line in source:
            f.write(line)


if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: TlhFilePatch.py <tlh-file-in> <tlh-file-out>")
        sys.exit(1)
    
    tlh_file_in  = sys.argv[1] # e.g. "appapi.tlh"
    tlh_file_out = sys.argv[2]
    
    tli_file_in  = tlh_file_in[:tlh_file_in.rfind(".")]+".tli"   # change extension
    tli_file_out = tlh_file_out[:tlh_file_out.rfind(".")]+".tli" # change extension
    
    remove_header = True # remove non-deterministic header containing time-stamps
    
    PatchTlhFile(tlh_file_in, tlh_file_out, tli_file_in, remove_header)
    PatchTliFile(tli_file_in, tli_file_out, remove_header)
