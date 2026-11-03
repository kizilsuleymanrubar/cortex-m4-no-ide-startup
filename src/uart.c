#if !defined(SEMIHOSTING)

#include <stdio.h>
#include <stdint.h>
#include "stm32f411xe.h"


#define GPIOAEN (1U << 0)
#define UART2EN (1U << 17)
#define UART_BAUDRATE 115200
#define SYS_FREQ 16000000
#define APB1_CLK SYS_FREQ


static void uart_write(int ch);
static int uart2_read(void);
static void uart_set_baudrate(uint32_t periph_clk, uint32_t baudrate);


int __io_putchar(int ch)
{
	if (ch == '\n')
		uart_write('\r');
	uart_write(ch);

	return ch;
}


int __io_getchar(void)
{
	int ch = uart2_read();

	if (ch == '\r') {
		__io_putchar('\n');
		return '\n';
	}

	__io_putchar(ch);
	return ch;
}


void uart_init(void)
{
	/*Enable clock access to GPIOA*/
	RCC->AHB1ENR |= GPIOAEN;

	/*Set the mode of PA2 to alternate function mode*/
	GPIOA->MODER &= ~(1U << 4);
	GPIOA->MODER |= (1U << 5);

	/*Set alternate function type to AF7(UART2_TX)*/
	GPIOA->AFR[0] |= (1U << 8);
	GPIOA->AFR[0] |= (1U << 9);
	GPIOA->AFR[0] |= (1U << 10);
	GPIOA->AFR[0] &= ~(1U << 11);

	/*Set PA3 mode to alternate function mode*/
	GPIOA->MODER &= ~(1U << 6);
	GPIOA->MODER |= (1U << 7);

	/*Set PA3 alternate function type to UART_RX (AF07)*/
	GPIOA->AFR[0] |= (1U << 12);
	GPIOA->AFR[0] |= (1U << 13);
	GPIOA->AFR[0] |= (1U << 14);
	GPIOA->AFR[0] &= ~(1U << 15);

	/*Enable clock access to UART2*/
	RCC->APB1ENR |= UART2EN;

	/*Configure uart baudrate*/
	uart_set_baudrate(APB1_CLK, UART_BAUDRATE);

	/*Configure transfer direction*/
	USART2->CR1 = (USART_CR1_TE | USART_CR1_RE);

	/*Enable UART Module*/
	USART2->CR1 |= USART_CR1_UE;

	setvbuf(stdin, NULL, _IONBF, 0);
	setvbuf(stdout, NULL, _IONBF, 0);
}


static void uart_write(int ch)
{
	/*Make sure transmit data register is empty*/
	while (!(USART2->SR & USART_SR_TXE)) { }

	/*Write to transmit data register*/
	USART2->DR = (ch & 0xFF);
}


static int uart2_read(void)
{
	for (;;) {
		uint32_t sr = USART2->SR;

		if (sr & USART_SR_RXNE)
		{
			int c = (int)(USART2->DR & 0xFF);

			if (sr & (USART_SR_ORE | USART_SR_NE | USART_SR_FE | USART_SR_PE))
				continue;

			return c;
		}
	}
}


static void uart_set_baudrate(uint32_t periph_clk, uint32_t baudrate)
{
	USART2->BRR = ((periph_clk + (baudrate / 2U)) / baudrate);
}

#endif
