# GLAD Core Profile Overlay

This vcpkg overlay provides GLAD with OpenGL 4.6 **Core** profile instead of the default compatibility profile.

## Setup Instructions

You need to generate GLAD files with core profile:

### Option 1: Use GLAD Web Service (Easiest)

1. Visit https://glad.dav1d.de/
2. Configure:
   - **Language**: C/C++
   - **Specification**: OpenGL
   - **API**: gl - Version 4.6
   - **Profile**: Core
   - **Generator**: C/C++
3. (Optional) Add extensions:
   - Click "Add all" if you want all extensions
   - Or select specific extensions you need
4. Click "GENERATE"
5. Download the zip file
6. Extract the `include/` and `src/` folders to: `vcpkg-overlays/glad/glad-core/`

The directory structure should be:
```
vcpkg-overlays/glad/
├── portfile.cmake
├── vcpkg.json
├── README.md (this file)
└── glad-core/
    ├── include/
    │   ├── glad/
    │   │   └── glad.h
    │   └── KHR/
    │       └── khrplatform.h
    └── src/
        └── glad.c
```

### Option 2: Use GLAD Python Generator

If you have Python installed:

```bash
pip install glad
cd vcpkg-overlays/glad/
python -m glad --api="gl:core=4.6" --out-path=glad-core --extensions
```

## Verification

After generating the files, check that `glad-core/include/glad/glad.h` contains:
```c
Profile: core
```

NOT:
```c
Profile: compatibility
```

## Rebuild

After placing the files, rebuild vcpkg packages:
```bash
# Clean vcpkg cache
rm -rf cmake-build-debug/vcpkg_installed

# Reconfigure CMake
cmake -B cmake-build-debug -S .
```
