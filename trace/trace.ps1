# Трассировка ключевых системных вызовов программы отладчиком cdb (WinDbg).
# Отлаживаются и родитель, и все дочерние процессы (ключ -o); точки останова на
# функциях ntdll из bps.wds ставятся в каждом новом процессе (событие cpr).
# Результат: trace\syscalls.txt (процесс, PID, TID, вызов, первые аргументы)
param([string]$Command = 'bin\lab3.exe -t 4 -n 200000 -v')

$ErrorActionPreference = 'Stop'
$Lab  = Split-Path -Parent $PSScriptRoot
$Work = Join-Path $env:TEMP ("trace_" + (Split-Path -Leaf $Lab))
New-Item -ItemType Directory -Force $Work | Out-Null
$dir = $Work -replace '\\', '/'
Get-ChildItem $PSScriptRoot -Filter *.wds | ForEach-Object {
    (Get-Content $_.FullName -Raw) -replace '@@DIR@@', $dir | Set-Content -Encoding ascii (Join-Path $Work $_.Name)
}

$pkg = Get-AppxPackage Microsoft.WinDbg -ErrorAction SilentlyContinue
if (-not $pkg) { throw 'WinDbg не установлен: winget install Microsoft.WinDbg' }
$cdb = Join-Path $pkg.InstallLocation 'amd64\cdb.exe'
$env:_NT_SYMBOL_PATH = "srv*$env:TEMP\symcache*https://msdl.microsoft.com/download/symbols"

$log = Join-Path $Work 'cdb.log'
$out = Join-Path $Work 'console.txt'
$err = Join-Path $Work 'stderr.txt'
Remove-Item $log, $out, $err -ErrorAction SilentlyContinue
# cmd.exe нужен только для перенаправления stdin из файла; точки останова ставятся
# лишь в процессах, которые он запускает (событие cpr), поэтому сам cmd в трассу не попадает
$p = Start-Process $cdb -WorkingDirectory $Lab -NoNewWindow -Wait -PassThru `
        -RedirectStandardOutput $out -RedirectStandardError $err `
        -ArgumentList @('-o', '-G', '-logou', $log, '-c', "`"`$`$<$dir/main.wds`"",
                        "$env:SystemRoot\System32\cmd.exe", '/c', $Command)
if (-not (Test-Path $log)) { throw "cdb не создал лог (код $($p.ExitCode))" }

# Имена процессов берём из вывода программы: "[parent PID=..]", "[child  PID=..]", "PID=.. main"
$console = (Get-Content $out, $err -ErrorAction SilentlyContinue) -join "`n"
$names = @{}
foreach ($m in [regex]::Matches($console, '\[(parent|child)\s+PID=(\d+)')) { $names[$m.Groups[2].Value] = $m.Groups[1].Value }
foreach ($m in [regex]::Matches($console, '(?m)^PID=(\d+) main')) { $names[$m.Groups[1].Value] = 'lab3' }

$text = [IO.File]::ReadAllText($log, [Text.Encoding]::Unicode)
$calls = [regex]::Matches($text, '(?m)^SC\|([0-9a-f]+)\|([0-9a-f]+)\|ntdll!(\w+)[^|]*\|([0-9a-f`]+)\|([0-9a-f`]+)\|([0-9a-f`]+)')
$lines = foreach ($m in $calls) {
    $procId = [string][Convert]::ToInt32($m.Groups[1].Value, 16)
    $name = if ($names.ContainsKey($procId)) { $names[$procId] } else { '?' }
    '{0,-7}{1,6} {2,6}  {3,-28} rcx={4} rdx={5}' -f $name, $procId,
        [Convert]::ToInt32($m.Groups[2].Value, 16), $m.Groups[3].Value,
        ($m.Groups[4].Value -replace '`', ''), ($m.Groups[5].Value -replace '`', '')
}
$hdr = "process   PID    TID  syscall                      first arguments"
@($hdr) + $lines | Set-Content -Encoding utf8 (Join-Path $PSScriptRoot 'syscalls.txt')
Write-Host "syscalls: $($lines.Count) -> trace\syscalls.txt"
