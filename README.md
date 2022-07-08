# Write SHM

Writes data from stdin to a named shared memory.

## Build
```
git submodule init
git submodule update
mkdir build
cd build
cmake .. -DCMAKE_CXX_COMPILER=$(which clang++) -DCMAKE_BUILD_TYPE=Release -DCLANG_FORMAT=OFF -COMPILER_WARNINGS=OFF
cmake --build .
```

As an alternative to the ```git submodule``` commands, the ```--recursive``` option can be used with ```git clone```.

## Use
```
write_shm [OPTION...]

  -n, --name arg     shared memory name (mandatory)
  -i, --invert       invert all input bits
  -r, --repeat       repeat input if input size is smaller than shared memory
  -p, --passthrough  output everything that is written to the shared memory to stdout
  -h, --help         print usage
```

## Libraries
This application uses the following libraries:
- cxxopts by jarro2783 (https://github.com/jarro2783/cxxopts)
