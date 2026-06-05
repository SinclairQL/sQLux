/*
 * (c) UQLX - see COPYRIGHT
 */
#ifndef IEXL_GENERAL_H
#define IEXL_GENERAL_H

#include "QL68000.h"

void ExecuteChunk(long);
void InitialSetup(void);
void ExceptionProcessing(void);

#define RewriteEA_b(_d_)   (*((w8*)dest)=_d_)  

#if 0
static REGP1 inline void RewriteEA_b(aw8 d) 
{
  *((w8*)dest)=d;
}
#endif


#if 1
#ifdef QM_BIG_ENDIAN
#define RewriteEA_w(_d_)    (WW((Ptr)dest,_d_))
#else /* little endian */
#define RewriteEA_w(_d_)  {if (isreg) *((w16*)dest)=_d_; \
                           else   WW((Ptr)dest,_d_);}
#endif
#else
static inline void RewriteEA_w(aw16 d)
{  
#ifndef QM_BIG_ENDIAN
  if (isreg) *((w16*)dest)=d;
  else 
#endif
    WW((Ptr)dest,d);
}
#endif

#if 1
#ifdef QM_BIG_ENDIAN
#define RewriteEA_l(_d_)    (WL((Ptr)dest,_d_))
#else /* little endian */
#define RewriteEA_l(_d_)  {if (isreg) *((w32*)dest)=_d_; \
                           else   WL((Ptr)dest,_d_);}
#endif
#else
static inline void RewriteEA_l(aw32 d)
{    
#ifndef QM_BIG_ENDIAN
      if (isreg) *((w32*)dest)=d;
      else
#endif 
	WL((Ptr)dest,d);

}
#endif


     /****************************************************/

#endif /* IEXL_GENERAL_H */

