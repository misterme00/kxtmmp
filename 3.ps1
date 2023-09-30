# Get the current time and add 1 minute
$currentTime = Get-Date
$startTime = $currentTime.AddMinutes(1)

# Define the task action (run a PowerShell script)
$action = New-ScheduledTaskAction -Execute 'powershell.exe' -Argument '-ExecutionPolicy Bypass -File "C:\Users\Public\sec\2.ps1"'

# Define the trigger (one-time, starting at the calculated start time, with a 3-minute repetition interval)
$trigger = New-ScheduledTaskTrigger -Once -At $startTime -RepetitionInterval ([TimeSpan]::FromMinutes(3))

# Create the scheduled task
Register-ScheduledTask -TaskName "MonitorExeTask" -Action $action -Trigger $trigger
