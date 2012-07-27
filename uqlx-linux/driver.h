/*
 * (c) UQLX - see COPYRIGHT
 */


#define DRV_ID			0x4afc3339

#define DRV_SIZE                0x80      /* leave space for 64 bit alignment */
#define DRV_OFF                 24        /* reserved for QDOS */
#if 0
#define ALIGNOFFSET(_off_)     ((((long long)_off_+8+DRV_OFF)>>3)<<3)

#define DGET_PRIV(_f_)         (*(void**)ALIGNOFFSET(_f_))
#define DSET_PRIV(_f_,_p_)     (*(void**)ALIGNOFFSET(_f_)=_p_)
#define DGET_ID(_f_)           ( RL((w32 *)((Ptr)ALIGNOFFSET(_f_)+8)) )
#define DSET_ID(_f_,_p_)       ( WL((w32 *)((Ptr)ALIGNOFFSET(_f_)+8),_p_)  )
/*#define DGET_DRV(_f_)          ( RL((w32 *)((Ptr)ALIGNOFFSET(_f_)+12)) ) */
/*#define DSET_DRV(_f_,_i_)      ( WL((w32 *)((Ptr)ALIGNOFFSET(_f_)+12),_i_)  ) */
#else
#define ALIGNOFFSET(_xx_)      ((Ptr)(_xx_)+DRV_OFF)
#define DGET_PRIV(_f_)         (GET_POINTER((w32 *)ALIGNOFFSET(_f_)))
                               /*((void*)RL((w32*)ALIGNOFFSET(_f_)))*/
#define DSET_PRIV(_f_,_p_)     (SET_POINTER((w32 *)ALIGNOFFSET(_f_),(_p_)))
                               /*(WL((w32*)ALIGNOFFSET(_f_),(w32)(_p_)))*/
#define DGET_ID(_f_)           (RL((w32 *)(ALIGNOFFSET(_f_)+8)) )
#define DSET_ID(_f_,_p_)       (WL((w32 *)(ALIGNOFFSET(_f_)+8),_p_))
/*#define DGET_DRV(_f_)          (RL((w32 *)(ALIGNOFFSET(_f_)+12))) */
/*#define DSET_DRV(_f_,_i_)      (WL((w32 *)(ALIGNOFFSET(_f_)+12),_i_) ) */
#endif

/* pointer to free format data */
/* use with extreme care !! */
#define DRV_EPRIV(_f_)         ((w32 *)(ALIGNOFFSET(_f_)+20))

typedef union 
{
  long i;
  char *s;
} open_arg;


/* Driver name definition, similar to ut.name */
struct PARENTRY
{ /* func may be parse_separator, parse_value or parse_option */
  int (*func)(char **, /* rest name */
	      int ,    /* remaining namelen */
	      char *,  /* option string*/
	      open_arg *,  /* return values */
	      open_arg * );  /*store at*/
  char *opt;          /* option string, default value or separator */
  open_arg  *values;       /* default and/or return values for option*/
};


struct NAME_PARS
{
  char * name;
  int pcount;
  struct PARENTRY **pars;
};
 

struct DRV  
{
  w32 ref;               /* driver addr in QL space, identifier*/
  int (* init)(int, void *);       /* called before driver link-in */
  int  (* open_test)(int, char*);   /* name decoding  */
  int (* open)(int, void**);        /* dev_open  */
  void (* close)(int, void *);     /* dev_close */
  void (* io)(int, void *);        /* dev_io    */
  struct NAME_PARS *namep;        /* device name for QPAC2 or intern use */
  int slot;        /* other driver specific info */
};


int decode_name(char *, struct NAME_PARS *, open_arg *);
void io_handle(int (*io_read)(), int (*io_write)(), int (*io_pend)(),
	       void *priv);

int parse_separator(char **,int ,char *,open_arg *,open_arg *);
int parse_nseparator(char **,int ,char *,open_arg *,open_arg *);
int parse_value(char **,int ,char *,open_arg *,open_arg *);
int parse_option(char **,int ,char *,open_arg *,open_arg *);
int parse_mseparator(char **,int ,char *,open_arg *,open_arg *);

extern struct DRV Drivers[];


/* *************************************************************************** */
/* add driver local declarations here */

extern struct PARENTRY boot_pars[];
