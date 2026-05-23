"""Check vtkCommonCore-9.5.dll for static initialization issues"""
import pefile
import os

target_dir = r'D:\project\OCCplusVTK\MyCAE\build-release\Release'

dll_path = os.path.join(target_dir, 'vtkCommonCore-9.5.dll')
pe = pefile.PE(dll_path)

print('=== vtkCommonCore-9.5.dll ===')
print(f'Entry point: 0x{pe.OPTIONAL_HEADER.AddressOfEntryPoint:08x}')

# Check if it has TLS callbacks
if hasattr(pe, 'DIRECTORY_ENTRY_TLS'):
    tls = pe.DIRECTORY_ENTRY_TLS.struct
    print(f'Has TLS callbacks')
    print(f'  AddressOfCallBacks: 0x{tls.AddressOfCallBacks:016x}')

# Check imports from MSVCP140 and VCRUNTIME
print('\nCRT imports:')
if hasattr(pe, 'DIRECTORY_ENTRY_IMPORT'):
    for entry in pe.DIRECTORY_ENTRY_IMPORT:
        dll = entry.dll.decode('ascii', errors='replace')
        if 'msvcp140' in dll.lower() or 'vcruntime' in dll.lower() or 'api-ms' in dll.lower():
            print(f'  From {dll}:')
            for imp in entry.imports[:15]:
                if imp.name:
                    print(f'    {imp.name.decode("ascii", errors="replace")}')

# Check for _Mtx_init import
print('\nChecking for mutex-related imports:')
if hasattr(pe, 'DIRECTORY_ENTRY_IMPORT'):
    for entry in pe.DIRECTORY_ENTRY_IMPORT:
        for imp in entry.imports:
            if imp.name:
                name = imp.name.decode('ascii', errors='replace')
                if 'mtx' in name.lower() or 'mutex' in name.lower() or 'lock' in name.lower():
                    print(f'  {name} from {entry.dll.decode("ascii", errors="replace")}')

pe.close()

# Also check vtkRenderingOpenGL2-9.5.dll
print('\n\n=== vtkRenderingOpenGL2-9.5.dll ===')
dll_path2 = os.path.join(target_dir, 'vtkRenderingOpenGL2-9.5.dll')
pe2 = pefile.PE(dll_path2)
print(f'Entry point: 0x{pe2.OPTIONAL_HEADER.AddressOfEntryPoint:08x}')

# Check for DllMain-related imports
print('\nChecking for DllMain-related imports:')
if hasattr(pe2, 'DIRECTORY_ENTRY_IMPORT'):
    for entry in pe2.DIRECTORY_ENTRY_IMPORT:
        for imp in entry.imports:
            if imp.name:
                name = imp.name.decode('ascii', errors='replace')
                if 'dllmain' in name.lower() or 'dll' in name.lower() or 'initterm' in name.lower():
                    print(f'  {name} from {entry.dll.decode("ascii", errors="replace")}')

pe2.close()

# Check Qt6Core.dll for DllMain
print('\n\n=== Qt6Core.dll ===')
dll_path3 = os.path.join(target_dir, 'Qt6Core.dll')
pe3 = pefile.PE(dll_path3)
print(f'Entry point: 0x{pe3.OPTIONAL_HEADER.AddressOfEntryPoint:08x}')
pe3.close()

# Check Qt6Gui.dll for DllMain
print('\n\n=== Qt6Gui.dll ===')
dll_path4 = os.path.join(target_dir, 'Qt6Gui.dll')
pe4 = pefile.PE(dll_path4)
print(f'Entry point: 0x{pe4.OPTIONAL_HEADER.AddressOfEntryPoint:08x}')
pe4.close()
