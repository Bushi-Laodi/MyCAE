import os
import subprocess
import struct
import tempfile

def get_dll_imports(filepath):
    """Get all imported DLLs from a PE file using dumpbin if available, else pefile"""
    try:
        import pefile
        pe = pefile.PE(filepath)
        imports = set()
        for entry in pe.DIRECTORY_ENTRY_IMPORT:
            imports.add(entry.dll.decode('ascii', errors='replace'))
        pe.close()
        return imports
    except ImportError:
        # Fallback: use dumpbin
        try:
            result = subprocess.run(
                ['dumpbin', '/imports', filepath],
                capture_output=True, text=True, timeout=10
            )
            imports = set()
            for line in result.stdout.split('\n'):
                line = line.strip()
                if line.endswith('.dll') and ' ' not in line:
                    imports.add(line)
            return imports
        except:
            return set()

def find_dll_init_crash(target_dir, exe_name):
    """
    Find which DLL causes the 0xc0000142 crash by binary search.
    Uses the fact that 0xc0000142 means a DLL's DllMain returned FALSE.
    """
    import pefile
    
    print("=" * 80)
    print("DLL Initialization Crash Analyzer")
    print("=" * 80)
    
    # Step 1: Build the full dependency tree
    print("\n[Step 1] Building dependency tree...")
    
    all_dlls = {}
    for f in os.listdir(target_dir):
        if f.lower().endswith('.dll'):
            fpath = os.path.join(target_dir, f)
            try:
                pe = pefile.PE(fpath)
                imports = set()
                for entry in pe.DIRECTORY_ENTRY_IMPORT:
                    imports.add(entry.dll.decode('ascii', errors='replace'))
                all_dlls[f] = imports
                pe.close()
            except:
                pass
    
    # Also check plugin subdirectories
    for item in os.listdir(target_dir):
        item_path = os.path.join(target_dir, item)
        if os.path.isdir(item_path):
            for f in os.listdir(item_path):
                if f.lower().endswith('.dll'):
                    fpath = os.path.join(item_path, f)
                    try:
                        pe = pefile.PE(fpath)
                        imports = set()
                        for entry in pe.DIRECTORY_ENTRY_IMPORT:
                            imports.add(entry.dll.decode('ascii', errors='replace'))
                        all_dlls[f] = imports
                        pe.close()
                    except:
                        pass
    
    print(f"  Found {len(all_dlls)} DLLs with import info")
    
    # Step 2: Build the dependency graph from MyCAE.exe
    print("\n[Step 2] Building dependency graph from MyCAE.exe...")
    
    exe_path = os.path.join(target_dir, exe_name)
    if not os.path.exists(exe_path):
        print(f"  ERROR: {exe_path} not found!")
        return
    
    # BFS from MyCAE.exe
    visited = set()
    queue = [exe_name.lower()]
    dep_order = []  # DLLs in order they would be loaded
    
    while queue:
        current = queue.pop(0)
        if current in visited:
            continue
        visited.add(current)
        
        # Normalize name
        current_normalized = None
        for dll_name in all_dlls:
            if dll_name.lower() == current:
                current_normalized = dll_name
                break
        
        if current_normalized:
            dep_order.append(current_normalized)
            for imp in all_dlls[current_normalized]:
                imp_lower = imp.lower()
                if imp_lower not in visited:
                    # Check if this DLL exists in target dir
                    found = False
                    for dll_name in all_dlls:
                        if dll_name.lower() == imp_lower:
                            queue.append(dll_name)
                            found = True
                            break
                    if not found:
                        # Check in plugin dirs
                        for item in os.listdir(target_dir):
                            item_path = os.path.join(target_dir, item)
                            if os.path.isdir(item_path):
                                for f in os.listdir(item_path):
                                    if f.lower() == imp_lower:
                                        queue.append(f)
                                        found = True
                                        break
                            if found:
                                break
    
    print(f"  Dependency chain has {len(dep_order)} DLLs")
    
    # Step 3: Check each DLL for potential issues
    print("\n[Step 3] Checking each DLL for potential initialization issues...")
    
    # Check for known problematic patterns
    issues = []
    
    for dll in dep_order:
        dll_path = None
        # Check main dir
        main_path = os.path.join(target_dir, dll)
        if os.path.exists(main_path):
            dll_path = main_path
        else:
            # Check plugin dirs
            for item in os.listdir(target_dir):
                item_path = os.path.join(target_dir, item)
                if os.path.isdir(item_path):
                    plugin_path = os.path.join(item_path, dll)
                    if os.path.exists(plugin_path):
                        dll_path = plugin_path
                        break
        
        if not dll_path:
            issues.append(f"[MISSING] {dll} - not found in target directory")
            continue
        
        # Check file size
        size = os.path.getsize(dll_path)
        
        # Check if it's a known problematic DLL
        dll_lower = dll.lower()
        
        # Check for Debug DLLs
        if dll_lower.endswith('d.dll') and not dll_lower.startswith('tk'):
            # Skip OCC DLLs that naturally end with 'd' (TKG2d, TKG3d, TKStd, TKV3d)
            if dll_lower not in ['tkg2d.dll', 'tkg3d.dll', 'tkstd.dll', 'tkv3d.dll']:
                issues.append(f"[DEBUG] {dll} - Debug DLL in Release build (size={size})")
        
        # Check for known problematic DLLs
        if 'debug' in dll_lower:
            issues.append(f"[DEBUG] {dll} - Debug DLL in Release build (size={size})")
        
        # Check for DLLs that import missing dependencies
        if dll in all_dlls:
            for imp in all_dlls[dll]:
                imp_lower = imp.lower()
                # Check if this import exists in any form
                found = False
                for dll_name in all_dlls:
                    if dll_name.lower() == imp_lower:
                        found = True
                        break
                if not found:
                    # Check plugin dirs
                    for item in os.listdir(target_dir):
                        item_path = os.path.join(target_dir, item)
                        if os.path.isdir(item_path):
                            for f in os.listdir(item_path):
                                if f.lower() == imp_lower:
                                    found = True
                                    break
                        if found:
                            break
                if not found:
                    # Check if it's a system DLL
                    sys_dll_path = os.path.join('C:\\Windows\\System32', imp)
                    if not os.path.exists(sys_dll_path):
                        issues.append(f"[MISSING_DEP] {dll} -> {imp} (not found anywhere)")
    
    if issues:
        print(f"\n  Found {len(issues)} issues:")
        for issue in issues:
            print(f"    {issue}")
    else:
        print("  No issues found!")
    
    # Step 4: Check for DLLs that might have conflicting dependencies
    print("\n[Step 4] Checking for DLL version conflicts...")
    
    # Check if there are multiple versions of the same DLL
    dll_names = {}
    for dll in all_dlls:
        base_name = dll.lower()
        # Remove version suffixes for comparison
        import re
        base_name = re.sub(r'-\d+\.\d+', '', base_name)
        if base_name not in dll_names:
            dll_names[base_name] = []
        dll_names[base_name].append(dll)
    
    conflicts = {k: v for k, v in dll_names.items() if len(v) > 1}
    if conflicts:
        print(f"  Found {len(conflicts)} potential version conflicts:")
        for base, versions in sorted(conflicts.items()):
            print(f"    {base}: {versions}")
    else:
        print("  No version conflicts found")
    
    # Step 5: Check for DLLs that depend on MSVC runtime
    print("\n[Step 5] Checking MSVC runtime dependencies...")
    msvc_dlls = [d for d in dep_order if any(x in d.lower() for x in ['msvcp', 'vcruntime', 'concrt'])]
    if msvc_dlls:
        print(f"  MSVC runtime DLLs in dependency chain: {msvc_dlls}")
    
    # Step 6: Check for DLLs that might have TLS (Thread Local Storage) issues
    print("\n[Step 6] Checking for DLLs with potential TLS issues...")
    # TLS issues are common cause of 0xc0000142
    # Check for DLLs that import from each other in circular ways
    
    # Build reverse dependency map
    reverse_deps = {}
    for dll, imports in all_dlls.items():
        for imp in imports:
            imp_lower = imp.lower()
            if imp_lower not in reverse_deps:
                reverse_deps[imp_lower] = []
            reverse_deps[imp_lower].append(dll)
    
    # Check for circular dependencies
    print("  (Circular dependency detection requires graph analysis)")
    
    print("\n" + "=" * 80)
    print("SUMMARY")
    print("=" * 80)
    print(f"  Total DLLs in dependency chain: {len(dep_order)}")
    print(f"  Issues found: {len(issues)}")
    for issue in issues:
        print(f"    {issue}")

if __name__ == '__main__':
    target_dir = r'D:\project\OCCplusVTK\MyCAE\build-release\Release'
    find_dll_init_crash(target_dir, 'MyCAE.exe')
