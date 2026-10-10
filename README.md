# Stupid INI
Stupid INI is an INI parser. The goal was to make a dumbed-down replacement for Simple INI for my personal use, since I don't actually use its advanced features. The main advantage over Simple INI is that Stupid INI does not pull in `windows.h` and that it has a simpler API.

## Quickstart
```cpp

#include "StupidINI/StupidINI.hpp"

int main() {
    auto parseResult = StupidINI::load_ini("Path/To/INI/file.ini");
    if (!parseResult.has_value()) {
        std::cout << "Failed to parse existing INI with error: " << parseResult.error() << "\n";
        return 1;
    }

    const auto ini = parseResult.value();
    const auto retrieved = ini.GetValue<size_t>("SomeSetting", "SomeSection");
    if (!retrieved.has_value()) {
        std::cout << "Failed to retrieve a setting with error: " << retrieved.error() << "\n";
        return 1;
    }

    const size_t val = retrieved.value();
}
```

## INI Standard
There is not, to my knowledge, an INI standard. There are several parsers, and every single one of them disagrees. Rather than embodying the xkdc meme, StupidINI is stupidly opinionated. Behold; The stupid standard.

```ini
[Section]
Setting = Value ; Comment
```
- Sections\
Sections are any text enclosed in square brackets (`[]`). You do not need to have a section be a single word, whitespaces are allowed. However, there must NOT be any non-whitespace character before the opening bracket (`[`), and the section MUST be closed at the end of the line with a closing bracket (`]`). If a section is opened, but not properly closed, it is thrown out and **any and all keys until a new valid section is found are ignored**. By default, sections are NOT case sensitive. You can redefine sections at any point - new settings are merged into the previous definition. Examples of valid sections:
```ini
[Section]
       [AlsoASection]
[Yes, this is also a section. Yes, you can use special characters!]
[Section]
[]
```
- Settings\
Settings have 5 characteristics: A `section`, a `key`, a `value`, the `assignment operator`, and a `comment`.
  * `Commend`\
A `comment` of a setting is any comment after a setting's value, or **any number** of comments directly above the setting. Comments are completely optional.
  * `Section`\
The `section` of a setting is the first section found above it. If there isn't a defined section, then the setting is considered to be in the "global" or "empty" section. `Settings` in a `section` are read **top-to-bottom**, and if a setting is redefined, the bottom setting is considered the source of truth.
  * `Key`\
A `key` is the term used for a setting's name. Note that **by default, keys are not case sensitive**. A key is every character before the assignment operator, except the leading whitespaces. Unlike most INI parsers, StupidINI permits keys with whitespaces between characters, so `Max Width` is a valid key. Note that whitespaces between the last non-whitespace character of a string and the `assignment operator` are also trimmed. A key is a mandatory part of a setting.
  * `Value`\
A value is every character after the `assignment operator`, except leading and trailing whitespaces. A value is a mandatory part of a setting.
  * `Assignment Operator`\
The `assignment operator` is the equals character (`=`), and determines where a key ends and where a value starts. Note that StupidINI trims all whitespaces before and after the `assignment operator` until it hits a character, so `key = value` is equivalent to `key=value`.
```ini
GlobalSetting = 100 ; This is a setting in the global/empty section.

[Screen Properties]
; This is a setting in the "Screen Properties" section.
; Both this comment and the one before it are associated with this setting.
; I should test if emojis are fine. 🍌
iWidth = 1920

; This will (by default) override iWidth in this section.
; The whitespaces from the start of the line, between the key and the
; assignment operator, and between the assignment operator and the value,
; and after the value are discarded.
    IWIDTH    =          3840

[]
; If you ever need to add more settings to the Global/Empty section,
; you can do it like this.
GlobalBoolean = true
```
- `Comments`\
Comments are anything and everything following a semi-colon (`;`). Comments can be freely appended after anything, but only have meaning before settings or in the same line as a setting. If a comment exists before a setting or in the same line, it is discarded.

## Compiler Definitions
You can add some compiler definitions to modify how StupidINI works. All of these are off by default, and not recommended.

| Name | Effect |
| :--- | ---: |
| STUPIDINI_CASE_SENSITIVE | Keys will no longer be case-insensitive. By default, `iWidth` is considered the same as `iwidth`, but using `STUPIDINI_CASE_SENSITIVE` will allow them to be treated as different keys. Slightly speeds up lookups and parsing. |
| STUPIDINI_CACHE | Searched values will be cached for later use. If, for some reason, you are constantly looking for the same values in the same INI, then this will speed up future lookups. Be aware that these values will be frozen the moment they are cached, and changing them will not update them. You will need to save the ini and then re-load it to discard the cache. |

Usage example:
```cpp
#define STUPIDINI_CASE_SENSITIVE
#include "StupidINI/StupidINI.hpp"

void foo(const char* iniPath) {
    const auto parsed = StupidINI::load_ini(iniPath);
    if (!parsed) {
        return;
    }

    const auto ini = parsed.value();

    // iwidth =/= iWidth
    const auto width1 = ini.GetSetting<int>("iwidth").value_or(100);
    const auto width2 = ini.GetSetting<int>("iWidth").value_or(150);
}
```

## Credits
Template shamelessly stolen from [powerofthree](https://github.com/powerof3) (MIT).