#ifndef __UXFILE_H
#define __UXFILE_H

void qaddpath(char *mount, char *name, int maxnlen);
int FSClose(int fd);
int eretry(void);

#endif /* __UXFILE_H */

