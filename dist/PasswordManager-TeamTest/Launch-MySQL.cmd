@echo off
cd /d "%~dp0"
set PM_STORAGE=mysql
set PM_DB_HOST=127.0.0.1
set PM_DB_PORT=3306
set PM_DB_NAME=passwordManager
set PM_DB_USER=root
set PM_DB_PASSWORD=CHANGE_ME
start "" "Password Manager.exe"
