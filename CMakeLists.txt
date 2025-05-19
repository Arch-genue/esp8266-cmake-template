cmake_minimum_required(VERSION 3.16)
project(ColorLamp C CXX ASM)
set(TARGET firmware.elf)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)

set(TOOLCHAIN ${CMAKE_SOURCE_DIR}/xtensa-toolchain)
set(PLATFORM_PATH ${CMAKE_SOURCE_DIR}/hardware)
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)

set(LIBC_PATH ${PLATFORM_PATH}/tools/sdk/libc/xtensa-lx106-elf)
set(TOOLS_PATH ${PLATFORM_PATH}/tools)
set(SDK_PATH ${PLATFORM_PATH}/tools/sdk)
set(SDK_LIB_PATH ${SDK_PATH}/lib)
set(BOOTLOADER ${PLATFORM_PATH}/bootloaders/eboot/eboot.elf)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(library)

set(CMAKE_C_COMPILER ${TOOLCHAIN}/bin/xtensa-lx106-elf-gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN}/bin/xtensa-lx106-elf-g++)
set(CMAKE_ASM_COMPILER ${TOOLCHAIN}/bin/xtensa-lx106-elf-gcc)
set(SDK_ELF2BIN ${TOOLCHAIN}/bin/xtensa-lx106-elf-objcopy)
set(TOOLCHAIN_OBJCOPY ${TOOLCHAIN}/bin/xtensa-lx106-elf-objcopy)
set(TOOLCHAIN_ADDR2LINE ${TOOLCHAIN}/bin/xtensa-lx106-elf-objcopy)
set(TOOLCHAIN_ELFSIZE ${TOOLCHAIN}/bin/xtensa-lx106-elf-size)
set(TOOLCHAIN_READELF ${TOOLCHAIN}/bin/xtensa-lx106-elf-readelf)

set (BUILD_FLOAT "-u _printf_float -u _scanf_float")
set (BUILD_SDK NONOSDK221)
set (BUILD_VTABLE_FLAGS -DVTABLES_IN_FLASH)
set (BUILD_FCPU 80000000L)

add_compile_definitions(${BUILD_SDK}=1)
add_compile_definitions(F_CPU=${BUILD_FCPU})
add_compile_definitions(LWIP_OPEN_SRC)
add_compile_definitions(TCP_MSS=536)
add_compile_definitions(LWIP_FEATURES=1)
add_compile_definitions(LWIP_IPV6=0)
add_compile_definitions(ARDUINO=10819)
add_compile_definitions(ARDUINO_ESP8266_NODEMCU)
add_compile_definitions(ARDUINO_ARCH_ESP8266)
add_compile_definitions(ARDUINO_BOARD=ESP8266_NODEMCU)
add_compile_definitions(FLASHMODE_DIO)
add_compile_definitions(ESP8266)

include_directories(
    ${PLATFORM_PATH}/cores/esp8266
    ${PLATFORM_PATH}/tools/sdk/include
    ${PLATFORM_PATH}/tools/sdk/lwip/include
    ${PLATFORM_PATH}/tools/sdk/lwip2/include
    ${PLATFORM_PATH}/tools/sdk/lwip2/include/arch
    ${PLATFORM_PATH}/tools/sdk/lwip2/include/lwip/apps
    ${PLATFORM_PATH}/tools/libc/include
    ${PLATFORM_PATH}/variants/generic
    ${TOOLCHAIN}/xtensa-lx106-elf/include/c++/4.8.2/
    ${TOOLCHAIN}/xtensa-lx106-elf/include/c++/4.8.2/xtensa-lx106-elf
)


set(CMAKE_C_OUTPUT_EXTENSION .o)
set(CMAKE_CXX_OUTPUT_EXTENSION .o)
set(CMAKE_C_FLAGS_INIT "-nostdlib")
set(CMAKE_CXX_FLAGS_INIT "-nostdlib")

set(CMAKE_C_FLAGS "-w -Os -g -Wall -Wno-unused-function -Wpointer-arith -Wundef -Werror")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-inline-functions -nostdlib")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mlongcalls -mtext-section-literals -MMD -ffunction-sections")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D__ets__ -DICACHE_FLASH -U__STRICT_ANSI__")

set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -fno-rtti -fno-exceptions -fpermissive")

set(CMAKE_EXE_LINKER_FLAGS "-g -w -Os -D__ets__ -DICACHE_FLASH -U__STRICT_ANSI__ -Wl,-static -L${LIBC_PATH}/lib -L${SDK_LIB_PATH}/${BUILD_SDK}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L${SDK_LIB_PATH} -L${SDK_PATH}/ld -T${SDK_PATH}/ld/eagle.flash.4m1m.ld")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L${TOOLCHAIN}/xtensa-lx106-elf/lib")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--no-check-sections ${BUILD_FLOAT}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -u call_user_start -u app_entry  -Wl,-EL -Wl,-wrap,system_restart_local -Wl,--wrap=spi_flash_read")
# ****************************************************************

set(CMAKE_C_LINK_EXECUTABLE "<CMAKE_C_COMPILER> <FLAGS> <CMAKE_C_LINK_FLAGS> <LINK_FLAGS>")
set(CMAKE_C_LINK_EXECUTABLE "${CMAKE_C_LINK_EXECUTABLE} -o <TARGET>")
set(CMAKE_C_LINK_EXECUTABLE "${CMAKE_C_LINK_EXECUTABLE} -Wl,--start-group <OBJECTS> <LINK_LIBRARIES> -Wl,--end-group")

set(CMAKE_CXX_LINK_EXECUTABLE "<CMAKE_CXX_COMPILER> <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS>")
set(CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_CXX_LINK_EXECUTABLE} -o <TARGET>")
set(CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_CXX_LINK_EXECUTABLE} -Wl,--start-group <OBJECTS> <LINK_LIBRARIES> -Wl,--end-group")

# ****************************************************************

# # Источники: сюда добавить все .c/.cpp и .S файлы проекта и ядра
file(GLOB_RECURSE SRC_FILES
    ${PLATFORM_PATH}/cores/esp8266/*.cpp
    ${PLATFORM_PATH}/cores/esp8266/*.c
    ${PLATFORM_PATH}/cores/esp8266/*.S
)

add_executable(firmware.elf
    ${SRC_FILES}
    ${SRC_DIR}/constants.cpp
    ${SRC_DIR}/utility.cpp
    ${SRC_DIR}/timers.cpp
    ${SRC_DIR}/palettes.cpp
    ${SRC_DIR}/volanalyzer.cpp
    ${SRC_DIR}/FFT_C.cpp
    ${SRC_DIR}/analog.cpp
    ${SRC_DIR}/main.cpp
)

LIBRARY(firmware.elf ESP8266WebServer)
LIBRARY(firmware.elf ESP8266WiFi)
LIBRARY(firmware.elf ESP8266mDNS DNSServer)
LIBRARY(firmware.elf ArduinoOTA)
LIBRARY(firmware.elf EEPROM)

# LIBRARY(firmware.elf WiFi)
# LIBRARY(firmware.elf WiFiManager)
LIBRARY(firmware.elf FastLED)
# LIBRARY(firmware.elf Time)
# LIBRARY(firmware.elf Timezone)
# # LIBRARY(firmware.elf AsyncTCP)
# LIBRARY(firmware.elf ESPAsyncTCP)
LIBRARY(firmware.elf GyverButton)
LIBRARY(firmware.elf NTPClient)


target_link_libraries(firmware.elf
    ${SDK_LIB_PATH}/libhal.a
    ${SDK_LIB_PATH}/liblwip_gcc.a 
    ${SDK_LIB_PATH}/${BUILD_SDK}/libnet80211.a
    ${SDK_LIB_PATH}/${BUILD_SDK}/libpp.a
    ${SDK_LIB_PATH}/${BUILD_SDK}/libphy.a
    ${SDK_LIB_PATH}/${BUILD_SDK}/libwpa.a
    ${SDK_LIB_PATH}/${BUILD_SDK}/libcrypto.a
    
    ${SDK_LIB_PATH}/${BUILD_SDK}/libwps.a
    ${SDK_LIB_PATH}/libbearssl.a
    ${SDK_LIB_PATH}/libaxtls.a
    ${SDK_LIB_PATH}/${BUILD_SDK}/libespnow.a
    ${SDK_LIB_PATH}/${BUILD_SDK}/libsmartconfig.a
    ${SDK_LIB_PATH}/${BUILD_SDK}/libairkiss.a
    ${SDK_LIB_PATH}/${BUILD_SDK}/libwpa2.a
    ${SDK_LIB_PATH}/libstdc++.a
    ${LIBC_PATH}/lib/libm.a
    ${LIBC_PATH}/lib/libc.a
    ${SDK_LIB_PATH}/libgcc.a
    ${SDK_LIB_PATH}/${BUILD_SDK}/libmain.a
)

set(LD_INPUT ${PLATFORM_PATH}/tools/sdk/ld/eagle.app.v6.common.ld.h)
set(LD_OUTPUT ${PLATFORM_PATH}/tools/sdk/ld/local.eagle.app.v6.common.ld)

add_custom_command(
    OUTPUT ${LD_OUTPUT}
    COMMAND ${CMAKE_C_COMPILER} -CC -E -P ${BUILD_VTABLE_FLAGS}
            -I${PLATFORM_PATH}/tools/sdk/include
            ${LD_INPUT}
            -o ${LD_OUTPUT}
    DEPENDS ${LD_INPUT}
    COMMENT "Generating ${LD_OUTPUT} from ${LD_INPUT}"
)

add_custom_target(generate_ld_script DEPENDS ${LD_OUTPUT})
add_dependencies(firmware.elf generate_ld_script)

add_custom_command(TARGET firmware.elf POST_BUILD
    COMMENT "Converting ELF to BIN and displaying size"
    COMMAND ${TOOLCHAIN_ELFSIZE} firmware.elf
    COMMAND ${TOOLS_PATH}/elf2bin.py --eboot ${BOOTLOADER} --app ${CMAKE_CURRENT_BINARY_DIR}/firmware.elf --flash_mode qio --flash_freq 40 --flash_size 4M --out ${CMAKE_CURRENT_BINARY_DIR}/firmware.bin -p ${TOOLCHAIN}/bin
    COMMAND ${TOOLS_PATH}/signing.py --mode sign --privatekey hardware/libraries/ESP8266httpUpdate/examples/httpUpdateSigned/private.key --bin ${CMAKE_CURRENT_BINARY_DIR}/firmware.bin --out ${CMAKE_CURRENT_BINARY_DIR}/firmware.bin.signed
)

add_custom_target(flash
    COMMENT "Flashing firmware to ESP8266"
    COMMAND ${TOOLS_PATH}/upload.py --port "/dev/ttyUSB0" --baud 921600 write_flash --flash_size detect 0x0 firmware.bin.signed --end
    DEPENDS firmware.elf
)
