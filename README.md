# Stupid INI
## Scope
Stupid INI is an INI parser. The goal was to make a dumbed-down replacement for Simple INI for my personal use, since I don't actually use its advanced features. The main advantage over Simple INI is that Stupid INI does not pull in `windows.h` and that it has a simpler API.

## Usage
Quickstart:
```cpp

#include "StupidINI/StupidINI.hpp"

int main() {
    auto parseResult = StupidINI::load_ini("Path/To/INI/file.ini");
    if (!parseResult.has_value()) {
        std::cout << "Something went wrong: " << Stupid_INI::to_string(parseResult.error()) << "\n";
        return 1;
    }

    const auto ini = parseResult.value();

    const auto retrieved = ini.GetValue<size_t>("SomeSetting", "SomeSection");
    if (!retrieved.has_value()) {
        std::cout << "Failed to retrieve a setting because: " << Stupid_INI::to_string(retrieved.error()) << "\n";
        return 1;
    }

    const size_t val = retrieved.value();
}
```

In order to parse an INI file, first call `load_ini`, and pass in the path to the desired INI file. Note that the extension must end in `.ini` (not cast sensitive). If parsing is successful, you will get back an `INI` object, otherwise you will get a parse failure.

In order to retrieve a value from an `INI`, you will need to use the `GetValue` method. `GetValue` takes in two argumets - a string representing the setting name and a string representing the section the setting lives in. For convenience, you can omit the section argument to lookup settings outside of sections. You can also automatically cast the value you get from `GetValue` by supplying a type. The supported types are:

- String
- Integral
- Float
- `bool`

If the setting is missing, you will get back an `std::nullopt`. If the setting *cannot* be cast to the provided value, you will get back an error.

The API is non-throwing, but uses `std::expected` and `std::optional` to enforce compile-time checks where needed. You should not (and cannot) construct an INI object directly or edit its contents. 

## Credits
Template shamelessly stolen from [powerofthree](https://github.com/powerof3) (MIT).