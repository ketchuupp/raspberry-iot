import os
import fnmatch
import sys

# --- Configuration ---

# Directory where the script is located (assumed to be project root)
PROJECT_ROOT = os.path.dirname(os.path.abspath(__file__))

# Files and directories to explicitly ignore (uses fnmatch patterns)
IGNORE_PATTERNS = [
    '.git',          # Git repository data
    '.DS_Store',     # macOS metadata
    'build',         # Common build directory name
    'build_*',       # Other build directories (build_rpi, build_macos)
    '_deps',         # FetchContent download/build directory
    'vendor',        # Often used for downloaded SDKs/libs
    '*.pyc',         # Python bytecode
    '*.pyo',
    '*~',            # Editor backups
    '*.swp',         # Vim swap files
    '*.log',         # Log files
    '*.o',           # Object files
    '*.a',           # Static libraries
    '*.so',          # Shared libraries (Linux)
    '*.dylib',       # Shared libraries (macOS)
    'myApp',         # Example executable name (adjust if needed)
    'Binaries',      # Output directory specified in CMake
    '.idea',         # JetBrains IDE files
    '.vscode',       # VS Code files
    '*.xcodeproj',   # Xcode files
    'CMakeCache.txt',
    'CMakeFiles',    # CMake intermediate files directory
    'CTestTestfile.cmake',
    'cmake_install.cmake',
    'Makefile',      # Generated Makefiles
    'CMakeScripts',
    'install_manifest.txt',
    '*.ninja*',      # Ninja build files
    '.build'         # Swift Package Manager Build dir
    # Add any other specific files or directories you want to ignore
    'project_dump.txt' # Don't include the output file itself
]

# File extensions to include
INCLUDE_EXTENSIONS = [
    '.cpp',
    '.h',
    '.hpp',
    '.c',           # Include C files if any
    'CMakeLists.txt', # Include CMakeLists files (exact match)
    '.cmake',       # Include other .cmake files (like toolchains)
    '.patch',       # Include patch files
    'Dockerfile',   # Include Dockerfile (exact match)
    '.gitignore',   # Include gitignore
    '.md',          # Include Markdown files (like README)
    # Add other relevant extensions if needed (e.g., .json, .txt for config)
]

# --- Helper Functions ---

def should_ignore(path, root):
    """Checks if a file or directory should be ignored based on IGNORE_PATTERNS."""
    relative_path = os.path.relpath(path, root)
    # Check against each part of the path for directory patterns
    parts = relative_path.split(os.sep)
    for pattern in IGNORE_PATTERNS:
        # Check full path or any directory component
        if fnmatch.fnmatch(relative_path, pattern) or \
           any(fnmatch.fnmatch(part, pattern) for part in parts):
            # print(f"Ignoring '{relative_path}' due to pattern '{pattern}'") # Debugging
            return True
    return False

def should_include(filename):
    """Checks if a file should be included based on INCLUDE_EXTENSIONS."""
    if filename in INCLUDE_EXTENSIONS: # Handle exact matches like CMakeLists.txt
        return True
    for ext in INCLUDE_EXTENSIONS:
        if ext.startswith('.') and filename.endswith(ext):
            return True
    return False

# --- Main Logic ---

output_lines = []

print(f"Scanning project root: {PROJECT_ROOT}", file=sys.stderr)
print(f"Ignoring patterns: {IGNORE_PATTERNS}", file=sys.stderr)
print(f"Including extensions/files: {INCLUDE_EXTENSIONS}", file=sys.stderr)
print("-" * 30, file=sys.stderr)

for root, dirs, files in os.walk(PROJECT_ROOT, topdown=True):
    # Filter ignored directories in-place to prevent walking into them
    dirs[:] = [d for d in dirs if not should_ignore(os.path.join(root, d), PROJECT_ROOT)]

    for filename in files:
        # Check if the file itself should be ignored or included
        filepath = os.path.join(root, filename)
        relative_filepath = os.path.relpath(filepath, PROJECT_ROOT)

        if not should_ignore(filepath, PROJECT_ROOT) and should_include(filename):
            print(f"Processing: {relative_filepath}", file=sys.stderr) # Progress indicator
            output_lines.append(f"--- START FILE: {relative_filepath} ---")
            try:
                with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
                    output_lines.append(f.read())
            except Exception as e:
                output_lines.append(f"!!! ERROR READING FILE: {e} !!!")
            output_lines.append(f"--- END FILE: {relative_filepath} ---")
            output_lines.append("") # Add a blank line for readability

# Print the combined output
print("\n".join(output_lines))

print("-" * 30, file=sys.stderr)
print("Finished scanning.", file=sys.stderr)