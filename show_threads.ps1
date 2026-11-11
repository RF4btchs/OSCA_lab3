# Показывает, сколько потоков у lab3.exe во время работы (стандартное средство ОС - Get-Process).
# Пример: powershell -File show_threads.ps1 -T 8
param([int]$T = 8, [int]$N = 400000000, [int]$Repeats = 15)

$exe = Join-Path $PSScriptRoot 'bin\lab3.exe'
$p = Start-Process $exe -ArgumentList "-t $T -n $N -r $Repeats" -NoNewWindow -PassThru
$max = 0
while (-not $p.HasExited) {
    $proc = Get-Process -Id $p.Id -ErrorAction SilentlyContinue
    if ($proc) {
        $c = $proc.Threads.Count
        if ($c -gt $max) { $max = $c }
        '{0:HH:mm:ss.fff}  lab3.exe PID={1}  threads={2}' -f (Get-Date), $p.Id, $c
    }
    Start-Sleep -Milliseconds 300
}
"max threads seen: $max (limit -t $T)"
