mingw_compile:
	g++ -o .\bin\edit.exe .\src\edit.cpp .\src\function.cpp .\src\myclass.cpp -I.\include -static
clean:
	@del /q bin\*
	@del /q build\*
	@for /d %%i in ("bin\*") do (del /q "%%i\*" & rmdir /s /q "%%i")
	@for /d %%i in ("build\*") do (del /q "%%i\*" & rmdir /s /q "%%i")
	@del /q obj\*
msvc_compile:
	if not exist obj mkdir obj
	cl /c /MD /O2 /EHsc /std:c++17 /I"include" /Fo"obj\edit.obj" src\edit.cpp
	cl /c /MD /O2 /EHsc /std:c++17 /I"include" /Fo"obj\function.obj" src\function.cpp
	cl /c /MD /O2 /EHsc /std:c++17 /I"include" /Fo"obj\myclass.obj" src\myclass.cpp
	link obj\edit.obj obj\function.obj obj\myclass.obj /OUT:"bin\edit.exe"




