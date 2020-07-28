#include <stdint.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>

int _write (int fd, char *ptr, int len);
int _read (int fd, char *ptr, int len);
void _ttywrch(int ch);

int _write (int fd, char *ptr, int len)
{
  /* Write "len" of char from "ptr" to file id "fd"
   * Return number of char written.
   * Need implementing with UART here. */
	int i = 0;
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

int _read (int fd, char *ptr, int len)
{
  /* not used in this example */
  *ptr = usart_recv_blocking(USART1);
  return 1;
}

void _ttywrch(int ch) {
  /* Write one char "ch" to the default console
   * Need implementing with UART here. */
	usart_send_blocking(USART1, ch);
}

