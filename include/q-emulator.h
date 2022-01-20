/*
 * Copyright (c) 2022 Graeme Gregory
 *
 * SPDX: Zlib
 */

#pragma once

#pragma pack(push,1)
typedef struct {
	uint8_t		h_header[18];	/* "]!QDOS File Header" */
	uint8_t		h_res;		/* reserved */
	uint8_t		h_wordlen;	/* header length in words */
	uint8_t		f_access;	/* file access type */
	uint8_t		f_type;		/* file type */
	uint32_t	f_datalen;	/* data length */
	uint32_t	f_res;		/* reserved */
	uint8_t		m_unused[14];   /* extra mdv info */
} q_emulator_hdr;
#pragma pack(pop)

#define QEMULATOR_SHORT_HEADER 30
#define QEMULATOR_LONG_HEADER 44
