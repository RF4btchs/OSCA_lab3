# ЛР 3. Потоки: наивный поиск образца в строке (вариант 18)

Сборка: `mingw32-make`. Запуск: `bin\lab3.exe -t 4 [-n 100000000] [-p abcabd] [-a 4] [-v]` (`-t` — максимум потоков).

- `show_threads.ps1` — число потоков процесса по данным ОС (Get-Process)
- `bench.ps1` → `results.csv`, `python plot.py` → `img\speedup.png`
- `trace\trace.ps1` → `trace\syscalls.txt`

## Process Monitor (на защите)

`..\lab1\tools\ProcessMonitor\Procmon64.exe` (от администратора).
Filter: `Process Name is lab3.exe` → Include, на панели оставить только Show Process and Thread Activity.

Запустить, например, `bin\lab3.exe -t 4 -v`. Что смотреть:
- `Thread Create` / `Thread Exit` — каждый CreateThread и завершение потока, TID совпадают с тем, что печатает `-v`;
- `Process Exit` — конец работы.

Число потоков в моменте: Process Explorer (`..\lab1\tools\ProcessExplorer`) → свойства процесса → вкладка Threads,
или `show_threads.ps1`.
