# ДЗ 5. HTTP-клиент и сервер на WinSock2

Клиент отправляет `GET /` на `example.com:80` и печатает HTTP-ответ вместе с заголовками. Сервер слушает `127.0.0.1:80`, отдаёт файлы из указанной при запуске папки и обслуживает до четырёх клиентов одновременно.

Сервер читает файлы в бинарном режиме, передаёт длину тела в `Content-Length` и возвращает `200 OK` или `404 Not Found`. Перед открытием файла он нормализует путь и проверяет, что путь находится внутри корневой папки сайта.

## Файлы проекта

| Файл | Что в нём |
| --- | --- |
| [`http_client.cpp`](http_client.cpp) | Поиск адреса, подключение, отправка HTTP-запроса и вывод ответа. |
| [`http_server.cpp`](http_server.cpp) | Приём подключений, разбор `GET`, чтение файла и отправка ответа. |
| [`site_path.h`](site_path.h) | Проверка принадлежности нормализованного пути корневой папке. |
| [`www/index.html`](www/index.html) и [`www/retro.html`](www/retro.html) | Простая и ретро-страницы для проверки в браузере. |

## Запуск в CLion

1. Открой папку `hw5` как отдельный проект CMake и выбери профиль `Debug` с MinGW.
2. Для `http_server` открой **Run → Edit Configurations…**. Укажи `www` в **Program arguments**, а папку `hw5` — в **Working directory**.
3. Запусти `http_server`. Когда появится `listening on: 127.0.0.1:80`, открой [простую страницу](http://127.0.0.1:80/index.html) или [ретро-страницу](http://127.0.0.1:80/retro.html).
4. Для проверки клиента выбери `http_client` и запусти его без аргументов. Он сам обращается к `example.com`. В консоли появятся строка статуса, заголовки и тело ответа.

## Тесты корректности

- [`check_paths`](tests/check_paths.cpp) — отдельная цель CMake. Проверяет, что путь внутри `www` разрешён, а путь в соседней папке `www_other` отклоняется. При успехе выводит `Root boundary checks (OK)`.
- [`check_http.ps1`](tests/check_http.ps1) — проверяет ответы `200 → 404 → 200` за один запуск сервера, совпадение байтов HTML и бинарного файла и работу двух клиентов одновременно. Для теста отсутствующего файла в `www` оставляют свободным имя `missing.html`.

Для HTTP-тестов оставь `http_server` запущенным с корнем `www`. В отдельном окне PowerShell из папки `hw5` выполни:

```powershell
.\tests\check_http.ps1
```

### Debug с AddressSanitizer

Проверенная конфигурация для Windows использует CMake 4.3 и Visual Studio Build Tools 2026 с поддержкой AddressSanitizer. Выполни команды из папки `hw5` в Developer PowerShell for Visual Studio, где доступны `cmake` и библиотеки MSVC.

```powershell
cmake -S . -B build-asan -G 'Visual Studio 18 2026' -A x64 `
    '-DCMAKE_CXX_STANDARD=17' `
    '-DCMAKE_CXX_FLAGS=/DWIN32 /D_WINDOWS /W3 /GR /EHsc /DNOMINMAX' `
    '-DCMAKE_CXX_FLAGS_DEBUG=/Zi /Od /Ob0 /D_DEBUG /fsanitize=address' `
    '-DCMAKE_EXE_LINKER_FLAGS_DEBUG=/DEBUG /INCREMENTAL:NO'
cmake --build build-asan --config Debug
.\build-asan\Debug\check_paths.exe
.\build-asan\Debug\http_server.exe .\www
```

После запуска сервера выполни `check_http.ps1` во втором окне PowerShell. Для ручной проверки клиентской сборки запусти `.\build-asan\Debug\http_client.exe`.
