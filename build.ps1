# GCC ARM Build Script for UART project (STM32F103VC)
# Usage: powershell -ExecutionPolicy Bypass -File build.ps1

$ErrorActionPreference = "Stop"

# ============== Toolchain ==============
$GCC_PATH = "$env:USERPROFILE\gcc-arm\bin"
$CC       = "$GCC_PATH\arm-none-eabi-gcc.exe"
$AS       = "$GCC_PATH\arm-none-eabi-gcc.exe"
$LD       = "$GCC_PATH\arm-none-eabi-gcc.exe"
$OBJCOPY  = "$GCC_PATH\arm-none-eabi-objcopy.exe"
$SIZE     = "$GCC_PATH\arm-none-eabi-size.exe"

if (-not (Test-Path $CC)) {
    Write-Host "ERROR: GCC not found at $CC" -ForegroundColor Red
    exit 1
}

# ============== Project paths ==============
$PROJECT_ROOT = Split-Path -Parent $MyInvocation.MyCommand.Path
$BUILD_DIR    = "$PROJECT_ROOT\build"
$TARGET       = "UART"

# ============== MCU flags ==============
$MCU_FLAGS = @("-mcpu=cortex-m3", "-mthumb", "-mfloat-abi=soft")

# ============== Defines ==============
$DEFINES = @("-DSTM32F10X_HD", "-DUSE_STDPERIPH_DRIVER")

# ============== Include paths ==============
$INC_DIRS = @(
    "$PROJECT_ROOT\USER",
    "$PROJECT_ROOT\CORE",
    "$PROJECT_ROOT\STM32F10x_FWLib\inc",
    "$PROJECT_ROOT\SYSTEM\delay",
    "$PROJECT_ROOT\SYSTEM\sys",
    "$PROJECT_ROOT\SYSTEM\usart",
    "$PROJECT_ROOT\SYSTEM\system_control",
    "$PROJECT_ROOT\SYSTEM\rtc",
    "$PROJECT_ROOT\SYSTEM\in_flash",
    "$PROJECT_ROOT\SYSTEM\iwdg",
    "$PROJECT_ROOT\SYSTEM\activate_key",
    "$PROJECT_ROOT\SYSTEM\out_flash",
    "$PROJECT_ROOT\HARDWARE\LED",
    "$PROJECT_ROOT\HARDWARE\timers",
    "$PROJECT_ROOT\HARDWARE\ADC",
    "$PROJECT_ROOT\HARDWARE\USART2",
    "$PROJECT_ROOT\HARDWARE\USART3",
    "$PROJECT_ROOT\HARDWARE\USART4",
    "$PROJECT_ROOT\HARDWARE\USART5",
    "$PROJECT_ROOT\HARDWARE\relays",
    "$PROJECT_ROOT\HARDWARE\USR_C210",
    "$PROJECT_ROOT\HARDWARE\Parallel_Serial",
    "$PROJECT_ROOT\HARDWARE\PWM",
    "$PROJECT_ROOT\HARDWARE\ADS1220",
    "$PROJECT_ROOT\USER\rcc",
    "$PROJECT_ROOT\USER\systick"
)
$INCLUDES = $INC_DIRS | ForEach-Object { "-I`"$_`"" }

# ============== Source files ==============
$C_SOURCES = @(
    # USER
    "USER\main.c",
    "USER\stm32f10x_it.c",
    "USER\system_stm32f10x.c",
    "USER\rcc\bsp_rccclkconfig.c",
    "USER\systick\bsp_systick.c",
    # SYSTEM
    "SYSTEM\delay\delay.c",
    "SYSTEM\sys\sys.c",
    "SYSTEM\usart\usart.c",
    "SYSTEM\system_control\system_control.c",
    "SYSTEM\rtc\bsp_calendar.c",
    "SYSTEM\rtc\bsp_date.c",
    "SYSTEM\rtc\bsp_rtc.c",
    "SYSTEM\in_flash\Internal_flash.c",
    "SYSTEM\iwdg\bsp_iwdg.c",
    "SYSTEM\activate_key\activate_key.c",
    "SYSTEM\out_flash\bsp_spi_flash.c",
    # CORE
    "CORE\core_cm3.c",
    # STM32F10x FWLib
    "STM32F10x_FWLib\src\misc.c",
    "STM32F10x_FWLib\src\stm32f10x_gpio.c",
    "STM32F10x_FWLib\src\stm32f10x_rcc.c",
    "STM32F10x_FWLib\src\stm32f10x_usart.c",
    "STM32F10x_FWLib\src\stm32f10x_adc.c",
    "STM32F10x_FWLib\src\stm32f10x_bkp.c",
    "STM32F10x_FWLib\src\stm32f10x_can.c",
    "STM32F10x_FWLib\src\stm32f10x_cec.c",
    "STM32F10x_FWLib\src\stm32f10x_crc.c",
    "STM32F10x_FWLib\src\stm32f10x_dac.c",
    "STM32F10x_FWLib\src\stm32f10x_dbgmcu.c",
    "STM32F10x_FWLib\src\stm32f10x_dma.c",
    "STM32F10x_FWLib\src\stm32f10x_exti.c",
    "STM32F10x_FWLib\src\stm32f10x_flash.c",
    "STM32F10x_FWLib\src\stm32f10x_fsmc.c",
    "STM32F10x_FWLib\src\stm32f10x_i2c.c",
    "STM32F10x_FWLib\src\stm32f10x_iwdg.c",
    "STM32F10x_FWLib\src\stm32f10x_pwr.c",
    "STM32F10x_FWLib\src\stm32f10x_rtc.c",
    "STM32F10x_FWLib\src\stm32f10x_sdio.c",
    "STM32F10x_FWLib\src\stm32f10x_spi.c",
    "STM32F10x_FWLib\src\stm32f10x_tim.c",
    "STM32F10x_FWLib\src\stm32f10x_wwdg.c",
    # HARDWARE
    "HARDWARE\timers\timer.c",
    "HARDWARE\USART2\usart2.c",
    "HARDWARE\USART3\usart3.c",
    "HARDWARE\USART4\usart4.c",
    "HARDWARE\USART5\usart5.c",
    "HARDWARE\led\bsp_led.c",
    "HARDWARE\relays\BSP_RELAYS.c",
    "HARDWARE\ADC\bsp_adc.c",
    "HARDWARE\USR_C210\usr_c210.c",
    "HARDWARE\Parallel_Serial\bsp_parallel_serial.c",
    "HARDWARE\PWM\pwm_output.c",
    "HARDWARE\ADS1220\ADS1220.c"
)

$ASM_SOURCES = @(
    "CORE\startup_stm32f10x_hd_gcc.s"
)

# ============== Compiler flags ==============
$CFLAGS = $MCU_FLAGS + $DEFINES + $INCLUDES + @(
    "-Wall",
    "-fdata-sections",
    "-ffunction-sections",
    "-Os",
    "-std=gnu99",
    "-g"
)

$ASFLAGS = $MCU_FLAGS + @("-x", "assembler-with-cpp") + $DEFINES + $INCLUDES

$LDFLAGS = $MCU_FLAGS + @(
    "-specs=nano.specs",
    "-T`"$PROJECT_ROOT\STM32F103VCTx_FLASH.ld`"",
    "-Wl,-Map=`"$BUILD_DIR\$TARGET.map`",--cref",
    "-Wl,--gc-sections",
    "-lc", "-lm"
)

# ============== Build ==============
Write-Host "========================================" -ForegroundColor Cyan
Write-Host " GCC ARM Build: $TARGET" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan

if (-not (Test-Path $BUILD_DIR)) {
    New-Item -ItemType Directory -Path $BUILD_DIR | Out-Null
}

$OBJ_FILES = @()
$errorCount = 0

# Compile C files
foreach ($src in $C_SOURCES) {
    $srcPath = "$PROJECT_ROOT\$src"
    $objName = [System.IO.Path]::GetFileNameWithoutExtension($src) + ".o"
    $objPath = "$BUILD_DIR\$objName"
    $OBJ_FILES += $objPath

    Write-Host "  CC  $src" -ForegroundColor Gray
    $allArgs = $CFLAGS + @("-c", "`"$srcPath`"", "-o", "`"$objPath`"")
    $proc = Start-Process -FilePath $CC -ArgumentList $allArgs -NoNewWindow -Wait -PassThru -RedirectStandardError "$BUILD_DIR\stderr.tmp"
    if ($proc.ExitCode -ne 0) {
        $errMsg = Get-Content "$BUILD_DIR\stderr.tmp" -Raw
        Write-Host "  ERROR compiling $src" -ForegroundColor Red
        Write-Host $errMsg -ForegroundColor Yellow
        $errorCount++
    } else {
        $warns = Get-Content "$BUILD_DIR\stderr.tmp" -Raw
        if ($warns -and $warns.Trim()) {
            Write-Host $warns -ForegroundColor Yellow
        }
    }
}

# Compile ASM files
foreach ($src in $ASM_SOURCES) {
    $srcPath = "$PROJECT_ROOT\$src"
    $objName = [System.IO.Path]::GetFileNameWithoutExtension($src) + ".o"
    $objPath = "$BUILD_DIR\$objName"
    $OBJ_FILES += $objPath

    Write-Host "  AS  $src" -ForegroundColor Gray
    $allArgs = $ASFLAGS + @("-c", "`"$srcPath`"", "-o", "`"$objPath`"")
    $proc = Start-Process -FilePath $AS -ArgumentList $allArgs -NoNewWindow -Wait -PassThru -RedirectStandardError "$BUILD_DIR\stderr.tmp"
    if ($proc.ExitCode -ne 0) {
        $errMsg = Get-Content "$BUILD_DIR\stderr.tmp" -Raw
        Write-Host "  ERROR assembling $src" -ForegroundColor Red
        Write-Host $errMsg -ForegroundColor Yellow
        $errorCount++
    }
}

if ($errorCount -gt 0) {
    Write-Host "`n========================================" -ForegroundColor Red
    Write-Host " BUILD FAILED: $errorCount error(s)" -ForegroundColor Red
    Write-Host "========================================" -ForegroundColor Red
    exit 1
}

# Link
Write-Host "`n  LD  $TARGET.elf" -ForegroundColor Gray
$linkArgs = $LDFLAGS + ($OBJ_FILES | ForEach-Object { "`"$_`"" }) + @("-o", "`"$BUILD_DIR\$TARGET.elf`"")
$proc = Start-Process -FilePath $LD -ArgumentList $linkArgs -NoNewWindow -Wait -PassThru -RedirectStandardError "$BUILD_DIR\stderr.tmp"
if ($proc.ExitCode -ne 0) {
    $errMsg = Get-Content "$BUILD_DIR\stderr.tmp" -Raw
    Write-Host "  LINK ERROR" -ForegroundColor Red
    Write-Host $errMsg -ForegroundColor Yellow
    exit 1
}

# Generate HEX and BIN
Write-Host "  HEX $TARGET.hex" -ForegroundColor Gray
& $OBJCOPY -O ihex "$BUILD_DIR\$TARGET.elf" "$BUILD_DIR\$TARGET.hex"
Write-Host "  BIN $TARGET.bin" -ForegroundColor Gray
& $OBJCOPY -O binary -S "$BUILD_DIR\$TARGET.elf" "$BUILD_DIR\$TARGET.bin"

# Size
Write-Host "`n  SIZE:" -ForegroundColor Green
& $SIZE "$BUILD_DIR\$TARGET.elf"

Write-Host "`n========================================" -ForegroundColor Green
Write-Host " BUILD SUCCESS" -ForegroundColor Green
Write-Host " Output: build\$TARGET.hex" -ForegroundColor Green
Write-Host "         build\$TARGET.bin" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green
