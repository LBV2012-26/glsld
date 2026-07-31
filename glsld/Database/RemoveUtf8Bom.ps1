param([string]$Path = $PSScriptRoot)

$root = (Resolve-Path -LiteralPath $Path).Path
$removed = 0

Get-ChildItem -LiteralPath $root -Recurse -File | ForEach-Object {
    $bytes = [System.IO.File]::ReadAllBytes($_.FullName)
    if ($bytes.Length -lt 3 -or $bytes[0] -ne 0xEF -or $bytes[1] -ne 0xBB -or $bytes[2] -ne 0xBF) {
        return
    }

    $content = if ($bytes.Length -eq 3) { [byte[]]::new(0) } else { [byte[]]$bytes[3..($bytes.Length - 1)] }
    [System.IO.File]::WriteAllBytes($_.FullName, $content)
    ++$removed
    Write-Host "Removed UTF-8 BOM: $($_.FullName)"
}

Write-Host "Removed UTF-8 BOM from $removed file(s)."
