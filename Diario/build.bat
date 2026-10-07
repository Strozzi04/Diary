@echo off
rem Compila le due versioni del diario con MinGW (g++), ad esempio quello di Dev-C++ o MSYS2.
rem Uso: fai doppio clic su questo file oppure eseguilo dal prompt dei comandi.

echo Compilo la versione a console (Diario.exe)...
g++ -std=c++17 -O2 -s -static Diario.cpp diario_core.cpp -o Diario.exe
if errorlevel 1 goto errore

echo Compilo la versione grafica (DiarioGUI.exe)...
windres diario.rc -O coff -o diario_res.o
if errorlevel 1 goto errore
g++ -std=c++17 -O2 -s -static -municode -mwindows DiarioGUI.cpp diario_core.cpp diario_res.o -o DiarioGUI.exe -lcomctl32
if errorlevel 1 goto errore
del diario_res.o

echo Fatto!
pause
exit /b 0

:errore
echo Errore durante la compilazione.
pause
exit /b 1
