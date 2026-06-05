/*
 * (c) 2021 Graeme Gregory <graeme@xora.org.uk>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef SQLUX_BDI_H
#define SQLUX_BDI_H

#include <stdint.h>

void SQLUXBDISelect(uint8_t d);
void SQLUXBDICommand(uint8_t command);
uint8_t SQLUXBDIStatus();
uint8_t SQLUXBDIDataRead();
void SQLUXBDIDataWrite(uint8_t d);
void SQLUXBDIAddressHigh(uint16_t bdi_addr);
void SQLUXBDIAddressLow(uint16_t bdi_addr);
uint16_t SQLUXBDISizeHigh();
uint16_t SQLUXBDISizeLow();
#endif /* SQLUX_BDI_H */
