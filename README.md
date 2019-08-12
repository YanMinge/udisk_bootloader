# udisk_bootloader
udisk firmware update bootloader for 3D printer


- 需要修改 `framework-arduino-lpc176x/system/CMSIS/system/LPC1768.ld`, 保留64K空间给bootloader

```/* Reserve first 64K (16 sectors * 4KB) for bootloader`

    * Reserve the last 32KB sector for EEPROM emulation

    */
   /* FLASH (rx) : ORIGIN = 0, LENGTH = (512K - 96K) */

   /* FLASH (rx) : ORIGIN = 64K, LENGTH = (512K - 96K) */

   FLASH (rx) : ORIGIN = 0K, LENGTH = (512K - 96K)

   RAM (rwx) : ORIGIN = 0x100000C8, LENGTH = (32K - 0xC8)```

- 需要修改 `framework-arduino-lpc176x/system/CMSIS/lib/usb_host/filesystems/fatfs/src/ffconf.h`, 去掉长文件名支持。
   `#define FF_USE_LFN		0`

