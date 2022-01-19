/*
 * Copyright (c) 2022 Graeme Gregory
 *
 * SPDX: Zlib
 */

#pragma once

#pragma pack(push,1)
typedef struct {
	int32_t		f_length;  /* file length */
	uint8_t		f_access;  /* file access type */
	uint8_t		f_type;    /* file type */
	int32_t		f_datalen; /* data length */
	int32_t		f_reserved;/* Unused */
	int16_t		f_szname;  /* size of name */
	int8_t		f_name[36];/* name area */
	int32_t		f_update;  /* last update */
	int32_t		f_refdate;
	int32_t		f_backup;   /* EOD */
} qdos_file_hdr;
#pragma pack(pop)
