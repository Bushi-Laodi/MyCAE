"""
Find which DLL's DllMain returns FALSE by checking:
1. DLL entry point (DllMain) address
2. Whether the DLL has a non-default entry point
3. Known problematic DLLs that often fail in DllMain
"""
import os
import pefile

target_dir = r'D:\project\OCCplusVTK\MyCAE\build-release\Release'

print("=" * 80)
print("DLL DllMain Analyzer")
print("=" * 80)

# Step 1: Get MyCAE.exe imports
print("\n[1] Analyzing MyCAE.exe imports...")
exe_path = os.path.join(target_dir, 'MyCAE.exe')
pe = pefile.PE(exe_path)

exe_imports = set()
for entry in pe.DIRECTORY_ENTRY_IMPORT:
    exe_imports.add(entry.dll.decode('ascii', errors='replace'))
print(f"  MyCAE.exe directly imports {len(exe_imports)} DLLs")

# Step 2: Build full dependency tree
print("\n[2] Building full dependency tree...")

all_dlls_map = {}
for f in os.listdir(target_dir):
    if f.lower().endswith('.dll'):
        all_dlls_map[f.lower()] = f

dll_imports = {}
for f in os.listdir(target_dir):
    if f.lower().endswith('.dll'):
        fpath = os.path.join(target_dir, f)
        try:
            p = pefile.PE(fpath)
            imports = set()
            if hasattr(p, 'DIRECTORY_ENTRY_IMPORT'):
                for entry in p.DIRECTORY_ENTRY_IMPORT:
                    imports.add(entry.dll.decode('ascii', errors='replace'))
            dll_imports[f] = imports
            p.close()
        except:
            pass

# BFS from MyCAE.exe
visited = set()
queue = ['MyCAE.exe']
dep_order = []

while queue:
    current = queue.pop(0)
    if current.lower() in visited:
        continue
    visited.add(current.lower())
    
    if current != 'MyCAE.exe':
        dep_order.append(current)
    
    imports = set()
    if current == 'MyCAE.exe':
        imports = exe_imports
    elif current in dll_imports:
        imports = dll_imports[current]
    
    for imp in imports:
        imp_lower = imp.lower()
        if imp_lower not in visited and imp_lower in all_dlls_map:
            actual_name = all_dlls_map[imp_lower]
            queue.append(actual_name)

print(f"  Dependency chain has {len(dep_order)} DLLs")

# Step 3: Check each DLL's entry point and characteristics
print("\n[3] Checking each DLL's entry point and DllMain...")

problematic_dlls = []

for dll in dep_order:
    dll_path = os.path.join(target_dir, dll)
    if not os.path.exists(dll_path):
        continue
    
    try:
        p = pefile.PE(dll_path)
        
        # Get entry point
        entry_point = p.OPTIONAL_HEADER.AddressOfEntryPoint
        
        # Check if DLL has TLS callbacks
        has_tls = hasattr(p, 'DIRECTORY_ENTRY_TLS')
        
        # Check DLL characteristics
        dll_char = p.OPTIONAL_HEADER.DllCharacteristics
        
        # Check if it's a .NET assembly (managed DLL)
        is_dotnet = False
        for section in p.sections:
            if section.Name.decode('utf-8', errors='replace').rstrip('\x00') == '.text':
                # Check for CLR header
                if hasattr(p, 'DIRECTORY_ENTRY_COM_DESCRIPTOR'):
                    is_dotnet = True
                break
        
        # Check image base
        image_base = p.OPTIONAL_HEADER.ImageBase
        
        # Check subsystem
        subsystem = p.OPTIONAL_HEADER.Subsystem
        
        # Check if DLL has a proper entry point
        # A DLL with entry point 0 means it has no DllMain
        has_dllmain = entry_point != 0
        
        p.close()
        
        # Check for known problematic patterns
        issues = []
        
        # DLLs that often have DllMain issues:
        # 1. OpenGL-related DLLs (try to create OpenGL context in DllMain)
        # 2. DLLs with complex static initialization
        # 3. DLLs that load other DLLs in DllMain
        
        dll_lower = dll.lower()
        
        if 'opengl' in dll_lower or 'glad' in dll_lower:
            issues.append("OpenGL-related DLL - may try to load OpenGL in DllMain")
        
        if has_tls and has_dllmain:
            issues.append("Has TLS callbacks AND DllMain - complex initialization")
        
        if not has_dllmain:
            issues.append("No DllMain entry point (resource-only DLL?)")
        
        if issues:
            problematic_dlls.append((dll, issues))
            print(f"  [CHECK] {dll}:")
            for issue in issues:
                print(f"    - {issue}")
        
    except Exception as e:
        print(f"  [ERROR] {dll}: {e}")

# Step 4: Check for DLLs that import from MSVCP140.dll
print("\n[4] Checking MSVCP140.dll dependencies...")
msvcp_dependents = []
for dll in dep_order:
    if dll in dll_imports:
        for imp in dll_imports[dll]:
            if 'msvcp140' in imp.lower() or 'vcruntime140' in imp.lower():
                msvcp_dependents.append(dll)
                break

print(f"  {len(msvcp_dependents)} DLLs depend on MSVCP140.dll")

# Step 5: Check for DLLs that might have circular dependencies
print("\n[5] Checking for potential circular dependencies...")
# Build reverse dependency map
reverse_deps = {}
for dll, imports in dll_imports.items():
    for imp in imports:
        imp_lower = imp.lower()
        if imp_lower not in reverse_deps:
            reverse_deps[imp_lower] = []
        reverse_deps[imp_lower].append(dll)

# Check for DLLs that import from each other
for dll in dep_order[:20]:  # Check first 20
    dll_lower = dll.lower()
    if dll_lower in reverse_deps:
        for dependent in reverse_deps[dll_lower]:
            if dependent.lower() in dll_imports:
                if dll_lower in [i.lower() for i in dll_imports[dependent]]:
                    print(f"  [CIRCULAR] {dll} <-> {dependent}")

print("\n" + "=" * 80)
print("SUMMARY")
print("=" * 80)
print(f"  DLLs in dependency chain: {len(dep_order)}")
print(f"  DLLs with potential DllMain issues: {len(problematic_dlls)}")
for dll, issues in problematic_dlls:
    print(f"    {dll}:")
    for issue in issues:
        print(f"      - {issue}")

print("\n  Most likely causes of 0xC0000142:")
print("  1. A DLL's DllMain returns FALSE during initialization")
print("  2. A DLL's static initializer throws an exception")
print("  3. A DLL's TLS callback crashes")
print("  4. Circular dependency between DLLs")
print("\n  To diagnose further, use Dependency Walker (depends.exe)")
print("  or enable 'Load Image' logging with gflags.exe")
