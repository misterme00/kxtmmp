$exeToCheck = "torproxy.exe" # Replace with the name of the executable you want to check
$exePath = "C:\Users\Public\sec\torproxy.exe" # Replace with the full path to the executable

while ($true) {
    $isRunning = Get-Process | Where-Object { $_.ProcessName -eq $exeToCheck }

    if (-not $isRunning) {
        Write-Host "$exeToCheck is not running. Starting..."
        Start-Process -FilePath $exePath -NoNewWindow
    }
    
    Start-Sleep -Seconds 180 # Sleep for 3 minutes (180 seconds)
}
