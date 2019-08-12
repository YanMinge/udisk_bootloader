#include "Arduino.h"
#ifndef  _LCD_H
#define  _LCD_H

#define MAX_SEND_BUF                                256
#define HEAD_ONE                                    (0x5A)
#define HEAD_TWO                                    (0xA5)
#define WRITE_VARIABLE_ADDR                         (0x82)
#define PRINT_PREPARE_PERCENTAGE_ADDR               0X1179
#define PRINT_PREPARE_PROGRESS_ICON_ADDR            0X100A
#define PRINT_PREPARE_TEXT_ICON_ADDR                0X100B
#define UPDATE_ICON_TEXT_NUM_CH                     0X02
#define UPDATE_ICON_TEXT_NUM_EN                     0X02
#define UPDATE_PAGE_NUM                             35
#define UPDATE_FAILE_PAGE_CH                        29
#define UPDATE_FAILE_PAGE_EN                        29
#define UPDATE_SUCCESS_PAGE_CH                      30
#define UPDATE_SUCCESS_PAGE_EN                      30

#define PAGE_BASE                                   (int)0x5A010000
#define PAGE_ADDR                                   0x0084

void send_progressbar_status(uint16_t percentage);
void send_progress_percentage(uint16_t percentage);
void send_update_icon(uint16_t icon_num);
void change_lcd_page(int16_t page_num);
void send_update_fail_page_ch(void);
void send_update_fail_page_en(void);
void send_update_success_page_ch(void);
void send_update_success_page_en(void);
#endif /* _LCD_H */

