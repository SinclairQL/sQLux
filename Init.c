/*
 * (c) UQLX - see COPYRIGHT
 */


#ifndef IE_XL

#include "QL68000.h"

void UseIPC(void);
void ReadIPC(void);
void WriteIPC(void);

void ReadMdvSector(void);
void WriteMdvSector(void);
void VerifyMdvSector(void);
void ReadMdvHeader(void);

void DoOpt1(void);
void DoOpt2(void);

void FastStartup(void);
void MdvIO(void);
void MdvOpen(void);
void MdvClose(void);
void MdvSlaving(void);
void MdvFormat(void);

void ori_b(void);
void ori_w(void);
void ori_l(void);
void ori_to_ccr(void);
void ori_to_sr(void);
void btst_d(void);
void bset_d(void);
void bclr_d(void);
void bchg_d(void);
void movep_w_rm(void);
void movep_l_rm(void);
void movep_w_mr(void);
void movep_l_mr(void);
void andi_b(void);
void andi_w(void);
void andi_l(void);
void andi_to_ccr(void);
void andi_to_sr(void);
void subi_b(void);
void subi_w(void);
void subi_l(void);
void addi_b(void);
void addi_w(void);
void addi_l(void);
void btst_s(void);
void bset_s(void);
void bclr_s(void);
void bchg_s(void);
void eori_b(void);
void eori_w(void);
void eori_l(void);
void eori_to_ccr(void);
void eori_to_sr(void);
void cmpi_b(void);
void cmpi_w(void);
void cmpi_l(void);
void move_b(void);
void move_b_to_dn(void);
void move_b_from_dn(void);
void move_l(void);
void move_l_to_dn(void);
void move_l_from_dn(void);
void movea_l(void);
void movea_l_an(void);
void move_w(void);
void move_w_to_dn(void);
void move_w_from_dn(void);
void movea_w(void);
void move_b_reg(void);
void move_w_reg(void);
void move_l_reg(void);
void negx_b(void);
void negx_w(void);
void negx_l(void);
void move_from_sr(void);
void chk(void);
void lea(void);
void clr_b(void);
void clr_w(void);
void clr_l(void);
void neg_b(void);
void neg_w(void);
void neg_l(void);
void move_to_ccr(void);
void not_b(void);
void not_w(void);
void not_l(void);
void move_to_sr(void);
void nbcd(void);
void pea(void);
void swap(void);
void ext_w(void);
void movem_save_w(void);
void movem_save_l(void);
void ext_l(void);
void tst_b(void);
void tst_w(void);
void tst_l(void);
void tas(void);
void illegal(void);
void movem_load_w(void);
void movem_load_l(void);
void trap(void);
void trap0(void);
void trap1(void);
void trap2(void);
void trap3(void);
void trap4(void);
void link(void);
void unlk(void);
void move_to_usp(void);
void move_from_usp(void);
void reset(void);
void nop(void);
void stop(void);
void rte(void);
void rts(void);
void trapv(void);
void rtr(void);
void jsr(void);
void jsr_displ(void);
void jmp(void);
void addq_b(void);
void addq_w(void);
void addq_l(void);
void addq_an(void);
void addq_4_an(void);
void scc(void);
void st(void);
void sf(void);
void dbcc(void);
void dbf(void);
void subq_b(void);
void subq_w(void);
void subq_l(void);
void subq_an(void);
void subq_4_an(void);
void bcc_s(void);
void bcc_bad(void);
void bne_s(void);
void beq_s(void);
void bpl_s(void);
void bmi_s(void);
void bge_s(void);
void blt_s(void);
void bgt_s(void);
void ble_s(void);
void bcs_s(void);
void bccc_s(void);
void bcc_l(void);
void beq_l(void);
void bne_l(void);
void bra_s(void);
void bra_l(void);
void bsr(void);
void moveq(void);
void or_b_dn(void);
void or_w_dn(void);
void or_l_dn(void);
void or_b_ea(void);
void or_w_ea(void);
void or_l_ea(void);
void divu(void);
void sbcd(void);
void divs(void);
void sub_b_dn(void);
void sub_w_dn(void);
void sub_l_dn(void);
void sub_b_ea(void);
void sub_w_ea(void);
void sub_l_ea(void);
void sub_w_an(void);
void sub_l_an(void);
void subx_b_r(void);
void subx_w_r(void);
void subx_l_r(void);
void subx_b_m(void);
void subx_w_m(void);
void subx_l_m(void);
void cmp_b(void);
void cmp_b_dan(void);
void cmp_w(void);
void cmp_l(void);
void cmp_b_dn(void);
void cmp_w_dn(void);
void cmp_l_dn(void);
void cmpa_w(void);
void cmpa_l(void);
void cmpa_l_an(void);
void eor_b(void);
void eor_w(void);
void eor_l(void);
void cmpm_b(void);
void cmpm_w(void);
void cmpm_l(void);
void and_b_dn(void);
void and_w_dn(void);
void and_l_dn(void);
void and_l_dn_dn(void);
void and_b_ea(void);
void and_w_ea(void);
void and_l_ea(void);
void mulu(void);
void abcd(void);
void exg_d(void);
void exg_a(void);
void exg_ad(void);
void muls(void);
void add_b_dn(void);
void add_w_dn(void);
void add_l_dn(void);
void add_b_dn_dn(void);
void add_w_dn_dn(void);
void add_l_dn_dn(void);
void add_b_ea(void);
void add_w_ea(void);
void add_l_ea(void);
void add_w_an(void);
void add_l_an(void);
void add_w_an_dn(void);
void add_l_an_dn(void);
void addx_b_r(void);
void addx_w_r(void);
void addx_l_r(void);
void addx_b_m(void);
void addx_w_m(void);
void addx_l_m(void);
void lsr_b_i(void);
void lsl_b_i(void);
void lsr1_b(void);
void lsl1_b(void);
void lsr_w_i(void);
void lsl_w_i(void);
void lsr1_w(void);
void lsl1_w(void);
void lsr_l_i(void);
void lsl_l_i(void);
void lsr1_l(void);
void lsl1_l(void);
void lsl2_l(void);
void lsr_b_r(void);
void lsl_b_r(void);
void lsr_w_r(void);
void lsl_w_r(void);
void lsr_l_r(void);
void lsl_l_r(void);
void asr_b_i(void);
void asl_b_i(void);
void asr_w_i(void);
void asl_w_i(void);
void asr_l_i(void);
void asl_l_i(void);
void asr_b_r(void);
void asl_b_r(void);
void asr_w_r(void);
void asl_w_r(void);
void asr_l_r(void);
void asl_l_r(void);
void roxr_b_i(void);
void roxl_b_i(void);
void roxr_w_i(void);
void roxl_w_i(void);
void roxr_l_i(void);
void roxl_l_i(void);
void roxr_b_r(void);
void roxl_b_r(void);
void roxr_w_r(void);
void roxl_w_r(void);
void roxr_l_r(void);
void roxl_l_r(void);
void ror_b_i(void);
void rol_b_i(void);
void ror_w_i(void);
void rol_w_i(void);
void ror_l_i(void);
void rol_l_i(void);
void ror_b_r(void);
void rol_b_r(void);
void ror_w_r(void);
void rol_w_r(void);
void ror_l_r(void);
void rol_l_r(void);
void asr_m(void);
void asl_m(void);
void lsr_m(void);
void lsl_m(void);
void roxr_m(void);
void roxl_m(void);
void ror_m(void);
void rol_m(void);
void code1010(void);
void code1111(void);
void InvalidCode(void);
#endif  /* IE_XL */

#ifdef IE_XL
#define LR  &&
#else 
#define LR
#endif

/*int p_stt=0;*/

#ifndef IE_XL
void SetTable(void (**itable)(void),
	      char *s,void (*f)(void))
{       long c;
        long m,m0;
        Cond multiplo=false;
        register void (**p)(void);

        short i,c1,c2;

	/*printf("SetTable: insns %s, *code %x\n",s,f);*/

        m=0x08000;
        c=0;
        for(i=0;i<16;i++)
        {       if(s[i]=='1') c+=m;
                if(s[i]=='x') multiplo=true;
                m>>=1;
        }
        if(!multiplo)
        {       itable[c]=f;
                return;
        }
        m0=1;
        i=15;
        while(s[i]!='x')
        {       m0<<=1;
                i--;
        }
        c1=1;
        while(s[i--]=='x') c1<<=1;
        while(i&&s[i]!='x') i--;
        if(i)
        {       c2=2;
                m=0x08000>>i;
                while(s[--i]=='x') c2<<=1;
                while(c2--)
                {       p=itable+c;
                        i=c1;
                        while(i--)
                        {       *p=f;
			/*if (p_stt) printf("setting %d\n",p-itable);*/
                                p+=m0;
                        }
                        c+=m;
                }
        }
        else
        {       p=itable+c;
                while(c1--)
                {       *p=f;
		/*if (p_stt) printf("setting %d\n",p-itable);*/
                        p+=m0;
                }
        }
}
#endif

#ifndef IE_XL
void SetInvalEntries(void (**itable)(void),void *code)
{
  long i;
  
  for(i=8;i<61440;i++) itable[i]= code;
}
#endif


#ifndef IE_XL
static void SetTabEntries(void (**itable)(void))
#endif

#if defined(IE_XL_II) || (!defined(IE_XL))
{   

	SetInvalEntries(itable, LR InvalidCode);


/* la procedura =E8 spezzata in pi=F9 subroutines per ovviare all'errore
   Too many external references from one function (internal overflow)
   segnalato dal compilatore MPW */


  SetTable(itable, "0000000000xxxxxx", LR ori_b);
        SetTable(itable, "0000000001xxxxxx", LR ori_w);
        SetTable(itable, "0000000010xxxxxx", LR ori_l);
        SetTable(itable, "0000000000111100", LR ori_to_ccr);
        SetTable(itable, "0000000001111100", LR ori_to_sr);
        SetTable(itable, "0000xxx100xxxxxx", LR btst_d);
        SetTable(itable, "0000xxx101xxxxxx", LR bchg_d);
        SetTable(itable, "0000xxx110xxxxxx", LR bclr_d);
        SetTable(itable, "0000xxx111xxxxxx", LR bset_d);
        SetTable(itable, "0000xxx100001xxx", LR movep_w_mr);
        SetTable(itable, "0000xxx101001xxx", LR movep_l_mr);
        SetTable(itable, "0000xxx110001xxx", LR movep_w_rm);
        SetTable(itable, "0000xxx111001xxx", LR movep_l_rm);
        SetTable(itable, "0000001000xxxxxx", LR andi_b);
        SetTable(itable, "0000001001xxxxxx", LR andi_w);
        SetTable(itable, "0000001010xxxxxx", LR andi_l);
        SetTable(itable, "0000001000111100", LR andi_to_ccr);
        SetTable(itable, "0000001001111100", LR andi_to_sr);
        SetTable(itable, "0000010000xxxxxx", LR subi_b);
        SetTable(itable, "0000010001xxxxxx", LR subi_w);
        SetTable(itable, "0000010010xxxxxx", LR subi_l);
        SetTable(itable, "0000011000xxxxxx", LR addi_b);
        SetTable(itable, "0000011001xxxxxx", LR addi_w);
        SetTable(itable, "0000011010xxxxxx", LR addi_l);
        SetTable(itable, "0000100000xxxxxx", LR btst_s);
        SetTable(itable, "0000100001xxxxxx", LR bchg_s);
        SetTable(itable, "0000100010xxxxxx", LR bclr_s);
        SetTable(itable, "0000100011xxxxxx", LR bset_s);
        SetTable(itable, "0000101000xxxxxx", LR eori_b);
        SetTable(itable, "0000101001xxxxxx", LR eori_w);
        SetTable(itable, "0000101010xxxxxx", LR eori_l);
        SetTable(itable, "0000101000111100", LR eori_to_ccr);
        SetTable(itable, "0000101001111100", LR eori_to_sr);
        SetTable(itable, "0000110000xxxxxx", LR cmpi_b);
        SetTable(itable, "0000110001xxxxxx", LR cmpi_w);
        SetTable(itable, "0000110010xxxxxx", LR cmpi_l);
        SetTable(itable, "0001xxxxxxxxxxxx", LR move_b);
        SetTable(itable, "0001xxxxxx000xxx", LR move_b_from_dn);
        SetTable(itable, "0001xxx000xxxxxx", LR move_b_to_dn);
        SetTable(itable, "0001xxx000000xxx", LR move_b_reg);
        SetTable(itable, "0010xxxxxxxxxxxx", LR move_l);
        SetTable(itable, "0010xxxxxx000xxx", LR move_l_from_dn);
        SetTable(itable, "0010xxx000xxxxxx", LR move_l_to_dn);
        SetTable(itable, "0010xxx000000xxx", LR move_l_reg);
        SetTable(itable, "0010xxx001xxxxxx", LR movea_l);
        SetTable(itable, "0010xxx001001xxx", LR movea_l_an);
        SetTable(itable, "0011xxxxxxxxxxxx", LR move_w);
        SetTable(itable, "0011xxxxxx000xxx", LR move_w_from_dn);
        SetTable(itable, "0011xxx000xxxxxx", LR move_w_to_dn);
        SetTable(itable, "0011xxx000000xxx", LR move_w_reg);
        SetTable(itable, "0011xxx001xxxxxx", LR movea_w);
        SetTable(itable, "0100000000xxxxxx", LR negx_b);
        SetTable(itable, "0100000001xxxxxx", LR negx_w);
        SetTable(itable, "0100000010xxxxxx", LR negx_l);
        SetTable(itable, "0100000011xxxxxx", LR move_from_sr);
        SetTable(itable, "0100xxx110xxxxxx", LR chk);
        SetTable(itable, "0100xxx111xxxxxx", LR lea);
	/**/
  SetTable(itable, "0100001000xxxxxx", LR clr_b);
        SetTable(itable, "0100001001xxxxxx", LR clr_w);
        SetTable(itable, "0100001010xxxxxx", LR clr_l);
        SetTable(itable, "0100010000xxxxxx", LR neg_b);
        SetTable(itable, "0100010001xxxxxx", LR neg_w);
        SetTable(itable, "0100010010xxxxxx", LR neg_l);
        SetTable(itable, "0100010011xxxxxx", LR move_to_ccr);
        SetTable(itable, "0100011000xxxxxx", LR not_b);
        SetTable(itable, "0100011001xxxxxx", LR not_w);
        SetTable(itable, "0100011010xxxxxx", LR not_l);
        SetTable(itable, "0100011011xxxxxx", LR move_to_sr);
        SetTable(itable, "0100100000xxxxxx", LR nbcd);
        SetTable(itable, "0100100001xxxxxx", LR pea);
        SetTable(itable, "0100100001000xxx", LR swap);
        SetTable(itable, "0100100010xxxxxx", LR movem_save_w);
        SetTable(itable, "0100100010000xxx", LR ext_w);
        SetTable(itable, "0100100011xxxxxx", LR movem_save_l);
        SetTable(itable, "0100100011000xxx", LR ext_l);
        SetTable(itable, "0100101000xxxxxx", LR tst_b);
        SetTable(itable, "0100101001xxxxxx", LR tst_w);
        SetTable(itable, "0100101010xxxxxx", LR tst_l);
        SetTable(itable, "0100101011xxxxxx", LR tas);
        SetTable(itable, "0100101011111xxx", LR InvalidCode);
        SetTable(itable, "010010101111100x", LR tas);
        SetTable(itable, "0100101011111100", LR illegal);
        SetTable(itable, "0100110010xxxxxx", LR movem_load_w);
        SetTable(itable, "0100110011xxxxxx", LR movem_load_l);
        SetTable(itable, "010011100100xxxx", LR trap);
#ifndef IE_XL_II
	SetTable(itable, "0100111001000000", LR trap0);
	SetTable(itable, "0100111001000001", LR trap1);
	SetTable(itable, "0100111001000010", LR trap2);
	SetTable(itable, "0100111001000011", LR trap3);
#if 0
	SetTable(itable, "0100111001000100", LR trap4); 
#endif 
#endif
        SetTable(itable, "0100111001010xxx", LR link);
        SetTable(itable, "0100111001011xxx", LR unlk);
        SetTable(itable, "0100111001100xxx", LR move_to_usp);
        SetTable(itable, "0100111001101xxx", LR move_from_usp);
        SetTable(itable, "0100111001110000", LR reset);
        SetTable(itable, "0100111001110001", LR nop);
        SetTable(itable, "0100111001110010", LR stop);
        SetTable(itable, "0100111001110011", LR rte);
        SetTable(itable, "0100111001110101", LR rts);
        SetTable(itable, "0100111001110110", LR trapv);
        SetTable(itable, "0100111001110111", LR rtr);
        SetTable(itable, "0100111010xxxxxx", LR jsr);
        SetTable(itable, "0100111010111010", LR jsr_displ);
        SetTable(itable, "0100111011xxxxxx", LR jmp);
        SetTable(itable, "0101xxx000xxxxxx", LR addq_b);
        SetTable(itable, "0101xxx001xxxxxx", LR addq_w);
        SetTable(itable, "0101xxx010xxxxxx", LR addq_l);
        SetTable(itable, "0101xxx001001xxx", LR addq_an);
        SetTable(itable, "0101xxx010001xxx", LR addq_an);
        SetTable(itable, "0101100001001xxx", LR addq_4_an);
        SetTable(itable, "0101100010001xxx", LR addq_4_an);
	/**/
       SetTable(itable, "0101xxxx11xxxxxx", LR scc);
        SetTable(itable, "0101000011xxxxxx", LR st);
        SetTable(itable, "0101000111xxxxxx", LR sf);
        SetTable(itable, "0101xxxx11001xxx", LR dbcc);
        SetTable(itable, "0101000111001xxx", LR dbf);
        SetTable(itable, "0101xxx100xxxxxx", LR subq_b);
        SetTable(itable, "0101xxx101xxxxxx", LR subq_w);
        SetTable(itable, "0101xxx110xxxxxx", LR subq_l);
        SetTable(itable, "0101xxx101001xxx", LR subq_an);
        SetTable(itable, "0101xxx110001xxx", LR subq_an);
        SetTable(itable, "0101100101001xxx", LR subq_4_an);
        SetTable(itable, "0101100110001xxx", LR subq_4_an);
        SetTable(itable, "0110xxxxxxxxxxx0", LR bcc_s);
        SetTable(itable, "0110xxxxxxxxxxx1", LR bcc_bad);
        SetTable(itable, "01100110xxxxxxx0", LR bne_s);
        SetTable(itable, "01101010xxxxxxx0", LR bpl_s);
        SetTable(itable, "01101011xxxxxxx0", LR bmi_s);
        SetTable(itable, "01101100xxxxxxx0", LR bge_s);
        SetTable(itable, "01101101xxxxxxx0", LR blt_s);
        SetTable(itable, "01101110xxxxxxx0", LR bgt_s);
        SetTable(itable, "01101111xxxxxxx0", LR ble_s);
        SetTable(itable, "01100101xxxxxxx0", LR bcs_s);
        SetTable(itable, "01100100xxxxxxx0", LR bccc_s);
        SetTable(itable, "01100111xxxxxxx0", LR beq_s);
        SetTable(itable, "0110xxxx00000000", LR bcc_l);
        SetTable(itable, "0110011100000000", LR beq_l);
        SetTable(itable, "0110011000000000", LR bne_l);
        SetTable(itable, "01100000xxxxxxx0", LR bra_s);
        SetTable(itable, "0110000000000000", LR bra_l);
        SetTable(itable, "01100001xxxxxxxx", LR bsr);
        SetTable(itable, "0111xxx0xxxxxxxx", LR moveq);
        SetTable(itable, "1000xxx000xxxxxx", LR or_b_dn);
        SetTable(itable, "1000xxx001xxxxxx", LR or_w_dn);
        SetTable(itable, "1000xxx010xxxxxx", LR or_l_dn);
        SetTable(itable, "1000xxx100xxxxxx", LR or_b_ea);
        SetTable(itable, "1000xxx101xxxxxx", LR or_w_ea);
        SetTable(itable, "1000xxx110xxxxxx", LR or_l_ea);
        SetTable(itable, "1000xxx011xxxxxx", LR divu);
        SetTable(itable, "1000xxx10000xxxx", LR sbcd);
        SetTable(itable, "1000xxx111xxxxxx", LR divs);
        SetTable(itable, "1001xxx000xxxxxx", LR sub_b_dn);
        SetTable(itable, "1001xxx001xxxxxx", LR sub_w_dn);
        SetTable(itable, "1001xxx010xxxxxx", LR sub_l_dn);
        SetTable(itable, "1001xxx100xxxxxx", LR sub_b_ea);
        SetTable(itable, "1001xxx101xxxxxx", LR sub_w_ea);
        SetTable(itable, "1001xxx110xxxxxx", LR sub_l_ea);
        SetTable(itable, "1001xxx011xxxxxx", LR sub_w_an);
        SetTable(itable, "1001xxx111xxxxxx", LR sub_l_an);
        SetTable(itable, "1001xxx100000xxx", LR subx_b_r);
        SetTable(itable, "1001xxx101000xxx", LR subx_w_r);
        SetTable(itable, "1001xxx110000xxx", LR subx_l_r);
        SetTable(itable, "1001xxx100001xxx", LR subx_b_m);
        SetTable(itable, "1001xxx101001xxx", LR subx_w_m);
        SetTable(itable, "1001xxx110001xxx", LR subx_l_m);
        SetTable(itable, "1011xxx000xxxxxx", LR cmp_b);
        SetTable(itable, "1011xxx001xxxxxx", LR cmp_w);
        SetTable(itable, "1011xxx010xxxxxx", LR cmp_l);
        SetTable(itable, "1011xxx000000xxx", LR cmp_b_dn);
        SetTable(itable, "1011xxx000101xxx", LR cmp_b_dan);
        SetTable(itable, "1011xxx001000xxx", LR cmp_w_dn);
        SetTable(itable, "1011xxx010000xxx", LR cmp_l_dn);
        SetTable(itable, "1011xxx011xxxxxx", LR cmpa_w);
        SetTable(itable, "1011xxx111xxxxxx", LR cmpa_l);
        SetTable(itable, "1011xxx111001xxx", LR cmpa_l_an);
        SetTable(itable, "1011xxx100xxxxxx", LR eor_b);
        SetTable(itable, "1011xxx101xxxxxx", LR eor_w);
        SetTable(itable, "1011xxx110xxxxxx", LR eor_l);
	/**/
       SetTable(itable, "1011xxx100001xxx", LR cmpm_b);
        SetTable(itable, "1011xxx101001xxx", LR cmpm_w);
        SetTable(itable, "1011xxx110001xxx", LR cmpm_l);
        SetTable(itable, "1100xxx000xxxxxx", LR and_b_dn);
        SetTable(itable, "1100xxx001xxxxxx", LR and_w_dn);
        SetTable(itable, "1100xxx010xxxxxx", LR and_l_dn);
        SetTable(itable, "1100xxx010000xxx", LR and_l_dn_dn);
        SetTable(itable, "1100xxx100xxxxxx", LR and_b_ea);
        SetTable(itable, "1100xxx101xxxxxx", LR and_w_ea);
        SetTable(itable, "1100xxx110xxxxxx", LR and_l_ea);
        SetTable(itable, "1100xxx011xxxxxx", LR mulu);
        SetTable(itable, "1100xxx10000xxxx", LR abcd);
        SetTable(itable, "1100xxx101000xxx", LR exg_d);
        SetTable(itable, "1100xxx101001xxx", LR exg_a);
        SetTable(itable, "1100xxx110001xxx", LR exg_ad);
        SetTable(itable, "1100xxx111xxxxxx", LR muls);
        SetTable(itable, "1101xxx000xxxxxx", LR add_b_dn);
        SetTable(itable, "1101xxx001xxxxxx", LR add_w_dn);
        SetTable(itable, "1101xxx010xxxxxx", LR add_l_dn);
        SetTable(itable, "1101xxx000000xxx", LR add_b_dn_dn);
        SetTable(itable, "1101xxx001000xxx", LR add_w_dn_dn);
        SetTable(itable, "1101xxx010000xxx", LR add_l_dn_dn);
        SetTable(itable, "1101xxx100xxxxxx", LR add_b_ea);
        SetTable(itable, "1101xxx101xxxxxx", LR add_w_ea);
        SetTable(itable, "1101xxx110xxxxxx", LR add_l_ea);
        SetTable(itable, "1101xxx011xxxxxx", LR add_w_an);
        SetTable(itable, "1101xxx111xxxxxx", LR add_l_an);
        SetTable(itable, "1101xxx011000xxx", LR add_w_an_dn);
        SetTable(itable, "1101xxx111000xxx", LR add_l_an_dn);
        SetTable(itable, "1101xxx100000xxx", LR addx_b_r);
        SetTable(itable, "1101xxx101000xxx", LR addx_w_r);
        SetTable(itable, "1101xxx110000xxx", LR addx_l_r);
        SetTable(itable, "1101xxx100001xxx", LR addx_b_m);
        SetTable(itable, "1101xxx101001xxx", LR addx_w_m);
        SetTable(itable, "1101xxx110001xxx", LR addx_l_m);
	/**/


        SetTable(itable, "1110xxx000001xxx", LR lsr_b_i);
        SetTable(itable, "1110xxx100001xxx", LR lsl_b_i);
        SetTable(itable, "1110001000001xxx", LR lsr1_b);
        SetTable(itable, "1110001100001xxx", LR lsl1_b);
        SetTable(itable, "1110xxx001001xxx", LR lsr_w_i);
        SetTable(itable, "1110xxx101001xxx", LR lsl_w_i);
        SetTable(itable, "1110001001001xxx", LR lsr1_w);
        SetTable(itable, "1110001101001xxx", LR lsl1_w);
        SetTable(itable, "1110xxx010001xxx", LR lsr_l_i);
        SetTable(itable, "1110xxx110001xxx", LR lsl_l_i);
        SetTable(itable, "1110001010001xxx", LR lsr1_l);
        SetTable(itable, "1110001110001xxx", LR lsl1_l);
        SetTable(itable, "1110010110001xxx", LR lsl2_l);
        SetTable(itable, "1110xxx000101xxx", LR lsr_b_r);
        SetTable(itable, "1110xxx100101xxx", LR lsl_b_r);
        SetTable(itable, "1110xxx001101xxx", LR lsr_w_r);
        SetTable(itable, "1110xxx101101xxx", LR lsl_w_r);
        SetTable(itable, "1110xxx010101xxx", LR lsr_l_r);
        SetTable(itable, "1110xxx110101xxx", LR lsl_l_r);
        SetTable(itable, "1110xxx000000xxx", LR asr_b_i);
        SetTable(itable, "1110xxx100000xxx", LR asl_b_i);
        SetTable(itable, "1110xxx001000xxx", LR asr_w_i);
        SetTable(itable, "1110xxx101000xxx", LR asl_w_i);
        SetTable(itable, "1110xxx010000xxx", LR asr_l_i);
        SetTable(itable, "1110xxx110000xxx", LR asl_l_i);
        SetTable(itable, "1110xxx000100xxx", LR asr_b_r);
        SetTable(itable, "1110xxx100100xxx", LR asl_b_r);
        SetTable(itable, "1110xxx001100xxx", LR asr_w_r);
        SetTable(itable, "1110xxx101100xxx", LR asl_w_r);
        SetTable(itable, "1110xxx010100xxx", LR asr_l_r);
        SetTable(itable, "1110xxx110100xxx", LR asl_l_r);
        SetTable(itable, "1110xxx000010xxx", LR roxr_b_i);
        SetTable(itable, "1110xxx100010xxx", LR roxl_b_i);
        SetTable(itable, "1110xxx001010xxx", LR roxr_w_i);
        SetTable(itable, "1110xxx101010xxx", LR roxl_w_i);
        SetTable(itable, "1110xxx010010xxx", LR roxr_l_i);
        SetTable(itable, "1110xxx110010xxx", LR roxl_l_i);
        SetTable(itable, "1110xxx000110xxx", LR roxr_b_r);
        SetTable(itable, "1110xxx100110xxx", LR roxl_b_r);
        SetTable(itable, "1110xxx001110xxx", LR roxr_w_r);
        SetTable(itable, "1110xxx101110xxx", LR roxl_w_r);
        SetTable(itable, "1110xxx010110xxx", LR roxr_l_r);
        SetTable(itable, "1110xxx110110xxx", LR roxl_l_r);
        SetTable(itable, "1110xxx000011xxx", LR ror_b_i);
        SetTable(itable, "1110xxx100011xxx", LR rol_b_i);
        SetTable(itable, "1110xxx001011xxx", LR ror_w_i);
        SetTable(itable, "1110xxx101011xxx", LR rol_w_i);
        SetTable(itable, "1110xxx010011xxx", LR ror_l_i);
        SetTable(itable, "1110xxx110011xxx", LR rol_l_i);
        SetTable(itable, "1110xxx000111xxx", LR ror_b_r);
        SetTable(itable, "1110xxx100111xxx", LR rol_b_r);
        SetTable(itable, "1110xxx001111xxx", LR ror_w_r);
        SetTable(itable, "1110xxx101111xxx", LR rol_w_r);
        SetTable(itable, "1110xxx010111xxx", LR ror_l_r);
        SetTable(itable, "1110xxx110111xxx", LR rol_l_r);
        SetTable(itable, "1110000011xxxxxx", LR asr_m); 
        SetTable(itable, "1110000111xxxxxx", LR asl_m);
        SetTable(itable, "1110001011xxxxxx", LR lsr_m);
        SetTable(itable, "1110001111xxxxxx", LR lsl_m);
        SetTable(itable, "1110010011xxxxxx", LR roxr_m);
        SetTable(itable, "1110010111xxxxxx", LR roxl_m);
        SetTable(itable, "1110011011xxxxxx", LR ror_m);
        SetTable(itable, "1110011111xxxxxx", LR rol_m);
        SetTable(itable, "1010xxxxxxxxxxxx", LR code1010);
        SetTable(itable, "1111xxxxxxxxxxxx", LR code1111);
}
#endif

#ifndef IE_XL
void EmulatorTable(Ptr ibuffer)
{  
  qlux_table=ibuffer;
  SetTabEntries(ibuffer);
}
#endif





