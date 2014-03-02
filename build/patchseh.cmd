:: patchseh.cmd
:: 10/2/2013 jichi
@echo off
setlocal

for %%i in (vnr*xp.dll) do (
  echo sehpatcher %%i
  sehpatcher %%i
)

:: EOF
