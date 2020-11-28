# Rest Server for C++

This is an easy to use Rest Server based on [Boost](https://www.boost.org/) and
[Nlohmann JSON](https://github.com/nlohmann/json).  

It is designed to exchange JSON data.  

## Usage

```cpp
auto restServer = std::make_shared<RestServer>("127.0.0.1", 8080);

restServer->registerEndpoint("/", [](auto session, const auto& request) {
    nlohmann::json message;
    message["message"] = "Test Response";

    session->sendResponse(message);
});

restServer->startListening();
```

## Compiling on Windows 10
For compiling on Windows 10 you have to install [Visual Studio 2019](https://visualstudio.microsoft.com) and [CMake](https://cmake.org/).  
You also have to install [Boost](https://www.boost.org/) 1.74.0 or later and [Nlohmann JSON](https://github.com/nlohmann/json).  
Install the dependencies under `C:\include` and `C:\lib` or pass the `Boost_ROOT` and `NLOHMANN_JSON_ROOT` CMake options.  
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
You also have to install [Boost](https://www.boost.org/) 1.74.0 or later and [Nlohmann JSON](https://github.com/nlohmann/json).  
Install the dependencies under `/usr/local/include` and `/usr/local/lib` or pass the `Boost_ROOT` and `NLOHMANN_JSON_ROOT` CMake options.  
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
You also have to install [Boost](https://www.boost.org/) 1.74.0 or later and [Nlohmann JSON](https://github.com/nlohmann/json).  
Install the dependencies under `/usr/local/include` and `/usr/local/lib` or pass the `Boost_ROOT` and `NLOHMANN_JSON_ROOT` CMake options.  
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
[Nlohmann JSON](https://github.com/nlohmann/json) is licenced under the [The MIT License (MIT)](https://github.com/nlohmann/json/blob/develop/LICENSE.MIT).  
