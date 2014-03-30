/*
 * (c) UQLX - see COPYRIGHT
 */


#ifdef USE_VM  

/*#include "QLtypes.h"*/
#include "QL68000.h"
#include "vm.h"
#include "insnx.h"

/*
  undo instructions: these do not undo all effects of the instruction,
  rather they reset PC,A0-A7 to values that can be used for an
  instruction retry.
  A full undo is not likely possible without a runtime overhead and not
  needed anyway.
  */





void save_state(x_state *state)
{
  int i;
  
  state->code=code;
  state->pc=pc;
  for(i=0;i<8;i++) state->aReg[i]=aReg[i];
}
void restore_state(x_state *state)
{
  int i;
  
  code=state->code;
  pc=state->pc;
  for(i=0;i<8;i++) aReg[i]=state->aReg[i];
}

int xsize(int size)
{
  switch (size){
  case SZ_B:
  case SZ_W: return 2;
  case SZ_L:return 4;
  }
}
int spsize(int size,int issp)
{
  switch (size){
  case SZ_B:
    if (issp) return 2;
    else return 1;
  case SZ_W: return 2;
  case SZ_L:return 4;
  }
}

/* return EA  for reg/mode/(text info) combination*/
/* returns DUMMY 0x28000 for immediate or reg operands */
uw32 iu_getea(int mode,int r1,int size)
{
  uw32 addr;
  
  if (mode==7 && r1==4)
    {
      pc=(uw16*)((char*)pc+xsize(size));
      return 0x28000;
    }
  if (mode<2) return 0x28000;
  if (mode==3)
    {
      addr=aReg[r1];
      aReg[r1]=aReg[r1]+spsize(size,r1==7);
      return addr;
    }
  
  if (mode==4)
    {
      aReg[r1]=aReg[r1]-spsize(size,r1==7);
      return aReg[r1];
    }
  
  return GetEA[mode](r1);
}


/* reverse the effects of EA access on PC and aRegs */
/* faulting instr opcode, #nr of arg processed: 1 or 2 */
void undo_ea(uw16 opcode, int arg)
{
  if (arg=1)
    {
      pc+=pcchange1;
      aReg[regnum1]+=chng1;
    }
  else
    {
      pc+=pcchange2;
      aReg[regnum1]+=chng1;
      aReg[regnum2]+=chng2;

    }
}

int check_addr(uw32 addr, int write)
{
  addr&=ADDR_MASK;
  if (!write)
    return (addr<0x18000 || addr>0x18000+pagesize );
  
  if (addr>=131072 && addr<=0x28000 && (!scrModTable[(addr-131072)/pagesize]))
    return 0;  /* screen, flag not yet set */
  if (addr<131072) return 0; /* ROM or registers */
  return 1;
}


/* returns 1 if addr is accessible in mode, else returns 0*/
int vm_access(uw32 addr, int write, int size)
{
  return check_addr(addr,write) && check_addr(addr+spsize(size,0)-1,write);
}



int vmerr()   
{
  printf("unexpected VM error \n");
  DbgInfo();
  
  return 0;    
}

/* compare current state with the saved one*/
/* returns 1 on identity */
int compare_state(x_state *state)
{
  int i;
  
  if (code!=state->code) return 0;
  if (pc!=state->pc) return 0;
  for (i=0;i<8;i++) if((state->aReg[i])!=(aReg[i])) return 0;
  return 1;
}

/* *************************************************************** */

/* undo an instruction which accessed 1 EA operand of size size*/
/* return 1 on success and reset pc,a0-a7 */
int insn_undo1(int size,int write,int mode,int r) 
{
  save_state(&orig_state); /* save state */
  
  /* theoretically undo_ea already does the job ...*/
  undo_ea(code,1);
  save_state(&res_state); /* assumed pre-instruction state */

  /* but since we want to be sure */
  /* simulate instr execution to confirm the vmem error */
  if (code!=RW(pc++)) 
    return vmerr();
  
  faultaddr=iu_getea(mode,r,size);  /* expect fault*/
  if (vm_access(faultaddr,write,size))
   return vmerr();

#if 0   /* this simply can't happen, can it? */
  if (!compare_state(&orig_state))
   return vmerr();
#endif
  
  restore_state(&res_state);
  return 1;  /* success !!!! */
}

/* undo an instruction which accessed 2 EA operand of size size*/
/* return 1 on success and reset pc,a0-a7 */
int insn_undo2(int size,int write,int m1,int r1,int m2,int r2)
{
  save_state(&orig_state); /* save state */
  
  undo_ea(code,2);  /* 2 args have been processed (supposedly)*/
  save_state(&res_state); /* supposed pre-instruction state */

  /* here we need real backtracking :*/
  /*    simulate instr execution to confirm the vmem error */
  if (code!=RW(pc++)) goto try1a;    /* try assuming only 1 arg was processed */

  faultaddr=iu_getea(m1,r1,size);  /* don't expect the error here */
  if (!vm_access(faultaddr,0,size)) goto try1a;
  faultaddr=iu_getea(m2,r2,size);  /* there should be the error !*/
  if (vm_access(faultaddr,write/*??????*/,size)) goto try1a; /* can this happen ???*/
  /*if(!compare_state(&orig_state)) goto try1a; */
  
  restore_state(&res_state);
  return 1;  /* success !!!! */

 try1a:
  restore_state(&orig_state);
  return insn_undo1(size,1, m1,r1);
}
int insn_undo_link()
{
  /* crude procedure, assume WriteLong failed */
  save_state(&orig_state);

  undo_ea(code,2);
  save_state(&res_state);
  return 1;
}
/* operand was on predecremented stack */
int stackdec_undo(int size)
{
  return insn_undo_link();
}
int stackop_undo(int size)
{
  return insn_undo_link();
}
int movep_mr_undo()
{ /* we don't really care what exactly caused the error, the retry is straigtforward */
  return insn_undo_link();
}
int movem_undo()
{ /* we don't really care what exactly caused the error, the retry is straigtforward */
  return insn_undo_link();
}

/* shortcut for instrs with 1 imm and 1 ea operand */
int insn_undoimm1(int size,int m,int r1)  
{
  return insn_undo2(size,1,7,4,m,r1);
}

bad_iu()
{
  printf("code %d: this instruction can't have caused a VM error\n",code);
  abort();
  return 0;
}
#define bad_ui bad_iu

/**********************************************************/
ui_ori_b()
{
  return insn_undoimm1(SZ_B,(code>>3)&7,code&7);
}
ui_ori_w()
{
  return insn_undoimm1(SZ_W,(code>>3)&7,code&7);
}
ui_ori_l()
{
  return insn_undoimm1(SZ_L,(code>>3)&7,code&7);
}
ui_btst_d()
{
  short mxx=(code>>3)&7;
  if (mxx) 
    return insn_undo1(SZ_B,0,mxx,code&7);
  else return bad_iu();
  }
ui_bchg_d()
{
  short mxx=(code>>3)&7;
  if (mxx) 
    return insn_undo1(SZ_B,1,mxx,code&7);
  else return bad_iu();
}
ui_btst_s()
{
  short mxx=(code>>3)&7;
  if (mxx) 
    return insn_undo2(SZ_B,0, 7,4,  mxx,code&7);
  else return bad_iu();
  }
ui_bchg_s()
{
  short mxx=(code>>3)&7;
  if (mxx) 
    return insn_undo2(SZ_B,1, 7,4, mxx,code&7);
  else return bad_iu();
}
ui_move_b()
{
  return insn_undo2(SZ_B,1, (code>>3)&7,code&7,  (code>>6)&7,((code>>9)&7));
}
ui_move_w()
{
  return insn_undo2(SZ_W,1, (code>>3)&7,code&7,  (code>>6)&7,((code>>9)&7));
}
ui_move_l()
{
  return insn_undo2(SZ_L,1, (code>>3)&7,code&7,  (code>>6)&7,((code>>9)&7));
}
ui_andi_b()
{
  return insn_undoimm1(SZ_B, (code>>3)&7,code&7);
}
ui_andi_w()
{
  return insn_undoimm1(SZ_W, (code>>3)&7,code&7);
}
ui_andi_l()
{
  return insn_undoimm1(SZ_L, (code>>3)&7,code&7);
}
ui_neg_b()
{
  return insn_undo1(SZ_B,1, (code>>3)&7,code&7);
}
ui_neg_w()
{
  return insn_undo1(SZ_W,1, (code>>3)&7,code&7);
}
ui_neg_l()
{
  return insn_undo1(SZ_L,1, (code>>3)&7,code&7);
}
ui_test_b()
{
  return insn_undo1(SZ_B,0, (code>>3)&7,code&7);
}
ui_test_w()
{
  return insn_undo1(SZ_W,0, (code>>3)&7,code&7);
}ui_test_l()
{
  return insn_undo1(SZ_L,0, (code>>3)&7,code&7);
}
ui_pea()
{
  return stackdec_undo(4);
}
ui_unlk()
{
  return stackop_undo(4);
}
ui_rte()
{
  return stackop_undo(6);
}
ui_cmpm_b()
{
  return insn_undo2(SZ_B,0, 3,code&7, 3,(code>>9)&7);
}
ui_cmpm_w()
{
  return insn_undo2(SZ_W,0, 3,code&7, 3,(code>>9)&7);
}
ui_cmpm_l()
{
  return insn_undo2(SZ_L,0, 3,code&7, 3,(code>>9)&7);
}

ui_addx_b_m()
{
  return insn_undo2(SZ_B,1, 4,code&7, 4,(code>>9)&7);
}
ui_addx_w_m()
{
  return insn_undo2(SZ_W,1, 4,code&7, 4,(code>>9)&7);
}
ui_addx_l_m()
{
  return insn_undo2(SZ_L,1, 4,code&7, 4,(code>>9)&7);
}
ui_movep_w_mr()
{
  return movep_mr_undo();
}
ui_movep_l_mr()
{
  return movep_mr_undo();
}
ui_movep_w_rm()
{
  return movep_mr_undo();
}
ui_movep_l_rm()
{
  return movep_mr_undo();
}
ui_movem_load_w()
{
  return movem_undo();
}
ui_movem_load_l()
{
  return movem_undo();
}
ui_movem_save_w()
{
  return movem_undo();
}
ui_movem_save_l()
{
  return movem_undo();
}
ui_link()
{
  return insn_undo_link();
}




/*******************************************************************/
int (**rx_table)(void);

int vm_undo_instruction()
{
  /*printf("vm_undo_instruction : code %d\n",code);*/
  return rx_table[code]();
}


void init_ixtable()
{
  rx_table=malloc(65536*sizeof(void*));
  
        SetTable(rx_table, "0000000000xxxxxx", ui_ori_b);
        SetTable(rx_table, "0000000001xxxxxx", ui_ori_w);
        SetTable(rx_table, "0000000010xxxxxx", ui_ori_l);
        SetTable(rx_table, "0000000000111100", bad_iu/*ui_ori_to_ccr*/);
        SetTable(rx_table, "0000000001111100", bad_iu/*ui_ori_to_sr*/);
        SetTable(rx_table, "0000xxx100xxxxxx", ui_btst_d);
        SetTable(rx_table, "0000xxx101xxxxxx", ui_bchg_d);
        SetTable(rx_table, "0000xxx110xxxxxx", ui_bchg_d/*clr*/);
        SetTable(rx_table, "0000xxx111xxxxxx", ui_bchg_d/*set*/);
        SetTable(rx_table, "0000xxx100001xxx", ui_movep_w_mr);
        SetTable(rx_table, "0000xxx101001xxx", ui_movep_l_mr);
        SetTable(rx_table, "0000xxx110001xxx", ui_movep_w_rm);
        SetTable(rx_table, "0000xxx111001xxx", ui_movep_l_rm);
        SetTable(rx_table, "0000001000xxxxxx", ui_andi_b);
        SetTable(rx_table, "0000001001xxxxxx", ui_andi_w);
        SetTable(rx_table, "0000001010xxxxxx", ui_andi_l);
        SetTable(rx_table, "0000001000111100", bad_iu/*ui_andi_to_ccr*/);
        SetTable(rx_table, "0000001001111100", bad_iu/*ui_andi_to_sr*/);
        SetTable(rx_table, "0000010000xxxxxx", ui_andi_b/*ui_subi_b*/);
        SetTable(rx_table, "0000010001xxxxxx", ui_andi_w/*ui_subi_w*/);
        SetTable(rx_table, "0000010010xxxxxx", ui_andi_l/*ui_subi_l*/);
        SetTable(rx_table, "0000011000xxxxxx", ui_andi_b/*ui_addi_b*/);
        SetTable(rx_table, "0000011001xxxxxx", ui_andi_w/*ui_addi_w*/);
        SetTable(rx_table, "0000011010xxxxxx", ui_andi_l/*ui_addi_l*/);
        SetTable(rx_table, "0000100000xxxxxx", ui_btst_s);
        SetTable(rx_table, "0000100001xxxxxx", ui_bchg_s);
        SetTable(rx_table, "0000100010xxxxxx", ui_bchg_s);
        SetTable(rx_table, "0000100011xxxxxx", ui_bchg_s);
        SetTable(rx_table, "0000101000xxxxxx", ui_andi_b/*ui_eori_b*/);
        SetTable(rx_table, "0000101001xxxxxx", ui_andi_w/*ui_eori_w*/);
        SetTable(rx_table, "0000101010xxxxxx", ui_andi_l/*ui_eori_l*/);
        SetTable(rx_table, "0000101000111100", bad_iu/*ui_eori_to_ccr*/);
        SetTable(rx_table, "0000101001111100", bad_iu/*ui_eori_to_sr*/);
        SetTable(rx_table, "0000110000xxxxxx", ui_andi_b/*ui_cmpi_b*/);
        SetTable(rx_table, "0000110001xxxxxx", ui_andi_w/*ui_cmpi_w*/);
        SetTable(rx_table, "0000110010xxxxxx", ui_andi_l/*ui_cmpi_l*/);
	/*printf("***** start ui_move_b *****\n");*/
	/*p_stt=0;*/
	
        SetTable(rx_table, "0001xxxxxxxxxxxx", ui_move_b);
        SetTable(rx_table, "0001xxxxxx000xxx", ui_move_b/*_from_dn*/);
        SetTable(rx_table, "0001xxx000xxxxxx", ui_move_b/*_to_dn*/);
        SetTable(rx_table, "0001xxx000000xxx", ui_move_b/*_reg*/);
	/*printf("***** start ui_move_l *****\n");*/
        SetTable(rx_table, "0010xxxxxxxxxxxx", ui_move_l);
        SetTable(rx_table, "0010xxxxxx000xxx", ui_move_l/*_from_dn*/);
        SetTable(rx_table, "0010xxx000xxxxxx", ui_move_l/*_to_dn*/);
        SetTable(rx_table, "0010xxx000000xxx", ui_move_l/*_reg*/);
	/*printf("***** end ui_move_l *****\n");*/
	/*p_stt=0;*/
	
        SetTable(rx_table, "0010xxx001xxxxxx", ui_test_l/*ui_movea_l*/);
        SetTable(rx_table, "0010xxx001001xxx", bad_iu/*ui_movea_l_an*/);
        SetTable(rx_table, "0011xxxxxxxxxxxx", ui_move_w);
        SetTable(rx_table, "0011xxxxxx000xxx", ui_move_w/*_from_dn*/);
        SetTable(rx_table, "0011xxx000xxxxxx", ui_move_w/*_to_dn*/);
        SetTable(rx_table, "0011xxx000000xxx", ui_move_w/*_reg*/);
        SetTable(rx_table, "0011xxx001xxxxxx", ui_test_w/*ui_movea_w*/);
        SetTable(rx_table, "0100000000xxxxxx", ui_neg_b/*ui_negx_b*/);
        SetTable(rx_table, "0100000001xxxxxx", ui_neg_w/*ui_negx_w*/);
        SetTable(rx_table, "0100000010xxxxxx", ui_neg_l/*ui_negx_l*/);
        SetTable(rx_table, "0100000011xxxxxx", ui_neg_w/*ui_move_from_sr*/);
        SetTable(rx_table, "0100xxx110xxxxxx", ui_test_w/*ui_chk*/);
        SetTable(rx_table, "0100xxx111xxxxxx", bad_iu/*ui_lea*/);
	/**/
  SetTable(rx_table, "0100001000xxxxxx", ui_neg_b/*ui_clr_b*/);
        SetTable(rx_table, "0100001001xxxxxx", ui_neg_w/*ui_clr_w*/);
        SetTable(rx_table, "0100001010xxxxxx", ui_neg_l/*ui_clr_l*/);
        SetTable(rx_table, "0100010000xxxxxx", ui_neg_b);
        SetTable(rx_table, "0100010001xxxxxx", ui_neg_w);
        SetTable(rx_table, "0100010010xxxxxx", ui_neg_l);
        SetTable(rx_table, "0100010011xxxxxx", ui_test_w/*ui_move_to_ccr*/);
        SetTable(rx_table, "0100011000xxxxxx", ui_neg_b/*ui_not_b*/);
        SetTable(rx_table, "0100011001xxxxxx", ui_neg_w/*ui_not_w*/);
        SetTable(rx_table, "0100011010xxxxxx", ui_neg_l/*ui_not_l*/);
        SetTable(rx_table, "0100011011xxxxxx", ui_test_w/*ui_move_to_sr*/);
        SetTable(rx_table, "0100100000xxxxxx", ui_neg_b/*ui_nbcd*/);
        SetTable(rx_table, "0100100001xxxxxx", ui_pea);
        SetTable(rx_table, "0100100001000xxx", bad_iu/*ui_swap*/);
        SetTable(rx_table, "0100100010xxxxxx", ui_movem_save_w);
        SetTable(rx_table, "0100100010000xxx", bad_iu/*ui_ext_w*/);
        SetTable(rx_table, "0100100011xxxxxx", ui_movem_save_l);
        SetTable(rx_table, "0100100011000xxx", bad_iu/*ui_ext_l*/);
        SetTable(rx_table, "0100101000xxxxxx", ui_test_b/*ui_tst_b*/);
        SetTable(rx_table, "0100101001xxxxxx", ui_test_w/*ui_tst_w*/);
        SetTable(rx_table, "0100101010xxxxxx", ui_test_l/*ui_tst_l*/);
        SetTable(rx_table, "0100101011xxxxxx", ui_neg_b/*ui_tas*/);
        SetTable(rx_table, "0100101011111xxx", bad_iu/*ui_InvalidCode*/);
        SetTable(rx_table, "010010101111100x", ui_neg_b/*ui_tas*/);
        SetTable(rx_table, "0100101011111100", bad_iu/*ui_illegal*/);
        SetTable(rx_table, "0100110010xxxxxx", ui_movem_load_w);
        SetTable(rx_table, "0100110011xxxxxx", ui_movem_load_l);
        SetTable(rx_table, "010011100100xxxx", bad_iu/*ui_trap*/);


        SetTable(rx_table, "0100111001010xxx", ui_link/*ui_link*/);
        SetTable(rx_table, "0100111001011xxx", ui_unlk);
        SetTable(rx_table, "0100111001100xxx", ui_test_l/*ui_move_to_usp*/);
        SetTable(rx_table, "0100111001101xxx", ui_neg_l/*ui_move_from_usp*/);
        SetTable(rx_table, "0100111001110000", bad_iu/*ui_reset*/);
        SetTable(rx_table, "0100111001110001", bad_iu/*ui_nop*/);
        SetTable(rx_table, "0100111001110010", bad_iu/*ui_stop*/);
        SetTable(rx_table, "0100111001110011", ui_rte);
        SetTable(rx_table, "0100111001110101", ui_unlk/*ui_rts*/);
        SetTable(rx_table, "0100111001110110", bad_iu/*ui_trapv*/);
        SetTable(rx_table, "0100111001110111", ui_rte/*ui_rtr*/);
        SetTable(rx_table, "0100111010xxxxxx", ui_link/*ui_jsr*/);
        SetTable(rx_table, "0100111010111010", ui_link/*ui_jsr_displ*/);
        SetTable(rx_table, "0100111011xxxxxx", bad_iu/*ui_jmp*/);
        SetTable(rx_table, "0101xxx000xxxxxx", ui_neg_b/*ui_addq_b*/);
        SetTable(rx_table, "0101xxx001xxxxxx", ui_neg_w/*ui_addq_w*/);
        SetTable(rx_table, "0101xxx010xxxxxx", ui_neg_l/*ui_addq_l*/);
        SetTable(rx_table, "0101xxx001001xxx", bad_iu/*ui_addq_an*/);
        SetTable(rx_table, "0101xxx010001xxx", bad_iu/*ui_addq_an*/);
        SetTable(rx_table, "0101100001001xxx", bad_iu/*ui_addq_4_an*/);
        SetTable(rx_table, "0101100010001xxx", bad_iu/*ui_addq_4_an*/);
	/**/
       SetTable(rx_table, "0101xxxx11xxxxxx", ui_neg_b/*ui_scc*/);

        SetTable(rx_table, "0101xxxx11001xxx", bad_iu/*ui_dbcc*/);
   
        SetTable(rx_table, "0101xxx100xxxxxx", ui_neg_b/*ui_subq_b*/);
        SetTable(rx_table, "0101xxx101xxxxxx", ui_neg_w/*ui_subq_w*/);
        SetTable(rx_table, "0101xxx110xxxxxx", ui_neg_l/*ui_subq_l*/);

        SetTable(rx_table, "0110xxxxxxxxxxx0", bad_iu/*ui_bcc_s*/);
        SetTable(rx_table, "0110xxxxxxxxxxx1", bad_iu/*ui_bcc_bad*/);

        SetTable(rx_table, "01100001xxxxxxxx", ui_link/*ui_bsr*/);
        SetTable(rx_table, "0111xxx0xxxxxxxx", bad_iu/*ui_moveq*/);
        SetTable(rx_table, "1000xxx000xxxxxx", ui_test_b/*ui_or_b_dn*/);
        SetTable(rx_table, "1000xxx001xxxxxx", ui_test_w/*ui_or_w_dn*/);
        SetTable(rx_table, "1000xxx010xxxxxx", ui_test_l/*ui_or_l_dn*/);
        SetTable(rx_table, "1000xxx100xxxxxx", ui_neg_b/*ui_or_b_ea*/);
        SetTable(rx_table, "1000xxx101xxxxxx", ui_neg_w/*ui_or_w_ea*/);
        SetTable(rx_table, "1000xxx110xxxxxx", ui_neg_l/*ui_or_l_ea*/);
        SetTable(rx_table, "1000xxx011xxxxxx", ui_test_w/*ui_divu*/);
        SetTable(rx_table, "1000xxx10000xxxx", ui_addx_b_m/*ui_sbcd*/);
        SetTable(rx_table, "1000xxx111xxxxxx", ui_test_w/*ui_divs*/);
        SetTable(rx_table, "1001xxx000xxxxxx", ui_test_b/*ui_sub_b_dn*/);
        SetTable(rx_table, "1001xxx001xxxxxx", ui_test_w/*ui_sub_w_dn*/);
        SetTable(rx_table, "1001xxx010xxxxxx", ui_test_l/*ui_sub_l_dn*/);
        SetTable(rx_table, "1001xxx100xxxxxx", ui_neg_b/*ui_sub_b_ea*/);
        SetTable(rx_table, "1001xxx101xxxxxx", ui_neg_w/*ui_sub_w_ea*/);
        SetTable(rx_table, "1001xxx110xxxxxx", ui_neg_l/*ui_sub_l_ea*/);
        SetTable(rx_table, "1001xxx011xxxxxx", ui_test_w/*ui_sub_w_an*/);
        SetTable(rx_table, "1001xxx111xxxxxx", ui_test_l/*ui_sub_l_an*/);
        SetTable(rx_table, "1001xxx100000xxx", bad_iu/*ui_subx_b_r*/);
        SetTable(rx_table, "1001xxx101000xxx", bad_iu/*ui_subx_w_r*/);
        SetTable(rx_table, "1001xxx110000xxx", bad_iu/*ui_subx_l_r*/);
        SetTable(rx_table, "1001xxx100001xxx", ui_test_b/*ui_subx_b_m*/);
        SetTable(rx_table, "1001xxx101001xxx", ui_test_w/*ui_subx_w_m*/);
        SetTable(rx_table, "1001xxx110001xxx", ui_test_l/*ui_subx_l_m*/);
        SetTable(rx_table, "1011xxx000xxxxxx", ui_test_b/*ui_cmp_b*/);
        SetTable(rx_table, "1011xxx001xxxxxx", ui_test_w/*ui_cmp_w*/);
        SetTable(rx_table, "1011xxx010xxxxxx", ui_test_l/*ui_cmp_l*/);
        SetTable(rx_table, "1011xxx000000xxx", bad_iu/*ui_cmp_b_dn*/);
        SetTable(rx_table, "1011xxx000101xxx", bad_iu/*ui_cmp_b_dan*/);
        SetTable(rx_table, "1011xxx001000xxx", bad_iu/*ui_cmp_w_dn*/);
        SetTable(rx_table, "1011xxx010000xxx", bad_iu/*ui_cmp_l_dn*/);
        SetTable(rx_table, "1011xxx011xxxxxx", ui_test_w/*ui_cmpa_w*/);
        SetTable(rx_table, "1011xxx111xxxxxx", ui_test_l/*ui_cmpa_l*/);
        SetTable(rx_table, "1011xxx111001xxx", bad_iu/*ui_cmpa_l_an);
        SetTable(rx_table, "1011xxx100xxxxxx", ui_neg_b/*ui_eor_b*/);
        SetTable(rx_table, "1011xxx101xxxxxx", ui_neg_w/*ui_eor_w*/);
        SetTable(rx_table, "1011xxx110xxxxxx", ui_neg_l/*ui_eor_l*/);
	/**/
       SetTable(rx_table, "1011xxx100001xxx", ui_cmpm_b);
        SetTable(rx_table, "1011xxx101001xxx", ui_cmpm_w);
        SetTable(rx_table, "1011xxx110001xxx", ui_cmpm_l);
        SetTable(rx_table, "1100xxx000xxxxxx", ui_test_b/*ui_and_b_dn*/);
        SetTable(rx_table, "1100xxx001xxxxxx", ui_test_w/*ui_and_w_dn*/);
        SetTable(rx_table, "1100xxx010xxxxxx", ui_test_l/*ui_and_l_dn*/);
        SetTable(rx_table, "1100xxx010000xxx", bad_ui/*ui_and_l_dn_dn*/);
        SetTable(rx_table, "1100xxx100xxxxxx", ui_neg_b/*ui_and_b_ea*/);
        SetTable(rx_table, "1100xxx101xxxxxx", ui_neg_w/*ui_and_w_ea*/);
        SetTable(rx_table, "1100xxx110xxxxxx", ui_neg_l/*ui_and_l_ea*/);
        SetTable(rx_table, "1100xxx011xxxxxx", ui_test_w/*ui_mulu*/);
        SetTable(rx_table, "1100xxx10000xxxx", ui_addx_b_m/*ui_abcd*/);
        SetTable(rx_table, "1100xxx101000xxx", bad_ui/*ui_exg_d*/);
        SetTable(rx_table, "1100xxx101001xxx", bad_ui/*ui_exg_a*/);
        SetTable(rx_table, "1100xxx110001xxx", bad_ui/*ui_exg_ad*/);
        SetTable(rx_table, "1100xxx111xxxxxx", ui_test_w/*ui_muls*/);
        SetTable(rx_table, "1101xxx000xxxxxx", ui_test_b/*ui_add_b_dn*/);
        SetTable(rx_table, "1101xxx001xxxxxx", ui_test_w/*ui_add_w_dn*/);
        SetTable(rx_table, "1101xxx010xxxxxx", ui_test_l/*ui_add_l_dn*/);
        SetTable(rx_table, "1101xxx000000xxx", bad_iu/*ui_add_b_dn_dn*/);
        SetTable(rx_table, "1101xxx001000xxx", bad_iu/*ui_add_w_dn_dn*/);
        SetTable(rx_table, "1101xxx010000xxx", bad_iu/*ui_add_l_dn_dn*/);
        SetTable(rx_table, "1101xxx100xxxxxx", ui_neg_b/*ui_add_b_ea*/);
        SetTable(rx_table, "1101xxx101xxxxxx", ui_neg_w/*ui_add_w_ea*/);
        SetTable(rx_table, "1101xxx110xxxxxx", ui_neg_l/*ui_add_l_ea*/);
        SetTable(rx_table, "1101xxx011xxxxxx", ui_test_w/*ui_add_w_an*/);
        SetTable(rx_table, "1101xxx111xxxxxx", ui_test_l/*ui_add_l_an*/);
        SetTable(rx_table, "1101xxx011000xxx", bad_iu/*ui_add_w_an_dn*/);
        SetTable(rx_table, "1101xxx111000xxx", bad_iu/*ui_add_l_an_dn*/);
        SetTable(rx_table, "1101xxx100000xxx", bad_iu/*ui_addx_b_r*/);
        SetTable(rx_table, "1101xxx101000xxx", bad_iu/*ui_addx_w_r*/);
        SetTable(rx_table, "1101xxx110000xxx", bad_iu/*ui_addx_l_r*/);
        SetTable(rx_table, "1101xxx100001xxx", ui_addx_b_m);
        SetTable(rx_table, "1101xxx101001xxx", ui_addx_w_m);
        SetTable(rx_table, "1101xxx110001xxx", ui_addx_l_m);
	/**/
 

        SetTable(rx_table, "1110xxx000001xxx", bad_iu/*ui_lsr_b_i*/);
        SetTable(rx_table, "1110xxx100001xxx", bad_iu/*ui_lsl_b_i*/);
        SetTable(rx_table, "1110001000001xxx", bad_iu/*ui_lsr1_b*/);
        SetTable(rx_table, "1110001100001xxx", bad_iu/*ui_lsl1_b*/);
        SetTable(rx_table, "1110xxx001001xxx", bad_iu/*ui_lsr_w_i*/);
        SetTable(rx_table, "1110xxx101001xxx", bad_iu/*ui_lsl_w_i*/);
        SetTable(rx_table, "1110001001001xxx", bad_iu/*ui_lsr1_w*/);
        SetTable(rx_table, "1110001101001xxx", bad_iu/*ui_lsl1_w*/);
        SetTable(rx_table, "1110xxx010001xxx", bad_iu/*ui_lsr_l_i*/);
        SetTable(rx_table, "1110xxx110001xxx", bad_iu/*ui_lsl_l_i*/);
        SetTable(rx_table, "1110001010001xxx", bad_iu/*ui_lsr1_l*/);
        SetTable(rx_table, "1110001110001xxx", bad_iu/*ui_lsl1_l*/);
        SetTable(rx_table, "1110010110001xxx", bad_iu/*ui_lsl2_l*/);
        SetTable(rx_table, "1110xxx000101xxx", bad_iu/*ui_lsr_b_r*/);
        SetTable(rx_table, "1110xxx100101xxx", bad_iu/*ui_lsl_b_r*/);
        SetTable(rx_table, "1110xxx001101xxx", bad_iu/*ui_lsr_w_r*/);
        SetTable(rx_table, "1110xxx101101xxx", bad_iu/*ui_lsl_w_r*/);
        SetTable(rx_table, "1110xxx010101xxx", bad_iu/*ui_lsr_l_r*/);
        SetTable(rx_table, "1110xxx110101xxx", bad_iu/*ui_lsl_l_r*/);
        SetTable(rx_table, "1110xxx000000xxx", bad_iu/*ui_asr_b_i*/);
        SetTable(rx_table, "1110xxx100000xxx", bad_iu/*ui_asl_b_i*/);
        SetTable(rx_table, "1110xxx001000xxx", bad_iu/*ui_asr_w_i*/);
        SetTable(rx_table, "1110xxx101000xxx", bad_iu/*ui_asl_w_i*/);
        SetTable(rx_table, "1110xxx010000xxx", bad_iu/*ui_asr_l_i*/);
        SetTable(rx_table, "1110xxx110000xxx", bad_iu/*ui_asl_l_i*/);
        SetTable(rx_table, "1110xxx000100xxx", bad_iu/*ui_asr_b_r*/);
        SetTable(rx_table, "1110xxx100100xxx", bad_iu/*ui_asl_b_r*/);
        SetTable(rx_table, "1110xxx001100xxx", bad_iu/*ui_asr_w_r*/);
        SetTable(rx_table, "1110xxx101100xxx", bad_iu/*ui_asl_w_r*/);
        SetTable(rx_table, "1110xxx010100xxx", bad_iu/*ui_asr_l_r*/);
        SetTable(rx_table, "1110xxx110100xxx", bad_iu/*ui_asl_l_r*/);
        SetTable(rx_table, "1110xxx000010xxx", bad_iu/*ui_roxr_b_i*/);
        SetTable(rx_table, "1110xxx100010xxx", bad_iu/*ui_roxl_b_i*/);
        SetTable(rx_table, "1110xxx001010xxx", bad_ui/*ui_roxr_w_i*/);
        SetTable(rx_table, "1110xxx101010xxx", bad_ui/*ui_roxl_w_i*/);
        SetTable(rx_table, "1110xxx010010xxx", bad_ui/*ui_roxr_l_i*/);
        SetTable(rx_table, "1110xxx110010xxx", bad_ui/*ui_roxl_l_i*/);
        SetTable(rx_table, "1110xxx000110xxx", bad_ui/*ui_roxr_b_r*/);
        SetTable(rx_table, "1110xxx100110xxx", bad_ui/*ui_roxl_b_r*/);
        SetTable(rx_table, "1110xxx001110xxx", bad_ui/*ui_roxr_w_r*/);
        SetTable(rx_table, "1110xxx101110xxx", bad_ui/*ui_roxl_w_r*/);
        SetTable(rx_table, "1110xxx010110xxx", bad_ui/*ui_roxr_l_r*/);
        SetTable(rx_table, "1110xxx110110xxx", bad_ui/*ui_roxl_l_r*/);
        SetTable(rx_table, "1110xxx000011xxx", bad_ui/*ui_ror_b_i*/);
        SetTable(rx_table, "1110xxx100011xxx", bad_ui/*ui_rol_b_i*/);
        SetTable(rx_table, "1110xxx001011xxx", bad_ui/*ui_ror_w_i*/);
        SetTable(rx_table, "1110xxx101011xxx", bad_ui/*ui_rol_w_i*/);
        SetTable(rx_table, "1110xxx010011xxx", bad_ui/*ui_ror_l_i*/);
        SetTable(rx_table, "1110xxx110011xxx", bad_ui/*ui_rol_l_i*/);
        SetTable(rx_table, "1110xxx000111xxx", bad_ui/*ui_ror_b_r*/);
        SetTable(rx_table, "1110xxx100111xxx", bad_ui/*ui_rol_b_r*/);
        SetTable(rx_table, "1110xxx001111xxx", bad_ui/*ui_ror_w_r*/);
        SetTable(rx_table, "1110xxx101111xxx", bad_ui/*ui_rol_w_r*/);
        SetTable(rx_table, "1110xxx010111xxx", bad_ui/*ui_ror_l_r*/);
        SetTable(rx_table, "1110xxx110111xxx", bad_ui/*ui_rol_l_r*/);
        SetTable(rx_table, "1110000011xxxxxx", ui_test_w/*ui_asr_m*/); 
        SetTable(rx_table, "1110000111xxxxxx", ui_test_w/*ui_asl_m*/);
        SetTable(rx_table, "1110001011xxxxxx", ui_test_w/*ui_lsr_m*/);
        SetTable(rx_table, "1110001111xxxxxx", ui_test_w/*ui_lsl_m*/);
        SetTable(rx_table, "1110010011xxxxxx", ui_test_w/*ui_roxr_m*/);
        SetTable(rx_table, "1110010111xxxxxx", ui_test_w/*ui_roxl_m*/);
        SetTable(rx_table, "1110011011xxxxxx", ui_test_w/*ui_ror_m*/);
        SetTable(rx_table, "1110011111xxxxxx", ui_test_w/*ui_rol_m*/);
        SetTable(rx_table, "1010xxxxxxxxxxxx", bad_ui/*ui_code1010*/);
        SetTable(rx_table, "1111xxxxxxxxxxxx", bad_ui/*ui_code1111*/);
}
#endif
