# ccall
Library for dynamic calling C/C++ functions.

## Usage example
```cpp
#include "ccall/ccall.hpp"

#include <cstdint>
#include <iostream>

int32_t add_i32(int32_t a, int32_t b)
{
    return a + b;
}

int main()
{
    int32_t arg0 = 123;
    int32_t arg1 = 234;

    CCallArgsBuilder args_builder;
    CCallRet result{};

    args_builder.expect_result<int32_t>();
    args_builder.add_arg(arg0);
    args_builder.add_arg(arg1);

    ccall((void*)&add_i32, args_builder.get_args(), &result);

    std::cout << "result: " << result.get_result<int32_t>() << "\n";
}
```

## Note on structs
Some ABI (Linux x64) can pass structs where all fields are `float` or `double` in `xmm` registers.
Therefore you need to mark your structs with special tag `CCallArgTypeIsFloatStruct`
```cpp
#include "ccall/ccall.hpp"

struct Vector3
{
    float x, y, z;
};

template <>
constexpr bool CCallArgTypeIsFloatStruct<Vector3> = true;
```

## Dependencies
`nasm` is required to build this library on Linux.

```
sudo apt install nasm -y
```

## How to import

Go to your project directory
```
cd myproject
```

git clone
```
git clone https://github.com/kerrytazi/ccall.git lib/ccall
```

Add to you CMakeLists.txt
```
add_subdirectory(lib/ccall)
target_link_libraries(myproject ccall)
```

## Build test project
### Linux
```
cmake -DCCALL_TEST=ON -B out
cmake --build out
```
### Windows (Visual Studio)

Add `cmake` component via `Visual Studio Installer`.

'Open a local folder' in root.

## Currently supported platforms
- Windows x86_64
- Linux x86_64 (amd64)
