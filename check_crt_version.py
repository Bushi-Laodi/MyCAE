"""Check CRT version requirements of Qt and VTK DLLs"""
import pefile
import os

target_dir = r'D:\project\OCCplusVTK\MyCAE\build-release\Release'

dlls_to_check = ['Qt6Core.dll', 'Qt6Gui.dll', 'Qt6Widgets.dll', 'Qt6OpenGL.dll',
                 'vtkCommonCore-9.5.dll', 'vtkRenderingOpenGL2-9.5.dll',
                 'vtkGUISupportQt-9.5.dll']

for dll_name in dlls_to_check:
    dll_path = os.path.join(target_dir, dll_name)
    if not os.path.exists(dll_path):
        print(f'{dll_name}: NOT FOUND')
        continue
    
    pe = pefile.PE(dll_path)
    print(f'\n=== {dll_name} ===')
    print(f'Entry point: 0x{pe.OPTIONAL_HEADER.AddressOfEntryPoint:08x}')
    
    # Check CRT imports
    if hasattr(pe, 'DIRECTORY_ENTRY_IMPORT'):
        for entry in pe.DIRECTORY_ENTRY_IMPORT:
            dll = entry.dll.decode('ascii', errors='replace')
            if 'msvcp' in dll.lower() or 'vcruntime' in dll.lower():
                print(f'  Requires: {dll}')
                for imp in entry.imports[:3]:
                    if imp.name:
                        name = imp.name.decode('ascii', errors='replace')
                        print(f'    {name}')
    
    pe.close()

# Also check the actual CRT DLLs in the target directory
print('\n\n=== CRT DLLs in target directory ===')
for f in os.listdir(target_dir):
    if f.lower().startswith('msvcp') or f.lower().startswith('vcruntime') or f.lower().startswith('concrt'):
        fpath = os.path.join(target_dir, f)
        pe = pefile.PE(fpath)
        print(f'{f}: entry=0x{pe.OPTIONAL_HEADER.AddressOfEntryPoint:08x}')
        pe.close()
