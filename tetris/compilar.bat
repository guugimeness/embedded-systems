@echo off
setlocal

REM === Binarios REAIS (PE32 Windows nativos - sem WSL) ===
set ARMTOOLS=C:\intelFPGA\23.1std\University_Program\Monitor_Program\arm_tools\baremetal
set CC=%ARMTOOLS%\arm-altera-eabi\bin\gcc.exe
set OC=%ARMTOOLS%\arm-altera-eabi\bin\objcopy.exe

REM === O gcc precisa saber onde estao cc1.exe e outros helpers ===
set GCC_EXEC_PREFIX=%ARMTOOLS%\libexec\gcc\

REM === -B e -L: diz ao gcc/ld onde encontrar crtbegin.o, libgcc.a, etc. ===
REM    IMPORTANTE: GCC exige forward slashes mesmo no Windows
set LIBGCC_DIR=C:/intelFPGA/23.1std/University_Program/Monitor_Program/arm_tools/baremetal/lib/gcc/arm-altera-eabi/4.7.3
set B_FLAG=-B%LIBGCC_DIR%/
set L_FLAG=-L%LIBGCC_DIR%/

REM === Linker script ===
set LD_SCRIPT=C:\intelFPGA\23.1std\University_Program\Monitor_Program\build\altera-socfpga-hosted.ld

REM === Flags de compilacao ===
set CCFLAGS=-Wall -c -g -O1 -std=c99 -mfloat-abi=soft -march=armv7-a -mtune=cortex-a9 -mcpu=cortex-a9

echo ============================================
echo  Compilando main.c (ARM Cortex-A9)
echo  Usando binarios Windows nativos (sem WSL)
echo ============================================

REM --- Passo 1: Compilar ---
echo [1/3] Compilando main.c ...
"%CC%" %CCFLAGS% main.c -o main.c.o
if %ERRORLEVEL% NEQ 0 (
    echo ERRO: falha na compilacao!
    goto :erro
)

REM --- Passo 2: Linkar ---
echo [2/3] Linkando main.axf ...
"%CC%" %B_FLAG% %L_FLAG% main.c.o ^
  -Wl,--defsym,arm_program_mem=0x0 ^
  -Wl,--defsym,arm_available_mem_size=0x3ffffff8 ^
  -Wl,--defsym,__cs3_stack=0x3ffffff8 ^
  -T"%LD_SCRIPT%" ^
  -o main.axf
if %ERRORLEVEL% NEQ 0 (
    echo ERRO: falha na linkagem!
    goto :erro
)

REM --- Passo 3: Gerar SREC ---
echo [3/3] Gerando main.srec ...
"%OC%" -O srec main.axf main.srec
if %ERRORLEVEL% NEQ 0 (
    echo ERRO: falha ao gerar srec!
    goto :erro
)

echo.
echo ==============================================
echo  COMPILACAO CONCLUIDA COM SUCESSO!
echo  Agora va no Monitor Program e clique em "Load".
echo ==============================================
goto :fim

:erro
echo.
echo ==============================================
echo  ERRO NA COMPILACAO. Verifique o codigo.
echo ==============================================

:fim
endlocal
pause
