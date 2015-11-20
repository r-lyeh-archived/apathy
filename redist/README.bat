apathy/redist
=============

- This optional folder is used to regenerate the amalgamated distribution. Do not include it into your project.
- Regenerate the distribution by typing the following lines:
```
    deps\amalgamate -i deps\ -w "*.c*;*.h*" apathy.hpp ..\apathy.hpp
    deps\fart.exe -- ..\apathy.hpp "#line" "//#line"
rem deps\fart.exe -- ..\apathy.hpp "#pragma once" "//#pragma once"
```
