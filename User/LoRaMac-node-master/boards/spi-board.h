/*
	20200810 by ysh
	硬件相关的SPI 接口驱动
*/
#ifndef __SPI_BOARD_H__
#define __SPI_BOARD_H__
#include <stdint.h>
void spi_io_init(void);
int spi_transdata(uint8_t *tx_data,uint16_t tx_len,uint8_t *rxbuf);

#endif // __SPI_BOARD_H__
