@echo off
echo ==========================================
echo   AUTO UPDATE FIRMWARE ESP32 KE GITHUB
echo ==========================================
echo.

:: Pindah ke direktori tempat script ini berada (folder repo lu)
cd /d %~dp0

:: Tambahin semua file yang berubah (termasuk .bin lu yang baru)
git add .

:: Bikin commit otomatis pakai timestamp waktu hari ini
git commit -m "Auto-update firmware bin: %date% %time%"

:: Push langsung ke branch main
git push origin main

echo.
echo ==========================================
echo   UPDATE SUKSES! FIRMWARE UDAH DI CLOUD!
echo ==========================================
pause