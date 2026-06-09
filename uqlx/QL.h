#ifndef QL_H
#define QL_H
/*
 * (c) UQLX - see COPYRIGHT
 */



#define rFormat		136
#define	rPat		129
#define	mRam		140
#define mKey		144


#define ERR_MEMORY		-9600
#define ERR_ROM			-9601
#define ERR_TIME		-9602
#define ERR_DISKROM		-9603
#define ERR_RESSIZE		-9604
#define ERR_OLD_SYSTEM	-9606
#define ERR_GENERIC		-9608
#define ERR_ROM_UNKNOWN	-9611

extern Ptr			theBuffer;
/* extern WindowPtr	theScreenWindow; */
/* extern WindowPtr	regWindow; */
/* extern WindowPtr	mdvWindow; */
extern long			nChunk;
extern Cond		qlRunning;

void ExecuteChunk(long);
void RefreshDisplay(void);
//void FlushDisplay(void);  /* Now in QL_screen.h */
/*void UpdateDisplay(Rect*,Boolean); */
void DisplayIsValid(void);
void DrawRegisters(void);
void DrawMdv(void);
void DoQLkey(char,short,short);
void CheckCapsLock(short);
void CloseAllFiles(void);
/*void InvalWindow(WindowPtr);*/
void AttachDirectory(short,Cond,short,long);

#endif