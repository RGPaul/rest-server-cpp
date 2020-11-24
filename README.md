# Rest Server for C++

This is an easy to use Rest Server based on [Boost](https://www.boost.org/), 
[Nlohmann JSON](https://github.com/nlohmann/json) and [OpenSSL](https://www.openssl.org/).  

## Compiling on Windows 10
For compiling on Windows 10 you have to install [Visual Studio 2019](https://visualstudio.microsoft.com) and [CMake](https://cmake.org/).  
You also have to install [Boost](https://www.boost.org/) 1.73.0 or later, [Nlohmann JSON](https://github.com/nlohmann/json) and [OpenSSL](https://www.openssl.org/).  
Install the dependencies under `C:\include` and `C:\lib` or pass the `Boost_ROOT`, `OPENSSL_ROOT_DIR` and `NLOHMANN_JSON_ROOT` CMake options.  
Alternatively you can use [Conan](https://conan.io/) to install all dependencies.  

Install Dependencies via Conan:  
```
conan install -s os=Windows -s compiler="Visual Studio" -s compiler.runtime=MT -s arch=x86_64 -s build_type=Release -if "build" .
```

Compile using CMake:  
```
cmake -G "Visual Studio 16 2019" -A x64 -S . -B "build"
cmake --build build --config Release
cmake --install build --config Release
```

## Compiling on macOS
For compiling on macOS you have to install [Xcode](https://developer.apple.com/xcode/) and [CMake](https://cmake.org/).  
You also have to install [Boost](https://www.boost.org/) 1.73.0 or later, [Nlohmann JSON](https://github.com/nlohmann/json) and [OpenSSL](https://www.openssl.org/).  
Install the dependencies under `/usr/local/include` and `/usr/local/lib` or pass the `Boost_ROOT`, `OPENSSL_ROOT_DIR` and `NLOHMANN_JSON_ROOT` CMake options.  
Alternatively you can use [Conan](https://conan.io/) to install all dependencies.  

Install Dependencies via Conan:  
```
conan install -s os=Macos -s arch=x86_64 -s build_type=Release -if "build" .
```

Compile using CMake:  
```
cmake -S . -B build
cmake --build build --config Release
cmake --install build --config Release
```


## Compiling on Linux Debian
For compiling on Linux you have to install `build-essential` and [CMake](https://cmake.org/).  
You also have to install [Boost](https://www.boost.org/) 1.73.0 or later, [Nlohmann JSON](https://github.com/nlohmann/json) and [OpenSSL](https://www.openssl.org/).  
Install the dependencies under `/usr/local/include` and `/usr/local/lib` or pass the `Boost_ROOT`, `OPENSSL_ROOT_DIR` and `NLOHMANN_JSON_ROOT` CMake options.  
Alternatively you can use [Conan](https://conan.io/) to install all dependencies.  

Install Dependencies via Conan:  
```
conan install -s os=Linux -s arch=x86_64 -s build_type=Release -if "build" .
```

Compile using CMake:  
```
cmake -S . -B build
cmake --build build --config Release
cmake --install build --config Release
```

## License
Rest Server C++ is licenced under the [The MIT License (MIT)](LICENSE).  
[Boost](https://www.boost.org/) is licensed under the [Boost Software License](https://www.boost.org/users/license.html).  
[Nlohmann JSON](https://github.com/nlohmann/json)is licenced under the [The MIT License (MIT)](https://github.com/nlohmann/json/blob/develop/LICENSE.MIT).  
[OpenSSL](https://www.openssl.org/) is licensed under the [OpenSSL License](https://www.openssl.org/source/license-openssl-ssleay.txt).  

