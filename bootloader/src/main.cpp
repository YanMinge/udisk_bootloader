#include <Arduino.h>
#include "MassStorageLib.h"
#include "sbl_config.h"
#include "sbl_iap.h"
#include "lcd.h"
#include "persistent_store_api.h"
#include "utility.h"

extern "C" bool is_usb_connected(void);
extern "C" usb_error_info_type get_usb_error_info(void);
extern "C"  void set_disk_status(DSTATUS status);
extern "C" void disk_timerproc(void);

const char *firmware_file = "firmware.bin";
const char *firmware_old  = "firmware.cur";
const char *factory_mask  = "factory.mb";

#define DEBUG 1
#define EEPROM_OFFSET 2000

#define NOOP (void(0))
#define UPDATE_TEST_INDEX(VAR) NOOP
#define EEPROM_SKIP(VAR) (eeprom_index += sizeof(VAR))
#define EEPROM_START(ADDR)      int eeprom_index = ADDR; persistentStore.access_start()
#define EEPROM_FINISH()         persistentStore.access_finish()
#define EEPROM_WRITE(VAR)       do{ persistentStore.write_data(eeprom_index, (uint8_t*)&VAR, sizeof(VAR), &working_crc);              UPDATE_TEST_INDEX(VAR); }while(0)
#define EEPROM_READ(VAR)        do{ persistentStore.read_data(eeprom_index, (uint8_t*)&VAR, sizeof(VAR), &working_crc, !validating);  UPDATE_TEST_INDEX(VAR); }while(0)
#define EEPROM_READ_ALWAYS(VAR) do{ persistentStore.read_data(eeprom_index, (uint8_t*)&VAR, sizeof(VAR), &working_crc);               UPDATE_TEST_INDEX(VAR); }while(0)
#define EEPROM_ASSERT(TST,ERR)  do{ if (!(TST)) { SERIAL_ERROR_MSG(ERR); eeprom_error = true; } }while(0)

FATFS fatFS; /* File system object */
static bool validating;
static uint16_t working_crc = 0;
uint32_t firmware_size = 0;
uint32_t firmware_crc = 0;
uint32_t firmware_flash_crc = 0;
language_type languag = LAN_NULL;
bool is_factory = false;

void SysTick_Callback(void)
{
  disk_timerproc();
}

void hal_init(void)
{
  pinMode(P1_18,OUTPUT);   //HEAD HEAT L
  pinMode(P1_09,OUTPUT);   //HEAD HEAT H
  pinMode(P1_08,OUTPUT);   //DIR ENABLE
  pinMode(P1_20,OUTPUT);   //BED HEAT
  pinMode(P1_04, OUTPUT);  //EEPROM WP
  digitalWrite(P1_18,LOW);
  digitalWrite(P1_09,LOW);
  digitalWrite(P1_08,LOW);
  digitalWrite(P1_20,LOW);
  digitalWrite(P1_04, HIGH);
}

language_type get_language_type(void)
{
  uint16_t language_index = 107;
  language_type language_current_type = LAN_NULL;
  EEPROM_START(language_index);
  EEPROM_READ(language_current_type);
  EEPROM_FINISH();
  Serial.printf("current language is: %d\r\n", language_current_type);
  return language_current_type;
}

void update_failed(void)
{
  if(languag == LAN_CHINESE)
  {
    send_update_fail_page_ch();
  }
  else
  {
    send_update_fail_page_en();
  }
  while(1);
}

void firmware_exception(void)
{
  if(languag == LAN_CHINESE)
  {
    send_firmware_exception_page_ch();
  }
  else
  {
    send_firmware_exception_page_en();
  }
}

uint32_t caculate_file_crc32(FIL* fp)
{
  FRESULT rc;
  uint8_t buf[512];
  UINT br = sizeof(buf);
  uint32_t crc_value = 0xffffffff;
  while(br == sizeof(buf))
  {
    if((rc = f_read(fp, buf, sizeof(buf), &br)) != FR_OK)
    {
      Serial.printf("Unable to read file: %s (%d)\r\n", firmware_file, rc);
      f_close(fp);
      return 0;
    }
    crc_value = crc32(crc_value, buf, br, 0);
  }
  crc_value = crc_value^0xFFFFFFFF;
  Serial.printf("caculate file crc32: 0x%x\r\n", crc_value);
  return crc_value;
}

uint32_t write_file_to_flash(FIL* fp)
{
  FRESULT rc;
  uint8_t buf[512];
  UINT br = sizeof(buf);
  uint32_t address = USER_FLASH_START;
  Serial.printf("write file %s to flash\r\n", firmware_file);
  f_lseek(fp, 0);
  while(br == sizeof(buf))
  {
    if((rc = f_read(fp, buf, sizeof(buf), &br)) != FR_OK)
    {
      Serial.printf("Unable to read file: %s (%d)\r\n", firmware_file, rc);
      f_close(fp);
      return 0;
    }
    write_flash((uint32_t *) address, (char *)buf, sizeof(buf));
    uint16_t percentage = int(((address - USER_FLASH_START) * 1.0 /fp->obj.objsize) * 100);
    change_lcd_page(UPDATE_PAGE_NUM);
    if(languag == LAN_CHINESE)
    {
      send_update_icon(UPDATE_ICON_TEXT_NUM_CH);
    }
    else
    {
      send_update_icon(UPDATE_ICON_TEXT_NUM_EN);
    }
    send_progressbar_status(percentage);
    send_progress_percentage(percentage);
    address += br;
  }
  return address - USER_FLASH_START;
}

uint32_t caculate_flash_crc32(uint32_t file_size)
{
  uint32_t crc_value = 0xffffffff;
  uint8_t *pmem= (uint8_t *)USER_FLASH_START;
  crc_value = crc32(crc_value, pmem, file_size, 1);
  Serial.printf("caculate flash crc32: 0x%x\r\n", crc_value);
  return crc_value;
}

void save_firmware_data(uint32_t file_size, uint32_t crc)
{
  EEPROM_START(EEPROM_OFFSET);
  EEPROM_WRITE(file_size);
  EEPROM_WRITE(crc);
  EEPROM_FINISH();
}

void load_firmware_data(void)
{
  EEPROM_START(EEPROM_OFFSET);
  EEPROM_READ(firmware_size);
  EEPROM_READ(firmware_crc);
  EEPROM_FINISH();
  if(firmware_size >= 0x64000)
  {
    firmware_size = 0x64000;
  }
  Serial.printf("firmware_size:(%ud), firmware_crc(0x%x)\r\n", firmware_size, firmware_crc);
}

// this seems to fix an issue with handoff after poweroff
// found here http://knowledgebase.nxp.trimm.net/showthread.php?t=2869
typedef void __attribute__((noreturn))(*exec)();

static void boot(uint32_t a)
{
  uint32_t *start;

  __set_MSP(*(uint32_t *)USER_FLASH_START);
  start = (uint32_t *)(USER_FLASH_START + 4);
  ((exec)(*start))();
}

static uint32_t delay_loop(uint32_t count)
{
  volatile uint32_t j, del;
  for(j=0; j<count; ++j)
  {
    del=j; // volatiles, so the compiler will not optimize the loop
  }
  return del;
}

static void new_execute_user_code(void)
{
  uint32_t addr=(uint32_t)USER_FLASH_START;
  // delay
  delay_loop(3000000);
  // relocate vector table
  SCB->VTOR = (addr & 0x1FFFFF80);
  // switch to RC generator
  LPC_SC->PLL0CON = 0x1; // disconnect PLL0
  LPC_SC->PLL0FEED = 0xAA;
  LPC_SC->PLL0FEED = 0x55;
  while (LPC_SC->PLL0STAT&(1<<25));
  LPC_SC->PLL0CON = 0x0;    // power down
  LPC_SC->PLL0FEED = 0xAA;
  LPC_SC->PLL0FEED = 0x55;
  while (LPC_SC->PLL0STAT&(1<<24));
  // disable PLL1
  LPC_SC->PLL1CON   = 0;
  LPC_SC->PLL1FEED  = 0xAA;
  LPC_SC->PLL1FEED  = 0x55;
  while (LPC_SC->PLL1STAT&(1<<9));

  LPC_SC->FLASHCFG &= 0x0fff;  // This is the default flash read/write setting for IRC
  LPC_SC->FLASHCFG |= 0x5000;
  LPC_SC->CCLKCFG = 0x0;     //  Select the IRC as clk
  LPC_SC->CLKSRCSEL = 0x00;
  LPC_SC->SCS = 0x00;       // not using XTAL anymore
  delay_loop(1000);
  // reset pipeline, sync bus and memory access
  __asm (
       "dmb\n"
       "dsb\n"
       "isb\n"
      );
  boot(addr);
}

bool check_firmware_integrity(void)
{
  if(firmware_flash_crc != firmware_crc)
  {
    Serial.printf("Firmware corruption\r\n");
    firmware_exception();
    while(true);
    return false;
  }
  else
  {
    Serial.printf("Firmware no corruption\r\n");
    return true;
  }
}

void lcd_update_init(void)
{
  change_lcd_page(UPDATE_PAGE_NUM);
  if(languag == LAN_CHINESE)
  {
    send_update_icon(UPDATE_ICON_TEXT_NUM_CH);
  }
  else
  {
    send_update_icon(UPDATE_ICON_TEXT_NUM_EN);
  }
  send_progressbar_status(0);
  send_progress_percentage(0);
}

void check_udisk_firmware(void)
{
  FRESULT rc;
  FIL file_obj;
  FILINFO fno;
  uint32_t file_crc32;
  uint32_t flash_crc32;
  rc = f_mount(&fatFS, "/" , 0);     /* Register volume work area (never fails) */
  if(rc != FR_OK)
  {
    Serial.printf("udisk is not insert!\r\n");
    check_firmware_integrity();
    return;
  }
  rc = f_open(&file_obj, factory_mask, FA_READ);
  if(rc == FR_OK)
  {
    Serial.printf("Enter factory burning mode\r\n");
    is_factory = true;
  }
  if(!f_stat(firmware_file, &fno))
  {
    rc = f_open(&file_obj, firmware_file, FA_READ);
    if(rc != FR_OK)
    {
      Serial.printf("Unable to open file: %s (%d)\r\n", firmware_file, rc);
      return;
    }
    else
    {
      file_crc32 = caculate_file_crc32(&file_obj);
      if((is_factory == false) && (firmware_flash_crc == file_crc32))
      {
        Serial.printf("This is the same firmware\r\n");
        delay(100);
        lcd_receive_data_clear();
        show_same_firmware_page_ch();
        lcd_receive_data();
        send_start_page();
        return;
      }
      lcd_update_init();
      uint32_t write_szie = write_file_to_flash(&file_obj);
      Serial.printf("write_szie = %d\n", write_szie);
      if (write_szie == file_obj.obj.objsize)
      {
        Serial.printf("Complete!\n");
        f_close(&file_obj);
        flash_crc32 = caculate_flash_crc32(write_szie);
        if(flash_crc32 == file_crc32)
        {
          Serial.printf("Firmware upgrade succeeded!\n");
          save_firmware_data(write_szie, file_crc32);
          if(languag == LAN_CHINESE)
          {
            send_update_success_page_ch();
          }
          else
          {
            send_update_success_page_en();
          }
          if(is_factory == false)
          {
            rc = f_unlink(firmware_old);
            rc = f_rename(firmware_file, firmware_old);
            rc = f_unlink(firmware_file);
          }
        }
        else
        {
          update_failed();
          Serial.printf("Firmware upgrade failed!!\n");
        }
      }
      else
      {
        update_failed();
        Serial.printf("Firmware upgrade failed!!\n");
      }
    }
  }
  else
  {
    check_firmware_integrity();
  }
}

void setup()
{
  Serial.begin(115200);
  Serial3.begin(115200);
  SetupHardware();
  hal_init();
  Serial.println("Bootloader Start\r\n");
  is_factory = false;
  languag = get_language_type();
  load_firmware_data();
  firmware_flash_crc = caculate_flash_crc32(firmware_size);
  check_udisk_firmware();
  Serial.println("Load user program\r\n");
  new_execute_user_code();
  Serial.printf("This should never happen\n");
  for (volatile int i = (1<<18);i;i--);
  NVIC_SystemReset();
}

void loop()
{
  Serial.println("bootloader failed!!!");
}
