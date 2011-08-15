#NoTrayIcon
$file=FileOpen("server.pid",0)
$PID=FileRead($file)
FileClose($file)
FileDelete("server.pid")
ProcessClose($PID)