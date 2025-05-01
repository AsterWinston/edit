compile:
	@echo off
	g++ -o .\bin\edit.exe .\src\edit.cpp .\src\function.cpp .\src\myclass.cpp -I.\include -static

	