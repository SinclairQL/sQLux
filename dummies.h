
#ifndef DUMMIES_H
#define DUMMIES_H

size_t x_read(int fildes, void *buf, size_t byt);
void debug(char *msg);
void debug2(char *msg, long n);
void debugIPC(char *msg, long n);
void BlockMoveData(void *source, void *dest,long len);
void ErrorAlert(int x);
void CustomErrorAlert(char *x);

#ifndef HAS_STPCPY
char * stpcpy(char *, const char *);
#endif /* HAS_STPCPY */

#endif /* DUMMIES_H */
