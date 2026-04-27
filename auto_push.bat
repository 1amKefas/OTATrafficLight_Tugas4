@echo off
echo ==========================================
echo   AUTO UPDATE FIRMWARE ESP32 KE GITHUB
echo ==========================================
echo.

:: Pindah ke direktori tempat script ini berada
cd /d %~dp0

:: Copy file .bin dari dalam folder build ke folder root repo lu
:: (Pakai /Y supaya otomatis nge-replace file .bin lama tanpa nanya)
copy /Y "build\esp32.esp32.esp32\OTATrafficLight_Tugas4.ino.bin" "OTATrafficLight_Tugas4.ino.bin"

:: Tambahin semua file ke Git (folder build bakal dicuekin berkat .gitignore)
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