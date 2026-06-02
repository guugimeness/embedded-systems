# Embedded Systems

Repositório com os projetos desenvolvidas na disciplina de **Projetos de Sistemas Computacionais Embarcados**.

## Estrutura do Repositório

```
embedded-systems/
├── blinker/          # Prática 1 — LED Blinker
├── lcd-keypad/       # Prática 2 — LCD 16×2 + Teclado Matricial + 7Seg-Display
├── safe-system/      # Prática 3 — Sistema de Cofre com Senha
├── serial/           # Prática 4 — Comunicação Serial (8052 ↔ Arduino)
└── games/            # Prática 5 — Jogos (T-Rex Runner & Stock Car)
```

## Projeto 1: LED Blinker

A primeira prática consiste em um programa clássico de pisca-LED (*blinker*), escrito em **Assembly**. O programa alterna o estado do pino `P1.0` continuamente, com um atraso gerado por dois loops aninhados usando os registradores `R0` e `R1`.

### Conceitos abordados
- Configuração e manipulação de portas GPIO (`clr`, `setb`)
- Sub-rotinas de delay por software (loops com `DJNZ`)
- Estrutura básica de um programa Assembly 8051 (`org`, `sjmp`, `acall`, `ret`)
- Gravação do firmware via ISP

## Projeto 2: LCD 16×2 + Teclado Matricial + 7-Seg Display

Nesta prática, o 8052 controla um **display LCD 16×2** no modo de **4 bits** e lê entradas de um **teclado matricial 4×3**. Os caracteres digitados são exibidos no LCD em tempo real. Ao pressionar `#`, o programa encerra. Adicionalmente, um programa separado controla **4 displays de 7 segmentos** multiplexados para exibir o número `8207`.

### Conceitos abordados
- Inicialização e comunicação com LCD HD44780 em modo 4 bits
- Envio de comandos e dados via nibbles (alto e baixo)
- Varredura de teclado matricial (scan por linhas e colunas)
- Debounce de tecla por software (sub-rotina `ESPSOL` com Timer 0)
- Multiplexação de displays 7 segmentos com chip decodificador

## Projeto 3: Sistema de Cofre com Senha

Sistema de cofre digital que solicita uma senha de **4 dígitos** via teclado matricial, exibe asteriscos (`*`) no LCD conforme o usuário digita e valida a senha contra um valor armazenado. Se a senha estiver correta, exibe **"BEM VINDO"**; caso contrário, exibe **"SENHA ERRADA"**. Implementado tanto em **Assembly** quanto em **C** para o 8052.

### Conceitos abordados
- Armazenamento de dados na RAM interna do 8052
- Comparação de strings: senha digitada (RAM) vs. senha correta (ROM / array)
- Controle de fluxo com contadores e ponteiros indiretos (`@R0`, `DPTR`)
- Reescrita do projeto de Assembly para C (usando `reg51.h`)
- Mascaramento de entrada (exibição de `*` em vez do dígito real)

## Projeto 4: Jogos para 8052

Prática avançada com **três jogos** desenvolvidos em **C** para o 8052, utilizando displays LCD 16×2 e GLCD 128×64 (ST7920). Os jogos demonstram renderização gráfica, sprites customizados, detecção de colisão e progressão de dificuldade — tudo rodando em um microcontrolador de 8 bits.

### T-Rex Runner (LCD 16×2)

Recriação do clássico jogo do dinossauro do Chrome, adaptado para um display LCD alfanumérico de 16×2. O T-Rex corre na linha inferior desviando de cactos e de pássaros.

**Mecânicas:**
- **Sprites customizados** via CGRAM do LCD (cacto, pássaro, T-Rex)
- **Modo Noite** ativado automaticamente ao atingir 100 pontos (sprites invertidos)
- **Velocidade progressiva** — o jogo acelera a cada 25 pontos
- **Detecção de colisão** contra cactos (no chão) e pássaros (no ar)
- **Sistema de score** com exibição em 3 dígitos no canto do display
- **Game Over** com exibição de pontuação e restart por botão

### T-Rex Runner (GLCD 128×64)

Versão aprimorada do T-Rex Runner para o display gráfico **GLCD 128×64** com controlador **ST7920**. Renderização pixel a pixel com sprites bitmap de 16×16 e fonte customizada para exibição do score.

**Mecânicas adicionais:**
- **Renderização gráfica** com controle direto de pixels via modo gráfico estendido do ST7920
- **Mapa scrollável** com obstáculos mapeados em bytes
- **Ciclo Dia/Noite** alternando a cada 50 pontos (inversão bit a bit dos dados do display)
- **Colisão pixel-perfect** comparando bits do sprite do dinossauro com os bits do mapa
- **Velocidade progressiva** do scroll do mapa

### Stock Car (GLCD 128×64)

Jogo de corrida top-down para o GLCD 128×64 onde o jogador controla um carro em uma pista que se move verticalmente. O objetivo é sobreviver o máximo possível desviando das bordas da pista e de carros inimigos.

**Mecânicas:**
- **Pista procedural** com curvas definidas por um mapa circular de 32 posições
- **Sprites 16×16** para carro do jogador, carro inimigo, ambulância e explosão
- **Carros inimigos** que aparecem em lanes alternadas após 5 km
- **Painel de informações** no canto direito: tempo, quilometragem e batidas
- **Animação de crash** com sequência: explosão → ambulância se aproximando → sirene piscando
- **Controle por teclado matricial** (teclas 7 e 9 para esquerda/direita)
- **Restart** após colisão pressionando o botão central

## Projeto 5: Comunicação Serial (8052 ↔ Arduino)

Prática de **comunicação serial assíncrona (UART)** entre um **Arduino** (transmissor) e o **8052** (receptor). O Arduino envia a string `"SistEmb-"` caractere por caractere a cada 1 segundo. O 8052 recebe cada byte via interrupção serial (`ORG 23h`) e exibe imediatamente no LCD 16×2.

### Conceitos abordados
- Configuração da UART do 8052 em Modo 1 (8 bits, baud rate variável)
- Uso do Timer 1 em modo Auto-Reload para geração de baud rate
- Bit `SMOD` (dobrador de baud rate) no registrador `PCON`
- Interrupções seriais (`ES`, `EA`, `RI`)
- Comunicação entre plataformas diferentes (Arduino → 8052) a **9600 bps**

## Autores

Desenvolvido por Gustavo de Oliveira Gimenes e Vinícius Marto da Veiga.