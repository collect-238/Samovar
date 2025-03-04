#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include "driver/uart.h"
#include <Arduino.h>
//#include "app_config.h"
//#include "sdkconfig.h"
//#include "driver/gpio.h"
#include "mod_rmvk.h"
#include "Samovar.h"

#define BUF_SIZE (1024)
#define RD_BUF_SIZE (BUF_SIZE)

StaticSemaphore_t xSemaphoreBuffer;
rmvk_t rmvk;

uint8_t RMVK_cmd(const char* cmd, rmvk_res_t res) {
  size_t cmd_len;
  int len;
  cmd_len = strlen(cmd);
  char cmd_buf[cmd_len + 2];
  const char * pc = cmd_buf;
  uint8_t buf[10];//10*8=80 бит, при плохом раскладе  получим за 80*1000/9600 ~ 8,5 ms
  String s;
  if ( xSemaphore != NULL )
  {
    //Serial.print("cmd = ");
    //Serial.println(cmd);
    if ( xSemaphoreTake( xSemaphore, ( TickType_t ) ((RMVK_DEFAULT_READ_TIMEOUT * 7) / portTICK_RATE_MS)) == pdTRUE)
    {
      sprintf(cmd_buf, "%s\r", cmd);
      uart_flush(RMVK_UART);
      cmd_len = uart_write_bytes(RMVK_UART, pc, strlen(pc));
      len = uart_read_bytes(RMVK_UART, buf, sizeof(buf), RMVK_DEFAULT_READ_TIMEOUT / portTICK_RATE_MS);
      buf[len] = '\0';
      //Serial.print("buf = ");
      //Serial.println((const char *)&buf);
      xSemaphoreGive( xSemaphore );
      if (len > 0) {
        rmvk.conn = 1;
        switch (res)
        {
          case RMVK_INT:
            return atoi((const char *)&buf);
            break;
          case RMVK_OK:
            if (strncmp((const char *)&buf, "OK", 2) == 0) return 1;
            else return 0;
            break;
          case RMVK_ON:
            if (strncmp((const char *)&buf, "ON", 2) == 0) return 1;
            else return 0;
            break;
          default:
            return atoi((const char *)&buf);
            break;
        }
      }
      else
      {
        rmvk.conn = 0;
      }

      return rmvk.conn;
    }
    else
    {
      rmvk.conn = 0;
      Serial.print("UART RMVK  = BUSY");//увидим что не получили семафор
    }
  }
  return rmvk.conn;
}

uint16_t RMVK_get_in_voltge() {
  char cmd[] = "AT+VI?";
  rmvk.VI = RMVK_cmd(cmd, RMVK_INT);
  return rmvk.VI;
}

uint16_t RMVK_get_out_voltge() {
  char cmd[] = "AT+VO?";
  //    char cmd[]="AT+VI?";
  rmvk.VO = RMVK_cmd(cmd, RMVK_INT);
  return rmvk.VO;
}

uint16_t RMVK_get_store_out_voltge() {
  char cmd[] = "AT+VS?";
  rmvk.VS = RMVK_cmd(cmd, RMVK_INT);
  return rmvk.VS;
}
bool RMVK_get_state() {
  char cmd[] = "AT+ON?";
  rmvk.on = RMVK_cmd(cmd, RMVK_ON) > 0;
  return rmvk.on;
}
uint16_t RMVK_set_out_voltge(uint16_t voltge) {
  uint16_t ret;
  char cmd[20];
  if (!rmvk.on)RMVK_set_on(1);
  if (voltge > MAX_VOLTAGE)voltge = MAX_VOLTAGE;
  sprintf(cmd, "AT+VS=%03d", voltge);
  ret = RMVK_cmd(cmd, RMVK_INT);
  return ret;
}


bool RMVK_get_conn() {
  return (rmvk.conn > 0);
}

uint16_t RMVK_set_on(uint16_t state) {
  uint16_t ret;
  char cmd[20];
  if (state > 0)state = 1;
  else state = 0;
  sprintf(cmd, "AT+ON=%d", state);
  ret = RMVK_cmd(cmd, RMVK_ON);
  return ret;
}
uint16_t RMVK_select_mem(uint16_t sm) {
  int ret;
  char cmd[20];
  sprintf(cmd, "AT+SM=%d", sm);
  ret = RMVK_cmd(cmd, RMVK_OK);
  return ret;
}

void rmvk_read()
{
  rmvk.VI = RMVK_get_in_voltge();
  vTaskDelay(RMVK_READ_DELAY / portTICK_PERIOD_MS);
  rmvk.VO = RMVK_get_out_voltge();
  vTaskDelay(RMVK_READ_DELAY / portTICK_PERIOD_MS);
  rmvk.VS = RMVK_get_store_out_voltge();
  vTaskDelay(RMVK_READ_DELAY / portTICK_PERIOD_MS);
  rmvk.on = RMVK_get_state() > 0;
}

uint8_t get_rmvk_state() {
  return rmvk.on;
}
uint8_t get_rmvk_conn() {
  return rmvk.conn;
}


void rmvkFn(void* arg) {
  rmvk_read();
}

void RMVK_init(void) {
  uart_config_t uart_config = {
    .baud_rate = RMVK_BAUD_RATE,
    .data_bits = UART_DATA_8_BITS,
    .parity = UART_PARITY_DISABLE,
    .stop_bits = UART_STOP_BITS_1,
    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    .rx_flow_ctrl_thresh = 122,
  };
  uart_param_config(RMVK_UART, &uart_config);
  uart_set_pin(RMVK_UART, RMVK_TXD, RMVK_RXD, -1, -1);
  uart_driver_install(RMVK_UART, BUF_SIZE * 2, BUF_SIZE * 2,  0, NULL, 0);

  xSemaphore = xSemaphoreCreateBinaryStatic( &xSemaphoreBuffer );
  xSemaphoreGive( xSemaphore );
  RMVK_set_on(0);
  rmvk_read();
}
