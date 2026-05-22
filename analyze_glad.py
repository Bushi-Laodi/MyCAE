"""Analyze vtkglad-9.5.dll to check if it tries to load OpenGL in DllMain"""
import pefile
import os

target_dir = r'D:\project\OCCplusVTK\MyCAE\build-release\Release'

# Check vtkglad-9.5.dll
dll_path = os.path.join(target_dir, 'vtkglad-9.5.dll')
pe = pefile.PE(dll_path)

print("=== vtkglad-9.5.dll ===")
print(f"Entry point: 0x{pe.OPTIONAL_HEADER.AddressOfEntryPoint:08x}")
print(f"Image base: 0x{pe.OPTIONAL_HEADER.ImageBase:016x}")
print(f"Subsystem: {pe.OPTIONAL_HEADER.Subsystem}")

# Check sections
print("\nSections:")
for section in pe.sections:
    name = section.Name.decode('utf-8', errors='replace').rstrip('\x00')
    print(f"  {name:10s} VA=0x{section.VirtualAddress:08x} Size=0x{section.SizeOfRawData:08x}")

# Check imports
print("\nImports:")
if hasattr(pe, 'DIRECTORY_ENTRY_IMPORT'):
    for entry in pe.DIRECTORY_ENTRY_IMPORT:
        dll_name = entry.dll.decode('ascii', errors='replace')
        print(f"  From {dll_name}:")
        for imp in entry.imports[:10]:
            if imp.name:
                name = imp.name.decode('ascii', errors='replace')
                print(f"    {name}")

# Check if it imports OpenGL functions
print("\nOpenGL-related imports:")
if hasattr(pe, 'DIRECTORY_ENTRY_IMPORT'):
    for entry in pe.DIRECTORY_ENTRY_IMPORT:
        dll_name = entry.dll.decode('ascii', errors='replace').lower()
        if 'opengl' in dll_name or 'gl' in dll_name:
            print(f"  From {entry.dll.decode('ascii', errors='replace')}:")
            for imp in entry.imports[:20]:
                if imp.name:
                    print(f"    {imp.name.decode('ascii', errors='replace')}")

pe.close()

# Also check vtkRenderingOpenGL2-9.5.dll
print("\n\n=== vtkRenderingOpenGL2-9.5.dll ===")
dll_path2 = os.path.join(target_dir, 'vtkRenderingOpenGL2-9.5.dll')
pe2 = pefile.PE(dll_path2)

print(f"Entry point: 0x{pe2.OPTIONAL_HEADER.AddressOfEntryPoint:08x}")

print("\nImports:")
if hasattr(pe2, 'DIRECTORY_ENTRY_IMPORT'):
    for entry in pe2.DIRECTORY_ENTRY_IMPORT:
        dll_name = entry.dll.decode('ascii', errors='replace')
        print(f"  From {dll_name}:")
        for imp in entry.imports[:5]:
            if imp.name:
                name = imp.name.decode('ascii', errors='replace')
                print(f"    {name}")

pe2.close()

# Check Qt6OpenGL.dll
print("\n\n=== Qt6OpenGL.dll ===")
dll_path3 = os.path.join(target_dir, 'Qt6OpenGL.dll')
pe3 = pefile.PE(dll_path3)
print(f"Entry point: 0x{pe3.OPTIONAL_HEADER.AddressOfEntryPoint:08x}")
pe3.close()
