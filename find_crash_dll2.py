import os
import pefile

target_dir = r'D:\project\OCCplusVTK\MyCAE\build-release\Release'

print("=" * 80)
print("DLL Initialization Crash Analyzer v2")
print("=" * 80)

# Step 1: Get MyCAE.exe imports
print("\n[1] Analyzing MyCAE.exe imports...")
exe_path = os.path.join(target_dir, 'MyCAE.exe')
pe = pefile.PE(exe_path)

exe_imports = set()
for entry in pe.DIRECTORY_ENTRY_IMPORT:
    exe_imports.add(entry.dll.decode('ascii', errors='replace'))
print(f"  MyCAE.exe directly imports {len(exe_imports)} DLLs")

# Step 2: Build full dependency tree (case-insensitive)
print("\n[2] Building full dependency tree...")

# Map lowercase name -> actual filename
all_dlls_map = {}
for f in os.listdir(target_dir):
    if f.lower().endswith('.dll'):
        all_dlls_map[f.lower()] = f
# Also check plugin dirs
for item in os.listdir(target_dir):
    item_path = os.path.join(target_dir, item)
    if os.path.isdir(item_path):
        for f in os.listdir(item_path):
            if f.lower().endswith('.dll'):
                all_dlls_map[f.lower()] = os.path.join(item, f)

# Parse imports for all DLLs
dll_imports = {}
for f in os.listdir(target_dir):
    if f.lower().endswith('.dll'):
        fpath = os.path.join(target_dir, f)
        try:
            p = pefile.PE(fpath)
            imports = set()
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
    
    # Get imports for current DLL
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

# Step 3: Check each DLL for TLS and DllMain
print("\n[3] Checking each DLL for TLS callbacks and DllMain...")
print("  (TLS callbacks in DLLs are a common cause of 0xc0000142)")

issues = []
tls_dlls = []

for dll in dep_order:
    dll_path = os.path.join(target_dir, dll)
    if not os.path.exists(dll_path):
        # Check plugin dirs
        for item in os.listdir(target_dir):
            item_path = os.path.join(target_dir, item)
            if os.path.isdir(item_path):
                plugin_path = os.path.join(item_path, dll)
                if os.path.exists(plugin_path):
                    dll_path = plugin_path
                    break
    
    if not os.path.exists(dll_path):
        continue
    
    try:
        p = pefile.PE(dll_path)
        
        # Check for TLS directory
        has_tls = False
        if hasattr(p, 'DIRECTORY_ENTRY_TLS'):
            has_tls = True
            tls_dlls.append(dll)
        
        # Check for Debug build (look for Debug directory)
        is_debug = False
        if hasattr(p, 'DIRECTORY_ENTRY_DEBUG'):
            is_debug = True
        
        # Check architecture
        is_x64 = p.FILE_HEADER.Machine == 0x8664
        
        # Check DLL characteristics
        dll_char = p.OPTIONAL_HEADER.DllCharacteristics
        
        p.close()
        
        if not is_x64:
            issues.append(f"[ARCH] {dll} - NOT x64 (machine=0x{p.FILE_HEADER.Machine:04x})")
        
        if has_tls:
            print(f"  [TLS] {dll} - has TLS callbacks")
        
    except Exception as e:
        issues.append(f"[ERROR] {dll} - {e}")

if tls_dlls:
    print(f"\n  DLLs with TLS callbacks: {len(tls_dlls)}")
    for d in tls_dlls:
        print(f"    {d}")

# Step 4: Check for Debug DLLs more carefully
print("\n[4] Checking for Debug DLLs...")
debug_dlls = []
for dll in dep_order:
    dll_lower = dll.lower()
    # Check for known debug patterns
    if dll_lower in ['msvcp140d.dll', 'vcruntime140d.dll', 'vcruntime140_1d.dll', 'ucrtbased.dll']:
        debug_dlls.append(dll)
    elif dll_lower.startswith('qt6') and dll_lower.endswith('d.dll'):
        debug_dlls.append(dll)
    elif 'debug' in dll_lower:
        debug_dlls.append(dll)

if debug_dlls:
    print(f"  Found Debug DLLs in dependency chain:")
    for d in debug_dlls:
        issues.append(f"[DEBUG] {d}")
        print(f"    {d}")
else:
    print("  No Debug DLLs found in dependency chain")

# Step 5: Check for missing dependencies
print("\n[5] Checking for missing dependencies...")
for dll in dep_order:
    if dll in dll_imports:
        for imp in dll_imports[dll]:
            imp_lower = imp.lower()
            if imp_lower not in visited and imp_lower not in all_dlls_map:
                # Check if it's a system DLL
                sys_path = os.path.join('C:\\Windows\\System32', imp)
                if not os.path.exists(sys_path):
                    issues.append(f"[MISSING] {dll} -> {imp}")

# Step 6: Check for version conflicts
print("\n[6] Checking for version conflicts...")
# Check if there are both 9.4 and 9.5 VTK DLLs
vtk_94 = [d for d in dep_order if '9.4' in d.lower()]
vtk_95 = [d for d in dep_order if '9.5' in d.lower()]
if vtk_94:
    print(f"  WARNING: VTK 9.4 DLLs found in dependency chain: {vtk_94}")
    issues.append(f"[VERSION] VTK 9.4 DLLs present: {vtk_94}")

# Check for OCC DLLs from both bin and bind
print("\n[7] Checking OCC DLL source (Release vs Debug)...")
occ_bin = r'D:\Tools\cbb_lib\OCC\opencascade-8.0.0-vc14-64-with-debug\opencascade-8.0.0-vc14-64\win64\vc14\bin'
occ_bind = r'D:\Tools\cbb_lib\OCC\opencascade-8.0.0-vc14-64-with-debug\opencascade-8.0.0-vc14-64\win64\vc14\bind'

occ_dlls_in_chain = [d for d in dep_order if d.lower().startswith('tk') and d.lower().endswith('.dll')]
for dll in occ_dlls_in_chain:
    target_path = os.path.join(target_dir, dll)
    if os.path.exists(target_path):
        target_size = os.path.getsize(target_path)
        release_path = os.path.join(occ_bin, dll)
        debug_path = os.path.join(occ_bind, dll)
        release_size = os.path.getsize(release_path) if os.path.exists(release_path) else -1
        debug_size = os.path.getsize(debug_path) if os.path.exists(debug_path) else -1
        
        if target_size == debug_size and target_size != release_size:
            issues.append(f"[OCC_DEBUG] {dll} - from Debug bind (size={target_size}, Release={release_size}, Debug={debug_size})")
            print(f"  [DEBUG_SRC] {dll} - from Debug bind!")

print("\n" + "=" * 80)
print("SUMMARY")
print("=" * 80)
print(f"  DLLs in dependency chain: {len(dep_order)}")
print(f"  Issues found: {len(issues)}")
for issue in issues:
    print(f"    {issue}")

if not issues:
    print("\n  No issues found! The crash may be caused by:")
    print("  1. A bug in the application code (DllMain returning FALSE)")
    print("  2. Missing system prerequisites (VC++ Redistributable)")
    print("  3. Antivirus or security software interference")
    print("  4. Corrupted DLL files")
