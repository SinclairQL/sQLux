/*
 * (c) UQLX - see COPYRIGHT
 */

#ifndef QDISK_H
#define QDISK_H

#define	ERR_UNINITIALIZED_DISK	-9810
#define	ERR_NO_FILE_BLOCK		-9811

#include "QFilesPriv.h"

OSErr	AllocateDisk(int);
void	DeallocateDisk(void);
void	QDiskFlush(void);
Cond	QDiskPresent(void);
OSErr	QDiskEject(void);
void TestCloseDevs();
OSErr KillFileTail(FileNum fileNum, int nBlock);
void RewriteHeader(void);

#endif /* QDISK_H */
