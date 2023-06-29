@echo off
echo NOTICE: Script MUST be run as Administrator.
:: Errors from "reg" tool are muted to avoid flooding the build log with errors from already deleted registry entries.

:: Script for cleaning up registry entries.
:: Useful for local development and after an improper uninstallation.

:: Delete IServer interface
for %%P in (32 64) do (
  reg delete "HKCR\Interface\{F586D6F4-AF37-441E-80A6-3D33D977882D}" /f /reg:%%P 2> NUL
)

:: Delete ServerLib TypeLib
reg delete "HKCR\TypeLib\{46F3FEB2-121D-4830-AA22-0CDA9EA90DC3}" /f 2> NUL

:: Delete Server COM class
reg delete "HKCR\CLSID\{AF080472-F173-4D9D-8BE7-435776617347}" /f 2> NUL


pause
