

#ifndef MMODES_H
#define MMODES_H

rw32     GetEA_m2(ashort) AREGP;
rw32     GetEA_m5(ashort) AREGP;
rw32     GetEA_m6(ashort) AREGP;
rw32     GetEA_m7(ashort) AREGP;
rw32     GetEA_mBad(ashort) AREGP;

rw8      GetFromEA_b_m0(void);
rw8      GetFromEA_b_mBad(void);
rw8      GetFromEA_b_m2(void);
rw8      GetFromEA_b_m3(void);
rw8      GetFromEA_b_m4(void);
rw8      GetFromEA_b_m5(void);
rw8      GetFromEA_b_m6(void); 
rw8      GetFromEA_b_m7(void);

rw16     GetFromEA_w_m0(void);
rw16     GetFromEA_w_m1(void);
rw16     GetFromEA_w_m2(void);
rw16     GetFromEA_w_m3(void);
rw16     GetFromEA_w_m4(void);
rw16     GetFromEA_w_m5(void);
rw16     GetFromEA_w_m6(void);
rw16     GetFromEA_w_m7(void);

rw32     GetFromEA_l_m0(void);
rw32     GetFromEA_l_m1(void);
rw32     GetFromEA_l_m2(void);
rw32     GetFromEA_l_m3(void);
rw32     GetFromEA_l_m4(void);
rw32     GetFromEA_l_m5(void);
rw32     GetFromEA_l_m6(void);
rw32     GetFromEA_l_m7(void);

void PutToEA_b_m0(ashort,aw8) AREGP;
void PutToEA_b_mBad(ashort,aw8) AREGP;
void PutToEA_b_m2(ashort,aw8) AREGP;
void PutToEA_b_m3(ashort,aw8) AREGP;
void PutToEA_b_m4(ashort,aw8) AREGP;
void PutToEA_b_m5(ashort,aw8) AREGP;
void PutToEA_b_m6(ashort,aw8) AREGP;
void PutToEA_b_m7(ashort,aw8) AREGP;

void PutToEA_w_m0(ashort,aw16) AREGP;
void PutToEA_w_m1(ashort,aw16) AREGP;
void PutToEA_w_m2(ashort,aw16) AREGP;
void PutToEA_w_m3(ashort,aw16) AREGP;
void PutToEA_w_m4(ashort,aw16) AREGP;
void PutToEA_w_m5(ashort,aw16) AREGP;
void PutToEA_w_m6(ashort,aw16) AREGP;
void PutToEA_w_m7(ashort,aw16) AREGP;

void PutToEA_l_m0(ashort,aw32) AREGP;
void PutToEA_l_m1(ashort,aw32) AREGP;
void PutToEA_l_m2(ashort,aw32) AREGP;
void PutToEA_l_m3(ashort,aw32) AREGP;
void PutToEA_l_m4(ashort,aw32) AREGP;
void PutToEA_l_m5(ashort,aw32) AREGP;
void PutToEA_l_m6(ashort,aw32) AREGP;
void PutToEA_l_m7(ashort,aw32) AREGP;

rw8 GetFromEA_rb_m3(ashort r);
rw32 GetFromEA_rl_m3(ashort r);
rw16 GetFromEA_rw_m3(ashort r);

Cond CondT(void);
Cond CondF(void);
Cond CondHI(void);
Cond CondLS(void);
Cond CondCC(void);
Cond CondCS(void);
Cond CondNE(void);
Cond CondEQ(void);
Cond CondVC(void);
Cond CondVS(void);
Cond CondPL(void);
Cond CondMI(void);
Cond CondGE(void);
Cond CondLT(void);
Cond CondGT(void);
Cond CondLE(void);

#endif /* MMODES_H */
