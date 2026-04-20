Password Manager Prototype - Team Test Build

What is included
- Password Manager.exe
- data\ folder
- required runtime DLLs
- Launch-Local.cmd
- Launch-MySQL.cmd

Recommended option for most teammates
- Double-click Launch-Local.cmd
- This runs the prototype without requiring MySQL
- Data will not be shared between teammates in this mode

If you want to test the MySQL-backed version
1. Install MySQL Server 8.0
2. Create or import the project database
3. Edit Launch-MySQL.cmd and set the correct database password
4. Double-click Launch-MySQL.cmd

Notes
- Keep all files together in the same folder
- Do not move Password Manager.exe out of this folder
- The app needs the data\ folder beside the .exe to start properly
- If Windows warns about an unknown app, choose More info, then Run anyway

Current MySQL defaults in Launch-MySQL.cmd
- Host: 127.0.0.1
- Port: 3306
- Database: passwordManager
- User: root
- Password: CHANGE_ME
