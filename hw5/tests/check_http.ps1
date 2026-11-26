function Assert-Status([string]$path, [string]$expected) {
    $status = curl.exe --noproxy '*' --silent --show-error --max-time 3 `
        --output NUL --write-out '%{http_code}' "http://127.0.0.1:80$path"

    if ($LASTEXITCODE -ne 0) {
        throw "Could not request $path"
    }

    if ($status -ne $expected) {
        throw "Expected $expected for $path, got $status"
    }

    Write-Output "$path -> $status (OK)"
}

Assert-Status '/index.html' '200'
Assert-Status '/missing.html' '404'
Assert-Status '/index.html' '200'

$original = Join-Path $PSScriptRoot '..\www\index.html'
$download = New-TemporaryFile

try {
    curl.exe --noproxy '*' --silent --show-error --fail --max-time 3 `
        --output $download.FullName 'http://127.0.0.1:80/index.html'

    if ($LASTEXITCODE -ne 0) {
        throw 'Could not download index.html'
    }

    $originalHash = (Get-FileHash -LiteralPath $original -Algorithm SHA256).Hash
    $downloadHash = (Get-FileHash -LiteralPath $download.FullName -Algorithm SHA256).Hash

    if ($originalHash -ne $downloadHash) {
        throw 'Response body differs from index.html'
    }

    Write-Output 'File contents match (OK)'
} finally {
    Remove-Item -LiteralPath $download.FullName
}

$slowClient = [System.Net.Sockets.TcpClient]::new()

try {
    $slowClient.Connect('127.0.0.1', 80)
    $stream = $slowClient.GetStream()

    $partial = [System.Text.Encoding]::ASCII.GetBytes(
            "GET /index.html HTTP/1.1`r`nHost: localhost`r`n"
    )
    $stream.Write($partial, 0, $partial.Length)

    Assert-Status '/index.html' '200'

    if ($slowClient.Client.Poll(0, [System.Net.Sockets.SelectMode]::SelectRead)) {
        throw 'The first connection received data or was closed'
    }

    Write-Output 'Second client received 200 while the first was waiting (OK)'
} finally {
    $slowClient.Dispose()
}

$name = "test-$([guid]::NewGuid().ToString('N')).bin"
$fixture = Join-Path $PSScriptRoot "..\www\$name"
$download = New-TemporaryFile

try {
    [byte[]]$bytes = @(0, 13, 10, 26, 255) * 1000
    [System.IO.File]::WriteAllBytes($fixture, $bytes)

    curl.exe --noproxy '*' --silent --show-error --fail --max-time 3 `
        --output $download.FullName "http://127.0.0.1:80/$name"

    if ($LASTEXITCODE -ne 0) {
        throw 'Could not download binary file'
    }

    $expected = (Get-FileHash -LiteralPath $fixture -Algorithm SHA256).Hash
    $actual = (Get-FileHash -LiteralPath $download.FullName -Algorithm SHA256).Hash

    if ($actual -ne $expected) {
        throw 'Binary response differs from the original file'
    }

    Write-Output 'Binary file matches (OK)'
} finally {
    Remove-Item -LiteralPath $fixture, $download.FullName -ErrorAction SilentlyContinue
}
