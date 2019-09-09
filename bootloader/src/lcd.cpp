#include "lcd.h"
#define DATA_BUF_SIZE 10
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
unsigned char recevie_data_buf[DATA_BUF_SIZE];
unsigned char receive_num = 0;

void clear_send_data_buf(void)
{
  memset(send_data_buf,0,sizeof(send_data_buf));
  memset(&send_data, 0, sizeof(send_data));
  send_data.head[0] = HEAD_ONE;
  send_data.head[1] = HEAD_TWO;
}

void clear_lcd_data_buf(void)
{
  receive_num = 0;
  memset(recevie_data_buf,0,sizeof(recevie_data_buf));
}


void write_lcd_data(uint8_t c)
{
  Serial3.write(c);
}

uint8_t read_lcd_data(void)
{
  uint8_t c;
  c = Serial3.read();
  return c;
}

bool lcd_data_available(void)
{
  return(Serial3.available() > 0);
}

void lcd_receive_data_clear(void)
{
  long timeout_judgment_value = millis();
  while(lcd_data_available())
  {
    read_lcd_data();
    if(millis() - timeout_judgment_value > 500)
    {
      break;
    }
  }
}

uint8_t lcd_receive_data_correct(void)
{
  if((HEAD_ONE != recevie_data_buf[0]) || (receive_num > DATA_BUF_SIZE))
  {
    return LCD_DATA_ERROR;
  }
  else if((receive_num >= 2) && (HEAD_TWO != recevie_data_buf[1]))
  {
    return LCD_DATA_ERROR;
  }
  else if((receive_num > 3) &&  receive_num > (recevie_data_buf[2]+3))
  {
    return LCD_DATA_ERROR;
  }
  else if((receive_num > 3) && receive_num == (recevie_data_buf[2]+3))
  {
    return LCD_DATA_FULL;
  }
  else
  {
    return LCD_DATA_NO_FULL;
  }
}

bool is_command_comfirm_button(void)
{
  return(recevie_data_buf[2] == 0x06 && \
        recevie_data_buf[3] == 0x83 && \
        recevie_data_buf[4] == 0x12 && \
        recevie_data_buf[5] == 0x1B && \
        recevie_data_buf[6] == 0x01 && \
        recevie_data_buf[7] == 0x00 && \
        recevie_data_buf[8] == 0x01);
}
void lcd_receive_data(void)
{
  long timeout_judgment_value = millis();
  while(true)
  {
    if(millis() - timeout_judgment_value > 2000)
    {
      break;
    }
    if(lcd_data_available())
    {
      recevie_data_buf[receive_num++] = read_lcd_data();
    }
    if(LCD_DATA_ERROR == lcd_receive_data_correct())
    {
      clear_lcd_data_buf();
      continue;
    }
    //recevie a command
    else if(LCD_DATA_FULL == lcd_receive_data_correct())
    {
      if(is_command_comfirm_button())
      {
        break;
      }
      clear_lcd_data_buf();
    }
  }
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
  clear_send_data_buf();
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
  clear_send_data_buf();
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
  lcd_send_data_int(PAGE_BASE + PRINT_FIRMWARE_UPDATE_FAIL_PAGE_CH, PAGE_ADDR);
}

void send_update_fail_page_en(void)
{
  lcd_send_data_int(PAGE_BASE + PRINT_FIRMWARE_UPDATE_FAIL_PAGE_EN, PAGE_ADDR);
}

void send_update_success_page_ch(void)
{
  lcd_send_data_int(PAGE_BASE + UPDATE_SUCCESS_PAGE_CH, PAGE_ADDR);
}

void send_update_success_page_en(void)
{
  lcd_send_data_int(PAGE_BASE + UPDATE_SUCCESS_PAGE_EN, PAGE_ADDR);
}

void show_same_firmware_page_ch(void)
{
  lcd_send_data_int(PAGE_BASE + EXCEPTION_SURE_HINT_PAGE_CH, PAGE_ADDR);
  lcd_send_data_int(SAME_FIRMWARE_ICON_NUM_CH, PRINT_STATUS_ADDR);
}

void send_firmware_exception_page_ch(void)
{
  lcd_send_data_int(PAGE_BASE + PRINT_FIRMWARE_EXCEPTION_PAGE_CH, PAGE_ADDR);
}

void send_firmware_exception_page_en(void)
{
  lcd_send_data_int(PAGE_BASE + PRINT_FIRMWARE_EXCEPTION_PAGE_EN, PAGE_ADDR);
}

void send_start_page(void)
{
  lcd_send_data_int(PAGE_BASE + MACHINE_START_UP_PAGE, PAGE_ADDR);
}

