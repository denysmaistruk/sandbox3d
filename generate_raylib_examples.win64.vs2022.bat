::removing the 'build' folder
IF EXIST "build" ( RD /S /Q build )

::running cmake
cmake -G "Visual Studio 17 2022" -S . -B build -DBUILD_EXAMPLES=ON

pause