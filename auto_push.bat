@echo off
echo ==========================================
echo   AUTO UPDATE FIRMWARE ESP32 KE GITHUB
echo ==========================================
echo.

cd /d %~dp0
copy /Y "build\esp32.esp32.esp32\OTATrafficLight_Tugas4.ino.bin" "OTATrafficLight_Tugas4.ino.bin"

:: BIKIN FILE VERSION.TXT OTOMATIS
echo %date% %time% > version.txt

git add .
git commit -m "Auto-update firmware: %date% %time%"
git push origin main

echo.
echo ==========================================
echo   UPDATE SUKSES! FIRMWARE UDAH DI CLOUD!
echo ==========================================
pause