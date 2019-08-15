#include "Arduino.h"
#ifndef  _LCD_H
#define  _LCD_H

enum lcd_data_status : uint8_t {
  LCD_DATA_ERROR,
  LCD_DATA_FULL,
  LCD_DATA_NO_FULL,
};

#define MAX_SEND_BUF                                256
#define HEAD_ONE                                    (0x5A)
#define HEAD_TWO                                    (0xA5)
#define WRITE_VARIABLE_ADDR                         (0x82)
#define	READ_VARIABLE_ADDR	                        (0x83)
#define PRINT_PREPARE_PERCENTAGE_ADDR               0X1179
#define PRINT_STATUS_ADDR                           0X1009
#define PRINT_PREPARE_PROGRESS_ICON_ADDR            0X100A
#define PRINT_PREPARE_TEXT_ICON_ADDR                0X100B
#define UPDATE_ICON_TEXT_NUM_CH                     0X02
#define UPDATE_ICON_TEXT_NUM_EN                     0X02
#define MACHINE_START_UP_PAGE                       0
#define SAME_FIRMWARE_ICON_NUM_CH                   23
#define SAME_FIRMWARE_ICON_NUM_EN                   24
#define UPDATE_PAGE_NUM                             35
#define UPDATE_FAILE_PAGE_CH                        29
#define UPDATE_FAILE_PAGE_EN                        29
#define UPDATE_SUCCESS_PAGE_CH                      30
#define UPDATE_SUCCESS_PAGE_EN                      30
#define PRINT_FIRMWARE_EXCEPTION_PAGE_CH            31
#define PRINT_FIRMWARE_UPDATE_FAIL_PAGE_CH          32
#define PRINT_FIRMWARE_EXCEPTION_PAGE_EN            31
#define PRINT_FIRMWARE_UPDATE_FAIL_PAGE_EN          32
#define EXCEPTION_SURE_HINT_PAGE_CH                 38
#define EXCEPTION_SURE_HINT_PAGE_EN                 38

#define PAGE_BASE                                   (int)0x5A010000
#define PAGE_ADDR                                   0x0084

void lcd_receive_data_clear(void);
void lcd_send_data_int(int n, unsigned long addr);
void send_progressbar_status(uint16_t percentage);
void send_progress_percentage(uint16_t percentage);
void send_update_icon(uint16_t icon_num);
void change_lcd_page(int16_t page_num);
void send_update_fail_page_ch(void);
void send_update_fail_page_en(void);
void send_update_success_page_ch(void);
void send_update_success_page_en(void);
void show_same_firmware_page_ch(void);
void lcd_receive_data(void);
void send_firmware_exception_page_ch(void);
void send_firmware_exception_page_en(void);
void send_start_page(void);
#endif /* _LCD_H */

