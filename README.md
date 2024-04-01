# Write SHM

Writes data from stdin to a named shared memory.

## Dependencies
- cxxopts by jarro2783 (https://github.com/jarro2783/cxxopts) (only required for building the application)
- cxxshm (https://github.com/NikolasK-source/cxxshm)
- cxxsemaphore (https://github.com/NikolasK-source/cxxsemaphore)

On Arch linux they are available via the official repositories and the AUR:
- https://archlinux.org/packages/extra/any/cxxopts/
- https://aur.archlinux.org/packages/cxxshm
- https://aur.archlinux.org/packages/cxxsemaphore

## Build
```
cmake -B build . -DCMAKE_CXX_COMPILER=$(which clang++) -DCMAKE_BUILD_TYPE=Release -DCLANG_FORMAT=OFF -DCOMPILER_WARNINGS=OFF
cmake --build build 
```

As an alternative to the ```git submodule``` commands, the ```--recursive``` option can be used with ```git clone```.

## Use
```
write-shm [OPTION...]

 shared memory options:
  -n, --name arg       shared memory name (mandatory)
  -s, --semaphore arg  protect the shared memory with an existing named semaphore against simultaneous access

 settings options:
  -i, --invert       invert all input bits
  -r, --repeat       repeat input if input size is smaller than shared memory
  -p, --passthrough  output everything that is written to the shared memory to stdout

 other options:
  -h, --help     print usage
      --license  show licences

 version information options:
      --version       print version and exit
      --longversion   print version (including compiler and system info) and exit
      --shortversion  print version (only version string) and exit
      --git-hash      print git hash
```
