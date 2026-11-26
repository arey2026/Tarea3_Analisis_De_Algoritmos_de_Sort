# generar_excel.ps1
# ---------------------------------------------------------------
# Crea results/resultados_sort.xlsx a partir de los CSV que genera
# benchmark.exe, usando Microsoft Excel (automatizacion COM).
# Lo llama automaticamente benchmark.exe al terminar, o se puede
# ejecutar a mano con:  mingw32-make excel
#
# Hojas: Resultados, Analisis, Complejidad, Graficos, Equipo.
# Los graficos tambien se exportan como PNG a results/graficos/.
#
# NUNCA se inventan tiempos: una prueba OMITIDA aparece como "Omitido"
# en las tablas y como #N/A (punto vacio) en los graficos.
# ---------------------------------------------------------------
$ErrorActionPreference = "Stop"

$raiz        = Split-Path -Parent $PSScriptRoot
$csvPath     = Join-Path $raiz "results\resultados_sort.csv"
$cfgPath     = Join-Path $raiz "results\config_benchmark.csv"
$xlsxPath    = Join-Path $raiz "results\resultados_sort.xlsx"
$graficosDir = Join-Path $raiz "results\graficos"
$inv         = [Globalization.CultureInfo]::InvariantCulture

if (-not (Test-Path $csvPath)) { throw "No existe $csvPath. Ejecuta primero benchmark.exe." }

$filas      = Import-Csv $csvPath
$config     = Import-Csv $cfgPath
$algoritmos = @("Insertion Sort", "Selection Sort", "Bubble Sort", "Merge Sort", "Heap Sort", "Quick Sort")
$cuadraticos = @("Insertion Sort", "Selection Sort", "Bubble Sort")
$nlogn       = @("Merge Sort", "Heap Sort", "Quick Sort")
# [int[]]: Sort-Object devuelve los numeros "envueltos" en objetos de PowerShell,
# y Excel (COM) no los acepta; la conversion los deja como enteros puros.
[int[]]$tamanos = @($filas | ForEach-Object { [int]$_.Tamano } | Sort-Object -Unique)
$columnas   = @("B", "C", "D", "E", "F", "G")   # columna de cada algoritmo
$ultimaFila = $tamanos.Count + 1                # fila del ultimo tamano en Resultados

# Indice para buscar rapido: "n|Algoritmo" -> fila del CSV
$dato = @{}
foreach ($f in $filas) { $dato["$($f.Tamano)|$($f.Algoritmo)"] = $f }

function Get-Tiempo([int]$n, [string]$alg) {
    $f = $dato["$n|$alg"]
    if ($null -eq $f -or $f.Estado -eq "OMITIDO") { return $null }
    return [double]::Parse($f.PromedioMs, $inv)
}
function RGB([int]$r, [int]$g, [int]$b) { return $r + 256 * $g + 65536 * $b }

# Tamanos en los que los seis algoritmos terminaron (comparacion justa entre todos)
$comunes = @($tamanos | Where-Object { $n = $_; @($algoritmos | Where-Object { $null -eq (Get-Tiempo $n $_) }).Count -eq 0 })
$nComun  = ($comunes | Measure-Object -Maximum).Maximum
$nMax    = ($tamanos | Measure-Object -Maximum).Maximum

$colorEncabezado = RGB 31 78 121
$colorOmitido    = RGB 217 217 217
$colorLimite     = RGB 252 228 214
$colorRapido     = RGB 198 239 206

# Escribe un valor en una celda o rango de Excel.
# Se usa InvokeMember en lugar de "$celda.Value2 = $valor" porque PowerShell 5.1
# tiene un fallo con COM: despues de asignar un texto desde una variable,
# rechaza asignar numeros ("No se puede convertir Int32 a String").
function Set-Valor($rango, $valor) {
    [void]$rango.GetType().InvokeMember("Value2", [Reflection.BindingFlags]::SetProperty, $null, $rango, @($valor))
}

function Set-Encabezado($ws, [string]$rango) {
    $r = $ws.Range($rango)
    $r.Font.Bold = $true
    $r.Font.Color = RGB 255 255 255
    $r.Interior.Color = $colorEncabezado
}
function Set-Titulo($ws, [string]$celda, [string]$texto) {
    Set-Valor ($ws.Range($celda)) ($texto)
    $ws.Range($celda).Font.Bold = $true
    $ws.Range($celda).Font.Size = 13
}

$excel = New-Object -ComObject Excel.Application
$excel.Visible = $false
$excel.DisplayAlerts = $false
$wb = $null

try {
    $wb = $excel.Workbooks.Add()
    while ($wb.Worksheets.Count -lt 5) {
        [void]$wb.Worksheets.Add([Type]::Missing, $wb.Worksheets.Item($wb.Worksheets.Count))
    }
    $wsRes  = $wb.Worksheets.Item(1); $wsRes.Name  = "Resultados"
    $wsAna  = $wb.Worksheets.Item(2); $wsAna.Name  = "Análisis"
    $wsComp = $wb.Worksheets.Item(3); $wsComp.Name = "Complejidad"
    $wsGraf = $wb.Worksheets.Item(4); $wsGraf.Name = "Gráficos"
    $wsEq   = $wb.Worksheets.Item(5); $wsEq.Name   = "Equipo"

    # =========================================================
    # HOJA 1: RESULTADOS (tiempo promedio en ms)
    # =========================================================
    Set-Valor ($wsRes.Cells.Item(1, 1)) ("Tamaño")
    for ($c = 0; $c -lt 6; $c++) { Set-Valor ($wsRes.Cells.Item(1, $c + 2)) ($algoritmos[$c]) }
    Set-Encabezado $wsRes "A1:G1"

    $notas = @()
    for ($i = 0; $i -lt $tamanos.Count; $i++) {
        $n = $tamanos[$i]; $fila = $i + 2
        Set-Valor ($wsRes.Cells.Item($fila, 1)) ($n)
        $wsRes.Cells.Item($fila, 1).NumberFormat = "#,##0"

        # tiempo minimo de la fila, para resaltar al mas rapido
        $tiemposFila = @($algoritmos | ForEach-Object { Get-Tiempo $n $_ } | Where-Object { $null -ne $_ })
        $minFila = ($tiemposFila | Measure-Object -Minimum).Minimum

        for ($c = 0; $c -lt 6; $c++) {
            $f = $dato["$n|$($algoritmos[$c])"]
            $celda = $wsRes.Cells.Item($fila, $c + 2)
            if ($f.Estado -eq "OMITIDO") {
                Set-Valor ($celda) ("Omitido")
                $celda.Interior.Color = $colorOmitido
                $celda.HorizontalAlignment = -4108   # centrado
                $notas += "n = {0:N0}, {1}: OMITIDO. {2}." -f $n, $f.Algoritmo, $f.Nota
            } else {
                $t = Get-Tiempo $n $algoritmos[$c]
                Set-Valor ($celda) ($t)
                $celda.NumberFormat = "#,##0.000000"
                if ($f.Estado -eq "SUPERO_LIMITE") {
                    $celda.Interior.Color = $colorLimite
                    $notas += "n = {0:N0}, {1}: SUPERÓ EL LÍMITE. Tiempo real medido en {2} ejecución(es); no se midieron tamaños mayores." -f $n, $f.Algoritmo, $f.Repeticiones
                } elseif ($t -eq $minFila) {
                    $celda.Interior.Color = $colorRapido
                }
            }
        }
    }

    $r = $ultimaFila + 2
    Set-Valor ($wsRes.Cells.Item($r, 1)) ("Valores: tiempo PROMEDIO por ordenamiento, en milisegundos (ms), de las ejecuciones completadas.")
    Set-Valor ($wsRes.Cells.Item($r + 1, 1)) ("Verde = más rápido del tamaño.  Gris = Omitido (no se ejecutó, no tiene tiempo).  Naranja = terminó pero superó el límite de tiempo.")
    $r += 3
    Set-Titulo $wsRes "A$r" "Notas sobre pruebas omitidas o que superaron el límite"
    $r++
    if ($notas.Count -eq 0) { Set-Valor ($wsRes.Cells.Item($r, 1)) ("Todas las pruebas se completaron."); $r++ }
    foreach ($nota in $notas) { Set-Valor ($wsRes.Cells.Item($r, 1)) ($nota); $r++ }

    # Tabla de repeticiones ejecutadas
    $r += 1
    Set-Titulo $wsRes "A$r" "Repeticiones completadas por prueba (cada repetición ordena un lote de copias)"
    $r++
    $filaRep = $r
    Set-Valor ($wsRes.Cells.Item($r, 1)) ("Tamaño")
    Set-Valor ($wsRes.Cells.Item($r, 8)) ("Lote")
    for ($c = 0; $c -lt 6; $c++) { Set-Valor ($wsRes.Cells.Item($r, $c + 2)) ($algoritmos[$c]) }
    Set-Encabezado $wsRes "A${r}:H${r}"
    foreach ($n in $tamanos) {
        $r++
        Set-Valor ($wsRes.Cells.Item($r, 1)) ($n)
        $wsRes.Cells.Item($r, 1).NumberFormat = "#,##0"
        for ($c = 0; $c -lt 6; $c++) {
            Set-Valor ($wsRes.Cells.Item($r, $c + 2)) ([int]$dato["$n|$($algoritmos[$c])"].Repeticiones)
        }
        Set-Valor ($wsRes.Cells.Item($r, 8)) ([int]$dato["$n|$($algoritmos[0])"].Lote)
    }
    # Columna A: solo según las tablas (las notas largas pueden desbordar a la derecha).
    # Columnas B-H: según todo su contenido, para que ningún número muestre "####".
    [void]$wsRes.Range("A1:A$ultimaFila").Columns.AutoFit()
    [void]$wsRes.Range("B1:H$r").Columns.AutoFit()

    # =========================================================
    # HOJA 2: ANALISIS (con formulas de Excel sobre la hoja Resultados)
    # =========================================================
    Set-Titulo $wsAna "A1" "1. Algoritmo más rápido y más lento por tamaño"
    $enc = @("Tamaño", "Más rápido", "Tiempo (ms)", "Más lento (completado)", "Tiempo (ms)", "Diferencia (lento / rápido)")
    for ($c = 0; $c -lt $enc.Count; $c++) { Set-Valor ($wsAna.Cells.Item(2, $c + 1)) ($enc[$c]) }
    Set-Encabezado $wsAna "A2:F2"
    for ($i = 0; $i -lt $tamanos.Count; $i++) {
        $fr = $i + 2; $fa = $i + 3
        $rng = "Resultados!B${fr}:G${fr}"
        $wsAna.Cells.Item($fa, 1).Formula = "=Resultados!A$fr"
        $wsAna.Cells.Item($fa, 2).Formula = "=INDEX(Resultados!`$B`$1:`$G`$1,MATCH(MIN($rng),$rng,0))"
        $wsAna.Cells.Item($fa, 3).Formula = "=MIN($rng)"
        $wsAna.Cells.Item($fa, 4).Formula = "=INDEX(Resultados!`$B`$1:`$G`$1,MATCH(MAX($rng),$rng,0))"
        $wsAna.Cells.Item($fa, 5).Formula = "=MAX($rng)"
        $wsAna.Cells.Item($fa, 6).Formula = "=E$fa/C$fa"
        $wsAna.Cells.Item($fa, 1).NumberFormat = "#,##0"
        $wsAna.Cells.Item($fa, 3).NumberFormat = "#,##0.000000"
        $wsAna.Cells.Item($fa, 5).NumberFormat = "#,##0.000000"
        $wsAna.Cells.Item($fa, 6).NumberFormat = "#,##0.0""x"""
    }
    $r = $tamanos.Count + 3
    Set-Valor ($wsAna.Cells.Item($r, 1)) ("La columna 'Más lento' solo considera las pruebas completadas; las omitidas habrían sido aún más lentas.")

    # --- 2. Comparacion relativa ---
    $r += 2
    Set-Titulo $wsAna "A$r" "2. Comparación de los seis algoritmos: veces más lento que el más rápido del mismo tamaño (1.0x = el más rápido)"
    $r++
    Set-Valor ($wsAna.Cells.Item($r, 1)) ("Tamaño")
    for ($c = 0; $c -lt 6; $c++) { Set-Valor ($wsAna.Cells.Item($r, $c + 2)) ($algoritmos[$c]) }
    Set-Encabezado $wsAna "A${r}:G${r}"
    for ($i = 0; $i -lt $tamanos.Count; $i++) {
        $r++; $fr = $i + 2
        $wsAna.Cells.Item($r, 1).Formula = "=Resultados!A$fr"
        $wsAna.Cells.Item($r, 1).NumberFormat = "#,##0"
        for ($c = 0; $c -lt 6; $c++) {
            $col = $columnas[$c]
            $wsAna.Cells.Item($r, $c + 2).Formula = "=IF(ISNUMBER(Resultados!$col$fr),Resultados!$col$fr/MIN(Resultados!`$B${fr}:`$G${fr}),""Omitido"")"
            $wsAna.Cells.Item($r, $c + 2).NumberFormat = "#,##0.0""x"""
        }
    }

    # --- 3. Promedios ---
    $r += 2
    Set-Titulo $wsAna "A$r" "3. Promedios de ejecución por algoritmo"
    $r++
    $filaUltimaComun = [array]::IndexOf($tamanos, [int]$nComun) + 2
    $enc = @("Algoritmo", "Tamaños completados", "Mayor n completado", "Promedio en n ≤ $('{0:N0}' -f $nComun) (ms)", "Promedio de todos sus tamaños completados (ms)")
    for ($c = 0; $c -lt $enc.Count; $c++) { Set-Valor ($wsAna.Cells.Item($r, $c + 1)) ($enc[$c]) }
    Set-Encabezado $wsAna "A${r}:E${r}"
    for ($c = 0; $c -lt 6; $c++) {
        $r++
        $col = $columnas[$c]
        $mayorN = ($tamanos | Where-Object { $null -ne (Get-Tiempo $_ $algoritmos[$c]) } | Measure-Object -Maximum).Maximum
        Set-Valor ($wsAna.Cells.Item($r, 1)) ($algoritmos[$c])
        $wsAna.Cells.Item($r, 2).Formula = "=COUNT(Resultados!${col}2:$col$ultimaFila)"
        Set-Valor ($wsAna.Cells.Item($r, 3)) ($mayorN)
        $wsAna.Cells.Item($r, 3).NumberFormat = "#,##0"
        $wsAna.Cells.Item($r, 4).Formula = "=AVERAGE(Resultados!${col}2:$col$filaUltimaComun)"
        $wsAna.Cells.Item($r, 5).Formula = "=AVERAGE(Resultados!${col}2:$col$ultimaFila)"
        $wsAna.Range("D${r}:E${r}").NumberFormat = "#,##0.000000"
    }
    $r++
    Set-Valor ($wsAna.Cells.Item($r, 1)) ("La columna D compara a los seis con los mismos tamaños (los que todos completaron). La columna E no es comparable entre algoritmos, porque cada uno completó tamaños distintos.")

    # --- 4. Diferencias de rendimiento: crecimiento observado vs teorico ---
    $r += 2
    Set-Titulo $wsAna "A$r" "4. Diferencias de rendimiento: ¿cuánto crece el tiempo al aumentar n? (observado vs teórico)"
    $r++
    Set-Valor ($wsAna.Cells.Item($r, 1)) ("Desde n")
    Set-Valor ($wsAna.Cells.Item($r, 2)) ("Hasta n")
    for ($c = 0; $c -lt 6; $c++) { Set-Valor ($wsAna.Cells.Item($r, $c + 3)) ($algoritmos[$c]) }
    Set-Valor ($wsAna.Cells.Item($r, 9)) ("Teórico O(n²)")
    Set-Valor ($wsAna.Cells.Item($r, 10)) ("Teórico O(n log n)")
    Set-Encabezado $wsAna "A${r}:J${r}"
    for ($i = 1; $i -lt $tamanos.Count; $i++) {
        $r++; $f0 = $i + 1; $f1 = $i + 2
        $wsAna.Cells.Item($r, 1).Formula = "=Resultados!A$f0"
        $wsAna.Cells.Item($r, 2).Formula = "=Resultados!A$f1"
        $wsAna.Range("A${r}:B${r}").NumberFormat = "#,##0"
        for ($c = 0; $c -lt 6; $c++) {
            $col = $columnas[$c]
            $wsAna.Cells.Item($r, $c + 3).Formula = "=IF(AND(ISNUMBER(Resultados!$col$f0),ISNUMBER(Resultados!$col$f1)),Resultados!$col$f1/Resultados!$col$f0,""-"")"
        }
        $wsAna.Cells.Item($r, 9).Formula  = "=(B$r/A$r)^2"
        $wsAna.Cells.Item($r, 10).Formula = "=(B$r*LOG(B$r,2))/(A$r*LOG(A$r,2))"
        $wsAna.Range("C${r}:J${r}").NumberFormat = "#,##0.0""x"""
    }
    $r++
    Set-Valor ($wsAna.Cells.Item($r, 1)) ("Ejemplo: si n se duplica, un algoritmo O(n²) debería tardar ~4 veces más; uno O(n log n), un poco más del doble.")
    [void]$wsAna.Range("A2:J$r").Columns.AutoFit()
    $wsAna.Columns.Item(1).ColumnWidth = 12

    # --- 5. Observaciones (calculadas a partir de los datos medidos) ---
    $r += 2
    Set-Titulo $wsAna "A$r" "5. Observaciones sobre los resultados"
    $obs = @()

    $tc = @{}; foreach ($a in $algoritmos) { $tc[$a] = Get-Tiempo $nComun $a }
    $ordenComun = $algoritmos | Sort-Object { $tc[$_] }
    $rap = $ordenComun[0]; $len = $ordenComun[-1]
    $obs += "El mayor tamaño que completaron los seis algoritmos fue n = {0:N0}. Ahí el más rápido fue {1} ({2:N4} ms) y el más lento {3} ({4:N4} ms): {5:N1} veces más lento." -f $nComun, $rap, $tc[$rap], $len, $tc[$len], ($tc[$len] / $tc[$rap])

    $mejorCuad = $cuadraticos | Sort-Object { $tc[$_] } | Select-Object -First 1
    $peorLog   = $nlogn | Sort-Object { $tc[$_] } | Select-Object -Last 1
    $obs += "En n = {0:N0}, incluso el O(n log n) más lento ({1}, {2:N4} ms) fue {3:N1} veces más rápido que el O(n²) más rápido ({4}, {5:N4} ms)." -f $nComun, $peorLog, $tc[$peorLog], ($tc[$mejorCuad] / $tc[$peorLog]), $mejorCuad, $tc[$mejorCuad]

    $ordenCuad = $cuadraticos | Sort-Object { $tc[$_] }
    $obs += "Entre los O(n²), en n = {0:N0} el orden fue: {1}. Insertion Sort suele ganar porque en promedio hace ~n²/4 desplazamientos y Selection Sort siempre hace ~n²/2 comparaciones; Bubble Sort hace muchos intercambios." -f $nComun, (($ordenCuad | ForEach-Object { "{0} ({1:N2} ms)" -f $_, $tc[$_] }) -join " < ")

    $tMax = @{}; foreach ($a in $nlogn) { $tMax[$a] = Get-Tiempo $nMax $a }
    $conTiempo = @($nlogn | Where-Object { $null -ne $tMax[$_] })
    if ($conTiempo.Count -gt 0) {
        $ordenMax = $conTiempo | Sort-Object { $tMax[$_] }
        $obs += "Con n = {0:N0}, los O(n log n) tardaron: {1}." -f $nMax, (($ordenMax | ForEach-Object { "{0} {1:N2} ms" -f $_, $tMax[$_] }) -join ", ")
    }

    foreach ($a in $cuadraticos) {
        $completados = @($tamanos | Where-Object { $null -ne (Get-Tiempo $_ $a) })
        if ($completados.Count -ge 2) {
            $n1 = $completados[-2]; $n2 = $completados[-1]
            $obs += "{0}: al pasar de n = {1:N0} a n = {2:N0} el tiempo se multiplicó por {3:N2}; la teoría O(n²) predice {4:N2}." -f $a, $n1, $n2, ((Get-Tiempo $n2 $a) / (Get-Tiempo $n1 $a)), [math]::Pow($n2 / $n1, 2)
        }
    }
    $omitidas = @($filas | Where-Object { $_.Estado -eq "OMITIDO" }).Count
    $obs += "Se omitieron $omitidas de $($filas.Count) pruebas porque su tiempo estimado superaba el límite configurado; esas celdas no tienen tiempo (no se inventaron valores)."
    $obs += "En tamaños muy pequeños (n = 10 y 100) los tiempos son de microsegundos; las diferencias entre algoritmos son mínimas y están más influidas por detalles de implementación y del procesador que por la complejidad."

    foreach ($o in $obs) { $r++; Set-Valor ($wsAna.Cells.Item($r, 1)) ("• $o") }

    # =========================================================
    # HOJA 3: COMPLEJIDAD
    # =========================================================
    $tabla = @(
        @("Algoritmo", "Mejor caso", "Caso promedio", "Peor caso", "Complejidad espacial", "Estable", "Grupo"),
        @("Insertion Sort", "O(n)", "O(n²)", "O(n²)", "O(1)", "Sí", "O(n²)"),
        @("Selection Sort", "O(n²)", "O(n²)", "O(n²)", "O(1)", "No", "O(n²)"),
        @("Bubble Sort", "O(n)", "O(n²)", "O(n²)", "O(1)", "Sí", "O(n²)"),
        @("Merge Sort", "O(n log n)", "O(n log n)", "O(n log n)", "O(n)", "Sí", "O(n log n)"),
        @("Heap Sort", "O(n log n)", "O(n log n)", "O(n log n)", "O(1)", "No", "O(n log n)"),
        @("Quick Sort", "O(n log n)", "O(n log n)", "O(n²)", "O(log n)", "No", "O(n log n)")
    )
    for ($i = 0; $i -lt $tabla.Count; $i++) {
        for ($c = 0; $c -lt $tabla[$i].Count; $c++) { Set-Valor ($wsComp.Cells.Item($i + 1, $c + 1)) ($tabla[$i][$c]) }
    }
    Set-Encabezado $wsComp "A1:G1"
    $notasComp = @(
        "Bubble Sort tiene mejor caso O(n) gracias a la bandera 'huboIntercambio' (si una pasada no intercambia nada, termina).",
        "Merge Sort usa un arreglo auxiliar de tamaño n: espacio O(n).",
        "Quick Sort usa el elemento del medio como pivote; su peor caso O(n²) es muy improbable con datos aleatorios. Espacio O(log n) promedio por la recursión.",
        "Estable = conserva el orden relativo de los elementos iguales."
    )
    $r = 9
    foreach ($nt in $notasComp) { Set-Valor ($wsComp.Cells.Item($r, 1)) ($nt); $r++ }
    [void]$wsComp.Range("A1:G7").Columns.AutoFit()

    # =========================================================
    # HOJA 4: GRAFICOS
    # =========================================================
    Set-Titulo $wsGraf "A1" "Datos de los gráficos (ms). #N/A = prueba omitida (el gráfico no dibuja ese punto)."
    Set-Valor ($wsGraf.Cells.Item(2, 1)) ("Tamaño")
    for ($c = 0; $c -lt 6; $c++) { Set-Valor ($wsGraf.Cells.Item(2, $c + 2)) ($algoritmos[$c]) }
    Set-Encabezado $wsGraf "A2:G2"
    for ($i = 0; $i -lt $tamanos.Count; $i++) {
        $fg = $i + 3; $n = $tamanos[$i]
        Set-Valor ($wsGraf.Cells.Item($fg, 1)) ($n)
        for ($c = 0; $c -lt 6; $c++) {
            $t = Get-Tiempo $n $algoritmos[$c]
            if ($null -eq $t) { $wsGraf.Cells.Item($fg, $c + 2).Formula = "=NA()" }
            else { Set-Valor ($wsGraf.Cells.Item($fg, $c + 2)) ($t) }
        }
    }
    $ultimaFilaGraf = $tamanos.Count + 2
    $filaComunGraf  = [array]::IndexOf($tamanos, [int]$nComun) + 3
    [void]$wsGraf.Range("A2:G$ultimaFilaGraf").Columns.AutoFit()

    if (-not (Test-Path $graficosDir)) { New-Item -ItemType Directory $graficosDir | Out-Null }

    function New-GraficoLineas($titulo, $algs, $filaFin, [bool]$logX, [bool]$logY, $top, $archivo) {
        $co = $wsGraf.ChartObjects().Add(10, $top, 720, 380)
        $ch = $co.Chart
        $ch.ChartType = 74   # xlXYScatterLines (dispersión con líneas)
        while ($ch.SeriesCollection().Count -gt 0) { $ch.SeriesCollection(1).Delete() }
        foreach ($a in $algs) {
            $col = $columnas[[array]::IndexOf($algoritmos, $a)]
            $s = $ch.SeriesCollection().NewSeries()
            $s.Name = $a
            $s.XValues = $wsGraf.Range("A3:A$filaFin")
            $s.Values  = $wsGraf.Range("${col}3:$col$filaFin")
        }
        $ch.HasTitle = $true
        $ch.ChartTitle.Text = $titulo
        $ch.Axes(1).HasTitle = $true
        $ch.Axes(1).AxisTitle.Text = "Tamaño del arreglo (n)" + $(if ($logX) { " - escala logarítmica" } else { "" })
        $ch.Axes(2).HasTitle = $true
        $ch.Axes(2).AxisTitle.Text = "Tiempo (ms)" + $(if ($logY) { " - escala logarítmica" } else { "" })
        if ($logX) { $ch.Axes(1).ScaleType = -4133 }   # xlScaleLogarithmic
        if ($logY) { $ch.Axes(2).ScaleType = -4133 }
    }

    $top = ($ultimaFilaGraf + 2) * 15
    New-GraficoLineas "Tiempo de ejecución vs tamaño: los seis algoritmos (escala log-log)" $algoritmos $ultimaFilaGraf $true $true $top "1_todos_log_log.png"
    $top += 400
    New-GraficoLineas ("O(n²) vs O(n log n) hasta n = {0:N0} (escala lineal)" -f $nComun) $algoritmos $filaComunGraf $false $false $top "2_cuadraticos_vs_nlogn_lineal.png"
    $top += 400
    New-GraficoLineas "Algoritmos O(n log n) hasta n = 1,000,000 (escala lineal)" $nlogn $ultimaFilaGraf $false $false $top "3_nlogn_lineal.png"
    $top += 400

    # Grafico de barras: los seis en el mayor tamaño común
    $co = $wsGraf.ChartObjects().Add(10, $top, 720, 380)
    $ch = $co.Chart
    $ch.ChartType = 51   # xlColumnClustered (barras verticales)
    while ($ch.SeriesCollection().Count -gt 0) { $ch.SeriesCollection(1).Delete() }
    $s = $ch.SeriesCollection().NewSeries()
    $s.Name = "n = {0:N0}" -f $nComun
    $s.XValues = $wsGraf.Range("B2:G2")
    $s.Values  = $wsGraf.Range("B${filaComunGraf}:G$filaComunGraf")
    $s.HasDataLabels = $true
    $s.DataLabels().NumberFormat = "#,##0.00"
    $ch.HasLegend = $false
    $ch.HasTitle = $true
    $ch.ChartTitle.Text = "Comparación de los seis algoritmos con n = {0:N0} (escala logarítmica)" -f $nComun
    $ch.Axes(2).ScaleType = -4133
    $ch.Axes(2).HasTitle = $true
    $ch.Axes(2).AxisTitle.Text = "Tiempo (ms) - escala logarítmica"

    # Exportar los 4 graficos como PNG (al final, cuando ya estan dibujados).
    # Si Excel devuelve una imagen vacia, se reintenta.
    $archivosPng = @("1_todos_log_log.png", "2_cuadraticos_vs_nlogn_lineal.png", "3_nlogn_lineal.png", "4_barras_tamano_comun.png")
    $wsGraf.Activate()
    for ($k = 1; $k -le $archivosPng.Count; $k++) {
        $destino = Join-Path $graficosDir $archivosPng[$k - 1]
        $co = $wsGraf.ChartObjects($k)
        for ($intento = 1; $intento -le 5; $intento++) {
            $co.Activate()
            Start-Sleep -Milliseconds 300
            [void]$co.Chart.Export($destino)
            if ((Test-Path $destino) -and (Get-Item $destino).Length -gt 0) { break }
        }
        if ((Get-Item $destino).Length -eq 0) { Write-Host "AVISO: no se pudo exportar $destino" }
    }
    [void]$wsGraf.Range("A1").Select()

    # =========================================================
    # HOJA 5: EQUIPO (datos reales leídos de Windows)
    # =========================================================
    $cpu = Get-CimInstance Win32_Processor | Select-Object -First 1
    $cs  = Get-CimInstance Win32_ComputerSystem
    $os  = Get-CimInstance Win32_OperatingSystem
    $equipo = @(
        @("Fabricante / modelo", "$($cs.Manufacturer) $($cs.Model)"),
        @("Procesador", $cpu.Name.Trim()),
        @("Núcleos físicos / lógicos", "$($cpu.NumberOfCores) / $($cpu.NumberOfLogicalProcessors)"),
        @("Frecuencia base (MHz)", "$($cpu.MaxClockSpeed)"),
        @("Memoria RAM (GB)", ("{0:N1}" -f ($cs.TotalPhysicalMemory / 1GB))),
        @("Sistema operativo", "$($os.Caption) (versión $($os.Version))"),
        @("Fecha del benchmark", (Get-Item $csvPath).LastWriteTime.ToString("yyyy-MM-dd HH:mm"))
    )
    foreach ($fc in $config) { $equipo += , @($fc.Clave, $fc.Valor) }

    Set-Valor ($wsEq.Cells.Item(1, 1)) ("Característica")
    Set-Valor ($wsEq.Cells.Item(1, 2)) ("Valor")
    Set-Encabezado $wsEq "A1:B1"
    for ($i = 0; $i -lt $equipo.Count; $i++) {
        Set-Valor ($wsEq.Cells.Item($i + 2, 1)) ($equipo[$i][0])
        $wsEq.Cells.Item($i + 2, 2).NumberFormat = "@"   # texto
        Set-Valor ($wsEq.Cells.Item($i + 2, 2)) ([string]$equipo[$i][1])
    }
    [void]$wsEq.Range("A1:B$($equipo.Count + 1)").Columns.AutoFit()

    # Guardar
    $wsRes.Activate()
    if (Test-Path $xlsxPath) { Remove-Item $xlsxPath }
    $wb.SaveAs($xlsxPath, 51)   # 51 = xlOpenXMLWorkbook (.xlsx)
    Write-Host "Excel creado: $xlsxPath"
    Write-Host "Graficos PNG: $graficosDir"
}
catch {
    Write-Host "ERROR al generar el Excel (linea $($_.InvocationInfo.ScriptLineNumber)): $($_.Exception.Message)"
    Write-Host "Si el archivo esta abierto en Excel, cierralo y vuelve a intentarlo."
    exit 1
}
finally {
    if ($null -ne $wb) { $wb.Close($false) }
    $excel.Quit()
    [void][Runtime.InteropServices.Marshal]::ReleaseComObject($excel)
    [GC]::Collect()
}
