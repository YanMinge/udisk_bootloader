#include "lcd.h"

typedef struct lcd_data_buffer
{
  unsigned char head[2];
  unsigned char len;
  unsigned char command;
  unsigned long addr;
  unsigned long bytelen;
  unsigned short data[32];
} lcd_buffer_t;

lcd_buffer_t send_data;
unsigned char send_data_buf[MAX_SEND_BUF];

void clear_send_data_buf(void)
{
  memset(send_data_buf,0,sizeof(send_data_buf));
  memset(&send_data, 0, sizeof(send_data));
  send_data.head[0] = HEAD_ONE;
  send_data.head[1] = HEAD_TWO;
}

void write_lcd_data(uint8_t c)
{
  Serial3.write(c);
}

void lcd_send_data(void)
{
  if((send_data.head[0] == HEAD_ONE) && (send_data.head[1] == HEAD_TWO) && send_data.len >= 3)
  {
    send_data_buf[0] = send_data.head[0];
    send_data_buf[1] = send_data.head[1];
    send_data_buf[2] = send_data.len;
    send_data_buf[3] = send_data.command;
    if(WRITE_VARIABLE_ADDR == send_data.command)
    {
      send_data_buf[4] = send_data.addr >> 8;
      send_data_buf[5] = send_data.addr & 0xFF;

      for(int i = 0; i < (send_data.len - 3); i += 2)
      {
        send_data_buf[6+i] = send_data.data[i/2] >> 8;
        send_data_buf[7+i] = send_data.data[i/2] & 0xFF;
      }
    }
    //send data to uart
    for(int i = 0; i < (send_data.len + 3); i++)
      write_lcd_data(send_data_buf[i]);
  }
  clear_send_data_buf();
}

void lcd_send_data_str(const char *str, unsigned long addr)
{
  int len = strlen(str);
  if( len > 0)
  {
    send_data_buf[0] = HEAD_ONE;
    send_data_buf[1] = HEAD_TWO;
    send_data_buf[2] = 3+len;
    send_data_buf[3] = WRITE_VARIABLE_ADDR;
    send_data_buf[4] = addr >> 8;
    send_data_buf[5] = addr & 0x00FF;
    for(int i = 0;i <len ;i++)
    {
      send_data_buf[6 + i] = str[i];
    }
    for(int i = 0; i < (len + 6); i++)
    {
      write_lcd_data(send_data_buf[i]);
    }
    clear_send_data_buf();
  }
}

void lcd_send_data_int(int n, unsigned long addr)
{
  if(n > 0xFFFF)
  {
    send_data.data[0] = n >> 16;
    send_data.data[1] = n & 0xFFFF;
    send_data.len = 7;
  }
  else
  {
    send_data.data[0] = n;
    send_data.len = 5;
  }
  send_data.command = WRITE_VARIABLE_ADDR;
  send_data.addr = addr;
  lcd_send_data();
}

void send_progress_percentage(uint16_t percentage)
{
  char str[8];
  char str1[4] = {0x25,0xff,0xff,0x0};

  itoa(percentage,str,10);
  strcat(str,str1);
  lcd_send_data_str(str,PRINT_PREPARE_PERCENTAGE_ADDR);
}

void send_progressbar_status(uint16_t percentage)
{
  lcd_send_data_int(percentage/5 > 19 ? 19 : percentage/5, PRINT_PREPARE_PROGRESS_ICON_ADDR);
}

void send_update_icon(uint16_t icon_num)
{
  lcd_send_data_int(icon_num, PRINT_PREPARE_TEXT_ICON_ADDR);
}

void change_lcd_page(int16_t page_num)
{
  lcd_send_data_int(PAGE_BASE + page_num, PAGE_ADDR);
}

void send_update_fail_page_ch(void)
{
  lcd_send_data_int(PAGE_BASE + UPDATE_FAILE_PAGE_CH, PAGE_ADDR);
}

void send_update_fail_page_en(void)
{
  lcd_send_data_int(PAGE_BASE + UPDATE_FAILE_PAGE_EN, PAGE_ADDR);
}

void send_update_success_page_ch(void)
{
  lcd_send_data_int(PAGE_BASE + UPDATE_SUCCESS_PAGE_CH, PAGE_ADDR);
}

void send_update_success_page_en(void)
{
  lcd_send_data_int(PAGE_BASE + UPDATE_SUCCESS_PAGE_EN, PAGE_ADDR);
}


