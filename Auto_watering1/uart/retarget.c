
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>

int _write (int file, char *ptr, int len);

int _write (int file, char *ptr, int len)
{
  /* Write "len" of char from "ptr" to file id "fd"
   * Return number of char written.
   * Need implementing with UART here. */
	int i = 0;
	if(file == STDOUT_FILENO || file == STDERR_FILENO)
	{
	while (*ptr && (i < len)) {
		usart_send_blocking(USART1, *ptr);
		if (*ptr == '\n') {
			usart_send_blocking(USART1, '\r');
		}
		i++;
		ptr++;
	}
	  return i;
	}

  errno = EIO;
  return -1;
}
