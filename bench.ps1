# Замеры времени поиска для разных входных данных и числа потоков -> results.csv
$exe = Join-Path $PSScriptRoot 'bin\lab3.exe'
if (-not (Test-Path $exe)) { throw 'сначала соберите программу: mingw32-make' }

# случайный текст из 4 букв, образец abcabd; и худший случай для наивного алгоритма
$cases = @(
    @{ name = 'random';  n = 1000000;   args = @() },
    @{ name = 'random';  n = 10000000;  args = @() },
    @{ name = 'random';  n = 100000000; args = @() },
    @{ name = 'worst';   n = 100000000; args = @('-a', '1', '-p', 'aaaaaaaaaaaaaaab') }
)
$threads = 1, 2, 3, 4, 6, 8, 12, 16, 32
$rows = foreach ($c in $cases) {
    foreach ($t in $threads) {
        $out = & $exe -t $t -n $c.n -r 5 @($c.args)
        if ($LASTEXITCODE -ne 0) { throw "lab3.exe -t $t -n $($c.n) завершилась с кодом $LASTEXITCODE" }
        $ms = [double]([regex]::Match(($out -join ' '), 'time=([\d.]+)').Groups[1].Value)
        $found = [regex]::Match(($out -join ' '), 'found (\d+)').Groups[1].Value
        Write-Host ("{0,-7} n={1,-10} t={2,-3} {3,9:N2} ms  found={4}" -f $c.name, $c.n, $t, $ms, $found)
        [pscustomobject]@{ case = $c.name; n = $c.n; threads = $t; ms = $ms; found = $found }
    }
}
$rows | Export-Csv (Join-Path $PSScriptRoot 'results.csv') -NoTypeInformation -Encoding utf8
