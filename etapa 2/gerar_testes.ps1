# Script PowerShell para gerar testes do Sistema de Restaurante
# Alternativa ao gerar_testes.py para quem não tem Python instalado

param(
    [Parameter(Mandatory=$true, Position=0)]
    [ValidateSet('basico', 'medio', 'grande', 'prioridades', 'abandonos', 'todos')]
    [string]$Tipo,
    
    [Parameter(Mandatory=$false)]
    [string]$Output = "",
    
    [Parameter(Mandatory=$false)]
    [int]$NumClientes = 0
)

function Gerar-TesteBasico {
    param([string]$arquivo)
    
    $conteudo = @"
# Teste Básico - Poucos Clientes

CHEGADA 101 0 2 10 120
CHEGADA 102 5 4 3 60
CHEGADA 103 10 3 7 90
CHEGADA 104 15 2 5 80
CHEGADA 105 20 5 2 40

# Chamadas intercaladas
CHAMAR 30
CHAMAR 35
RELATORIO ID
CHAMAR 40
BUSCAR 101
RELATORIO ESPERA
CHAMAR 70
CHAMAR 75
FIM
"@
    
    Set-Content -Path $arquivo -Value $conteudo -Encoding UTF8
    Write-Host "[OK] Teste basico gerado: $arquivo" -ForegroundColor Green
}

function Gerar-TestePrioridades {
    param([string]$arquivo)
    
    $conteudo = @"
# Teste de Prioridades

# Clientes chegam em ordem de ID, mas com prioridades diferentes
CHEGADA 500 0 2 1 200
CHEGADA 501 5 2 3 200
CHEGADA 502 10 2 2 200
CHEGADA 503 15 2 5 200
CHEGADA 504 20 2 4 200
CHEGADA 505 25 2 10 200
CHEGADA 506 30 2 7 200
CHEGADA 507 35 2 6 200
CHEGADA 508 40 2 9 200
CHEGADA 509 45 2 8 200

# Deve atender na ordem: 505(10), 508(9), 509(8), 506(7), ...
CHAMAR 50
CHAMAR 60
CHAMAR 70
CHAMAR 80
CHAMAR 90
CHAMAR 100
CHAMAR 110
CHAMAR 120
CHAMAR 130
CHAMAR 140

RELATORIO ID
FIM
"@
    
    Set-Content -Path $arquivo -Value $conteudo -Encoding UTF8
    Write-Host "[OK] Teste de prioridades gerado: $arquivo" -ForegroundColor Green
}

function Gerar-TesteAbandonos {
    param([string]$arquivo)
    
    $conteudo = @"
# Teste de Abandonos

# Clientes com baixa tolerância
CHEGADA 600 0 2 5 20
CHEGADA 601 2 2 5 25
CHEGADA 602 4 2 5 30
CHEGADA 603 6 2 5 35
CHEGADA 604 8 2 5 40
CHEGADA 605 10 2 5 45
CHEGADA 606 12 2 5 50
CHEGADA 607 14 2 5 55
CHEGADA 608 16 2 5 60
CHEGADA 609 18 2 5 65

# Espera muito tempo antes de começar atender
CHAMAR 100
CHAMAR 110
CHAMAR 120

RELATORIO ID
FIM
"@
    
    Set-Content -Path $arquivo -Value $conteudo -Encoding UTF8
    Write-Host "[OK] Teste de abandonos gerado: $arquivo" -ForegroundColor Green
}

function Gerar-TesteMedio {
    param([string]$arquivo, [int]$num)
    
    if ($num -eq 0) { $num = 50 }
    
    $linhas = @()
    $linhas += "# Teste Médio - $num Clientes`n"
    
    $momento = 0
    for ($i = 0; $i -lt $num; $i++) {
        $id = 1000 + $i
        $grupo = Get-Random -Minimum 1 -Maximum 7
        $prioridade = Get-Random -Minimum 1 -Maximum 11
        $tolerancia = Get-Random -Minimum 30 -Maximum 151
        
        $linhas += "CHEGADA $id $momento $grupo $prioridade $tolerancia"
        
        if ((Get-Random -Minimum 0 -Maximum 100) -lt 70) {
            $momento += Get-Random -Minimum 1 -Maximum 6
        }
    }
    
    $linhas += "`n# Chamadas periódicas"
    $tempoChamada = 10
    for ($i = 0; $i -lt ($num / 2); $i++) {
        $linhas += "CHAMAR $tempoChamada"
        $tempoChamada += Get-Random -Minimum 5 -Maximum 16
    }
    
    $linhas += "`n# Relatórios"
    $linhas += "RELATORIO ID"
    $linhas += "RELATORIO ESPERA"
    
    $linhas += "`n# Buscas"
    for ($i = 0; $i -lt 5; $i++) {
        $idBusca = 1000 + (Get-Random -Minimum 0 -Maximum $num)
        $linhas += "BUSCAR $idBusca"
    }
    
    $linhas += "`nFIM"
    
    Set-Content -Path $arquivo -Value ($linhas -join "`n") -Encoding UTF8
    Write-Host "[OK] Teste medio gerado: $arquivo ($num clientes)" -ForegroundColor Green
}

function Gerar-TesteGrande {
    param([string]$arquivo, [int]$num)
    
    if ($num -eq 0) { $num = 1000 }
    
    Write-Host "Gerando teste grande com $num clientes..." -ForegroundColor Yellow
    
    $linhas = @()
    $linhas += "# Teste Grande - Benchmark ($num clientes)`n"
    
    $momento = 0
    for ($i = 0; $i -lt $num; $i++) {
        $id = 10000 + $i
        $grupo = Get-Random -Minimum 1 -Maximum 9
        $prioridade = Get-Random -Minimum 1 -Maximum 11
        $tolerancia = Get-Random -Minimum 50 -Maximum 201
        
        $linhas += "CHEGADA $id $momento $grupo $prioridade $tolerancia"
        
        if ((Get-Random -Minimum 0 -Maximum 100) -lt 80) {
            $momento += Get-Random -Minimum 1 -Maximum 4
        } else {
            $momento += Get-Random -Minimum 3 -Maximum 11
        }
    }
    
    $linhas += "`n# Processamento em lote"
    $tempoChamada = 20
    for ($i = 0; $i -lt ($num / 3); $i++) {
        $linhas += "CHAMAR $tempoChamada"
        $tempoChamada += 10
    }
    
    $linhas += "`n# Relatório final"
    $linhas += "RELATORIO ID"
    $linhas += "RELATORIO ESPERA"
    $linhas += "`nFIM"
    
    Set-Content -Path $arquivo -Value ($linhas -join "`n") -Encoding UTF8
    Write-Host "[OK] Teste grande gerado: $arquivo ($num clientes)" -ForegroundColor Green
    Write-Host "  Execute: Measure-Command { .\restaurante.exe $arquivo saida.txt }" -ForegroundColor Cyan
}

# Main
switch ($Tipo) {
    'basico' {
        $arquivo = if ($Output) { $Output } else { 'teste_basico.in' }
        Gerar-TesteBasico -arquivo $arquivo
    }
    'medio' {
        $arquivo = if ($Output) { $Output } else { 'teste_medio.in' }
        Gerar-TesteMedio -arquivo $arquivo -num $NumClientes
    }
    'grande' {
        $arquivo = if ($Output) { $Output } else { 'teste_grande.in' }
        Gerar-TesteGrande -arquivo $arquivo -num $NumClientes
    }
    'prioridades' {
        $arquivo = if ($Output) { $Output } else { 'teste_prioridades.in' }
        Gerar-TestePrioridades -arquivo $arquivo
    }
    'abandonos' {
        $arquivo = if ($Output) { $Output } else { 'teste_abandonos.in' }
        Gerar-TesteAbandonos -arquivo $arquivo
    }
    'todos' {
        Write-Host "Gerando todos os tipos de teste...`n" -ForegroundColor Yellow
        Gerar-TesteBasico -arquivo 'teste_basico.in'
        Gerar-TesteMedio -arquivo 'teste_medio.in' -num 50
        Gerar-TesteGrande -arquivo 'teste_grande.in' -num 1000
        Gerar-TestePrioridades -arquivo 'teste_prioridades.in'
        Gerar-TesteAbandonos -arquivo 'teste_abandonos.in'
        Write-Host "`n[OK] Todos os testes gerados!" -ForegroundColor Green
        Write-Host "`nExecute:" -ForegroundColor Cyan
        Write-Host "  .\restaurante.exe teste_basico.in saida1.txt"
        Write-Host "  .\restaurante.exe teste_medio.in saida2.txt"
        Write-Host "  etc..."
    }
}
