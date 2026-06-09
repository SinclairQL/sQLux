/*
 * (c) UQLX - see COPYRIGHT
 */

/* Here are the inline versions of flag-testing functions*/

static inline Cond CondT(void)
{   
  return true;
}

static inline Cond CondF(void)
{    
  return false;
}

static inline Cond CondHI(void)
{   
  return !(carry||zero);
}

static inline Cond CondLS(void)
{    
  return carry||zero;
}

static inline Cond CondCC(void)
{  
  return !carry;
}

static inline Cond CondCS(void)
{     
  return carry;
}

static inline Cond CondNE(void)
{   
  return !zero;
}

static inline Cond CondEQ(void)
{    
  return zero;
}

static inline Cond CondVC(void)
{   
  return !overflow;
}

static inline Cond CondVS(void)
{   
  return overflow;
}

static inline Cond CondPL(void)
{     
  return !negative;
}

static inline Cond CondMI(void)
{    
  return negative;
}

static inline Cond CondGE(void)
{   
  return (negative==overflow);/*  return (negative&&overflow)||(!(negative||overflow));*/
}

static inline Cond CondLT(void)
{    
  return (negative!=overflow);/*  return (negative&&(!overflow))||((!negative)&&overflow);*/
}

static inline Cond CondGT(void)
{    
  return (!zero)&&(negative==overflow);/*  return (!zero)&&((negative&&overflow)||(!(negative||overflow)));*/
}

static inline Cond CondLE(void)
{ 
  return zero||(negative!=overflow);/*  return zero||(negative&&(!overflow))||((!negative)&&overflow);*/
}
