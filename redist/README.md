apathy/redist
=============

- This optional folder is used to regenerate the amalgamated distribution. Do not include it into your project.
- Regenerate the distribution by typing the following lines:
```
copy apathy.hpp .. /y
deps\Amalgamate.exe -p apathy.hpp -w "*.*pp;*.c;*.h" apathy.cpp ..\apathy.cpp
deps\Amalgamate.exe -p apathy.hpp -w "*.*pp;*.c*;*.h" tests.cxx ..\tests.cxx
deps\fart.exe -- ..\apathy.cpp "#line" "//#line"
deps\fart.exe -- ..\tests.cxx "#line" "//#line"
```
