apathy/redist
=============

- This optional branch is used to regenerate the amalgamated distribution. Do not include it into your project.
- Setup
```
git checkout redist
git submodule init
git submodule update
```
- Regenerate the distribution by typing the following lines:
```
move /y apathy.hpp ..
deps\Amalgamate.exe -p apathy.hpp -w "*.*pp;*.c;*.h" apathy.cpp ..\apathy.cpp
deps\Amalgamate.exe -p apathy.hpp -w "*.*pp;*.c*;*.h" tests.cxx ..\tests.cxx
deps\fart.exe -- ..\apathy.cpp "#line" "//#line"
deps\fart.exe -- ..\apathy.cpp "#pragma once" "//#pragma once"
deps\fart.exe -- ..\tests.cxx  "#line" "//#line"
deps\fart.exe -- ..\tests.cxx  "#pragma once" "//#pragma once"
copy /y ..\apathy.hpp .
```
