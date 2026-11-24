# Makes textures/poster.png from textures/poster_src.png:
# cuts a torn top-right corner and a small bite on the left edge (real transparency, alpha = 0),
# and adds a pale paper rim along the torn edges so the tear looks like ripped paper.
# Run from the repo root:  powershell -ExecutionPolicy Bypass -File scripts\make_poster.ps1
Add-Type -AssemblyName System.Drawing

$src = "GLProject\textures\poster_src.png"
$dst = "GLProject\textures\poster.png"
$img = [System.Drawing.Image]::FromFile((Resolve-Path $src))
$W = $img.Width; $H = $img.Height
$rnd = New-Object System.Random 7          # fixed seed: same tear every time

# Jagged line from point A to point B (a list of points with random sideways jitter)
function JaggedLine($ax, $ay, $bx, $by, $step, $jitter) {
    $pts = New-Object System.Collections.Generic.List[System.Drawing.PointF]
    $len = [math]::Sqrt(($bx-$ax)*($bx-$ax) + ($by-$ay)*($by-$ay))
    $n = [math]::Max(2, [int]($len / $step))
    $nx = -($by-$ay) / $len; $ny = ($bx-$ax) / $len          # unit normal of the line
    for ($i = 0; $i -le $n; $i++) {
        $t = $i / $n
        $o = 0
        if ($i -gt 0 -and $i -lt $n) { $o = ($rnd.NextDouble() * 2 - 1) * $jitter }
        $pts.Add((New-Object System.Drawing.PointF(($ax + ($bx-$ax)*$t + $nx*$o), ($ay + ($by-$ay)*$t + $ny*$o))))
    }
    return $pts
}

# Tear 1: top-right corner, from (0.60 W, 0) down to (W, 0.34 H)
$tear1 = JaggedLine ($W*0.60) 0 $W ($H*0.34) 22 ($W*0.018)
# Tear 2: small bite in the left edge, from (0, 0.74 H) in to (0.05 W, 0.68 H) and back out to (0, 0.62 H)
$tear2 = JaggedLine 0 ($H*0.74) ($W*0.05) ($H*0.68) 16 ($W*0.012)
$tear2b = JaggedLine ($W*0.05) ($H*0.68) 0 ($H*0.62) 16 ($W*0.012)

# Outline of the part of the poster we KEEP (clockwise)
$outline = New-Object System.Collections.Generic.List[System.Drawing.PointF]
$outline.Add((New-Object System.Drawing.PointF(0, 0)))
foreach ($p in $tear1) { $outline.Add($p) }
$outline.Add((New-Object System.Drawing.PointF($W, $H)))
$outline.Add((New-Object System.Drawing.PointF(0, $H)))
foreach ($p in $tear2) { $outline.Add($p) }
foreach ($p in $tear2b) { $outline.Add($p) }
$path = New-Object System.Drawing.Drawing2D.GraphicsPath
$path.AddPolygon($outline.ToArray())

# Draw into a transparent 32-bit image, clipped to the kept shape
$bmp = New-Object System.Drawing.Bitmap $W, $H, ([System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
$g = [System.Drawing.Graphics]::FromImage($bmp)
$g.Clear([System.Drawing.Color]::Transparent)
$g.SmoothingMode = 'AntiAlias'
$g.SetClip($path)
$g.DrawImage($img, 0, 0, $W, $H)

# Pale paper rim + thin dark edge along the torn lines (half of each pen is clipped away)
$rim = New-Object System.Drawing.Pen ([System.Drawing.Color]::FromArgb(255, 236, 224, 196)), 16
$edge = New-Object System.Drawing.Pen ([System.Drawing.Color]::FromArgb(255, 110, 80, 45)), 4
$rim.LineJoin = 'Round'; $edge.LineJoin = 'Round'
foreach ($line in @($tear1, ($tear2 + $tear2b))) {
    $arr = [System.Drawing.PointF[]]$line
    $g.DrawLines($rim, $arr)
    $g.DrawLines($edge, $arr)
}
$g.Dispose()
$img.Dispose()
$bmp.Save((Join-Path (Get-Location) $dst), [System.Drawing.Imaging.ImageFormat]::Png)
$bmp.Dispose()
"saved $dst ($W x $H)"
