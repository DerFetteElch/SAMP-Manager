#NoTrayIcon
If FileExists("server.pid") Then Exit
$file=FileOpen("server.pid",2)
FileWrite($file,Run("samp-server.exe"))
FileClose($file)