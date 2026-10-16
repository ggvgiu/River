# Regenerate gameplay collision masks only. Visual models are edited separately.
$root = Split-Path $PSScriptRoot -Parent
$assembly = Get-Content "$root\RiverRaid2600AssemblerSourceCode\RiverRaid.2020AUG.asm" -Raw
function Read-Bytes([string]$label, [int]$count) {
 $match = [regex]::Match($assembly, '(?m)^' + [regex]::Escape($label) + ':\s*\r?\n')
 if (-not $match.Success) { throw "Missing sprite label $label" }
 $tail = $assembly.Substring($match.Index + $match.Length)
 $matches = [regex]::Matches($tail, '(?m)^\s*\.byte\s+\$([0-9A-Fa-f]{2})\s*;')
 if ($matches.Count -lt $count) { throw "Not enough rows for $label" }
 return @(0..($count - 1) | ForEach-Object { [Convert]::ToInt32($matches[$_].Groups[1].Value, 16) })
}
$header = "#pragma once`n#include <array>`n#include <cstdint>`n`n// Gameplay collision masks imported from Carol Shaw's River Raid shapes.`n// Visual models live separately under src/rendering/visuals/models and are not regenerated.`nnamespace RiverRaid::CollisionData`n{`n"
function Emit([string]$name, [array]$bytes) {
 $formatted = ($bytes | ForEach-Object { '0x{0:X2}' -f $_ }) -join ', '
 $script:header += "    inline constexpr std::array<uint8_t, $($bytes.Count)> $name = { $formatted };`n"
}
Emit 'JetStraight' (Read-Bytes 'JetStraight' 18)
Emit 'JetMove' (Read-Bytes 'JetMove' 18)
Emit 'JetExplode' (Read-Bytes 'JetExplode' 17)
foreach ($sprite in @(@('Fuel',11,12), @('Ship',7,7), @('Plane',9,8), @('Heli0',9,8), @('Heli1',9,8), @('House',11,11))) {
 $a = Read-Bytes ($sprite[0] + 'A') $sprite[1]
 $b = Read-Bytes ($sprite[0] + 'B') $sprite[2]
 $rows = @()
 for ($i = 0; $i -lt [Math]::Max($a.Count,$b.Count); $i++) {
  $rows += $(if ($i -lt $a.Count) { $a[$i] } else { 0 })
  $rows += $(if ($i -lt $b.Count) { $b[$i] } else { 0 })
 }
 Emit $sprite[0] $rows
}
# The bridge's interlaced pointers overlap other tables on the Atari. Use a
# readable 13-row road silhouette, retaining the original $42 / $FF rail masks.
Emit 'Bridge' @(0x42,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x42)
$header += "}`n"
Set-Content "$root\src\game\CollisionData.h" $header -Encoding utf8 -NoNewline
