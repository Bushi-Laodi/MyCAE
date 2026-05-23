"""Check for VTK autoinit code in MyCAE.exe"""
import pefile
import os
import re

target_dir = r'D:\project\OCCplusVTK\MyCAE\build-release\Release'
exe_path = os.path.join(target_dir, 'MyCAE.exe')

pe = pefile.PE(exe_path)

# Search for vtkModuleAutoInit strings in the exe
print('Searching for VTK autoinit strings in MyCAE.exe...')
data = pe.get_memory_mapped_image()
# Look for 'vtkRenderingOpenGL2_AutoInit' or similar strings
matches = re.findall(rb'[A-Za-z_][A-Za-z0-9_]*AutoInit[A-Za-z0-9_]*', data)
for m in set(matches):
    print(f'  Found: {m.decode("ascii", errors="replace")}')

# Also search for 'vtkRenderingOpenGL2' in the import section
print('\nSearching for vtkRenderingOpenGL2 references...')
if hasattr(pe, 'DIRECTORY_ENTRY_IMPORT'):
    for entry in pe.DIRECTORY_ENTRY_IMPORT:
        dll = entry.dll.decode('ascii', errors='replace')
        if 'vtk' in dll.lower() or 'opengl' in dll.lower():
            print(f'  Import from {dll}:')
            for imp in entry.imports[:3]:
                if imp.name:
                    name = imp.name.decode('ascii', errors='replace')
                    if 'AutoInit' in name or 'Init' in name:
                        print(f'    {name}')

# Check for static initializers in the exe
print('\nChecking for static initializer patterns...')
# Look for _initterm references
if hasattr(pe, 'DIRECTORY_ENTRY_IMPORT'):
    for entry in pe.DIRECTORY_ENTRY_IMPORT:
        dll = entry.dll.decode('ascii', errors='replace')
        for imp in entry.imports:
            if imp.name:
                name = imp.name.decode('ascii', errors='replace')
                if '_initterm' in name:
                    print(f'  {name} from {dll}')

pe.close()
