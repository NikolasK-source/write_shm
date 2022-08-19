# Shared Memory Writer

This tool writes data that it reads from stdin into a named shared memory.


## Use the Application
The tool must be started with the name of the shared memory to be written to as an argument (--name).
The shared memory with this name must already exist.

By default, the application reads as many bytes as available from stdin and writes them to shared memory.
If the input is smaller than the shared memory, the excess bytes of the shared memory are not written.
If the input is larger, it will be truncated to the size of the shared memory.

By setting the argument ```--repeat``` the input data, if smaller than the shared memory, is repeated until all bytes of the shared memory are written.

By setting the option ```--invert``` all bits of the input data are inverted.

By specifying the argument ```--passthrough``` all data written to the shared memory will be output to stdout.

### Examples
All the following examples use the shared memory with the name ```mem```.

#### Fill the shared memory with zeroes
```
write-shm -n mem < /dev/zero
```

#### Fill the shared memory with ones
```
write-shm -n mem -i < /dev/zero
```

#### Fill the shared memory with random values
```
write-shm -n mem -i < /dev/random
```

#### Write file to shared memory
```
write-shm -n mem < file
```

#### Repeat file in shared memory
```
write-shm -n mem -r < file
```

## Using the Flatpak package
The flatpak package can be installed via the .flatpak file.
This can be downloaded from the GitHub [projects release page](https://github.com/NikolasK-source/write_shm/releases):

```
flatpak install --user write-shm.flatpak
```

The application is then executed as follows:
```
flatpak run network.koesling.write-shm
```

To enable calling with ```write-shm``` [this script](https://gist.github.com/NikolasK-source/76a7160a9804140b65c0fdabd77d0a28) can be used.
In order to be executable everywhere, the path in which the script is placed must be in the ```PATH``` environment variable.


## Build from Source

The following packages are required for building the application:
- cmake
- clang or gcc

Use the following commands to build the application:
```
git clone --recursive https://github.com/NikolasK-source/write_shm.git
cd write_shm
mkdir build
cmake -B build . -DCMAKE_BUILD_TYPE=Release -DCLANG_FORMAT=OFF -DCOMPILER_WARNINGS=OFF
cmake --build build
```


## Links to related projects

### General Shared Memory Tools
- [Shared Memory Dump](https://nikolask-source.github.io/dump_shm/)
- [Shared Memory Write](https://nikolask-source.github.io/write_shm/)
- [Shared Memory Random](https://nikolask-source.github.io/shared_mem_random/)

### Modbus Clients
- [RTU](https://nikolask-source.github.io/modbus_rtu_client_shm/)
- [TCP](https://nikolask-source.github.io/modbus_tcp_client_shm/)

### Modbus Shared Memory Tools
- [STDIN to Modbus](https://nikolask-source.github.io/stdin_to_modbus_shm/)
- [Float converter](https://nikolask-source.github.io/modbus_conv_float/)


## License

MIT License

Copyright (c) 2021-2022 Nikolas Koesling

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
