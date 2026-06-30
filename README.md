# Embedded Systems

Repositório com os projetos desenvolvidas na disciplina de **Projetos de Sistemas Computacionais Embarcados**.

## Estrutura do Repositório

```
embedded-systems/
├── blinker/          # Prática 1 — LED Blinker
├── lcd-keypad/       # Prática 2 — LCD 16×2 + Teclado Matricial + 7Seg-Display
├── safe-system/      # Prática 3 — Sistema de Cofre com Senha
├── games/            # Prática 4 — Jogos (T-Rex Runner & Stock Car)
├── serial/           # Prática 5 — Comunicação Serial
├── light_pid/        # Prática 6 — Light PID
├── rtos/             # Prática 7 — RTOS
└── tetris/           # Projeto Final — Tetris
```

## Prática 1: LED Blinker

A primeira prática consiste em um programa clássico de pisca-LED (*blinker*), escrito em **Assembly**. O programa alterna o estado do pino `P1.0` continuamente, com um atraso gerado por dois loops aninhados usando os registradores `R0` e `R1`.

### Conceitos abordados
- Configuração e manipulação de portas GPIO (`clr`, `setb`)
- Sub-rotinas de delay por software (loops com `DJNZ`)
- Estrutura básica de um programa Assembly 8051 (`org`, `sjmp`, `acall`, `ret`)
- Gravação do firmware via ISP

## Prática 2: LCD 16×2 + Teclado Matricial + 7-Seg Display

Nesta prática, o 8052 controla um **display LCD 16×2** no modo de **4 bits** e lê entradas de um **teclado matricial 4×3**. Os caracteres digitados são exibidos no LCD em tempo real. Ao pressionar `#`, o programa encerra. Adicionalmente, um programa separado controla **4 displays de 7 segmentos** multiplexados para exibir o número `8207`.

### Conceitos abordados
- Inicialização e comunicação com LCD HD44780 em modo 4 bits
- Envio de comandos e dados via nibbles (alto e baixo)
- Varredura de teclado matricial (scan por linhas e colunas)
- Debounce de tecla por software (sub-rotina `ESPSOL` com Timer 0)
- Multiplexação de displays 7 segmentos com chip decodificador

## Prática 3: Sistema de Cofre com Senha

Sistema de cofre digital que solicita uma senha de **4 dígitos** via teclado matricial, exibe asteriscos (`*`) no LCD conforme o usuário digita e valida a senha contra um valor armazenado. Se a senha estiver correta, exibe **"BEM VINDO"**; caso contrário, exibe **"SENHA ERRADA"**. Implementado tanto em **Assembly** quanto em **C** para o 8052.

### Conceitos abordados
- Armazenamento de dados na RAM interna do 8052
- Comparação de strings: senha digitada (RAM) vs. senha correta (ROM / array)
- Controle de fluxo com contadores e ponteiros indiretos (`@R0`, `DPTR`)
- Reescrita do projeto de Assembly para C (usando `reg51.h`)
- Mascaramento de entrada (exibição de `*` em vez do dígito real)

## Prática 4: Jogos para 8052

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

## Prática 5: Comunicação Serial

Prática focada na comunicação UART entre o microcontrolador **8052** e um **Arduino**. O sistema demonstra o envio e recebimento de dados seriais, permitindo a integração e a transferência de comandos bidirecionais.

### Conceitos abordados
- Configuração de Baud Rate via Timer 1 no 8052
- Registradores `SCON` e `SBUF`
- Tratamento de interrupções seriais
- Sincronização e tratamento de dados no lado do Arduino

## Prática 6: Light PID

Implementação de um controlador PID (Proporcional, Integral e Derivativo) utilizando um **Arduino** para controlar a intensidade luminosa de um LED. Um sensor LDR atua como *feedback* lendo a luminosidade do ambiente, e um display I2C (Grove RGB LCD) exibe as informações em tempo real.

### Conceitos abordados
- Malha de controle fechada (Closed-loop)
- Sintonia empírica de ganhos Kp, Ki e Kd
- Leitura analógica (ADC) do sensor LDR e controle de potência via PWM
- Utilização da biblioteca `PID_v1` no ecossistema do Arduino

## Prática 7: RTOS (Cálculo de Pi)

Introdução a Sistemas Operacionais de Tempo Real utilizando a biblioteca **FreeRTOS** no Arduino. A prática demonstra a execução concorrente de tarefas, calculando o valor de Pi (π) interativamente através do método de **Monte Carlo**.

### Conceitos abordados
- Criação de tarefas concorrentes (`xTaskCreate`) com diferentes prioridades
- Preempção e escalonamento de tarefas (TaskCalc e TaskPrint)
- Algoritmo de Monte Carlo para aproximação de Pi
- Cedência de controle de processamento (`vTaskDelay`) e escopo de variáveis globais

## Projeto Final: Tetris Arcade (Bare-Metal ARM)

Projeto final da disciplina consistindo em um **clone completo do Tetris** desenvolvido em **C (Bare-Metal)** para a placa **Terasic DE10-Standard (SoCFPGA Cyclone V com ARM Cortex-A9)**. O jogo roda diretamente no hardware sem nenhum Sistema Operacional embarcado, acessando diretamente os controladores de vídeo e os botões.

### Mecânicas e Conceitos abordados
- **Renderização Gráfica via VGA**: Controle do buffer de vídeo (resolução 320x240, 16-bit RGB565) a partir do mapeamento de memória base do SoC.
- **Primitivas de Renderização Customizadas**: Funções desenhadas do zero para renderizar pixels, retângulos, linhas e textos char-a-char na tela.
- **Lógica e Física do Tetris**: Detecção de colisão via grid, rotação dos tetrominós em suas matrizes, animação de queda, e detecção/limpeza de linhas concluídas.
- **Máquina de Estados de Jogo**: Separação clara entre Menu Inicial, Loop de Jogo, e Tela de Game Over.
- **Polling de Botões Nativos**: Leitura contínua dos registradores dos botões da placa (KEY0 a KEY3) para navegação, rotação e queda livre (drop).

## Autores
Desenvolvido por Gustavo de Oliveira Gimenes e Vinícius Marto da Veiga.