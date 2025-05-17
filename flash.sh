#--chip esp8266 --port "/dev/ttyUSB0" --baud 250000 erase_flash --end 
hardware/tools/elf2bin.py --eboot hardware/bootloaders/eboot/eboot.elf --app firmware.elf --flash_mode qio --flash_freq 40 --flash_size 4M --out firmware.bin -p /home/user/dev/ColorLamp/xtensa-toolchain/bin
# "{runtime.tools.signing}" --mode sign --privatekey "{build.source.path}/private.key" --bin "{build.path}/{build.project_name}.bin" --out "{build.path}/{build.project_name}.bin.signed"
hardware/tools/signing.py --mode sign --privatekey hardware/libraries/ESP8266httpUpdate/examples/httpUpdateSigned/private.key --bin firmware.bin --out firmware.bin.signed

hardware/tools/upload.py --port "/dev/ttyUSB0" --baud 921600  write_flash 0x0 firmware.bin.signed --end