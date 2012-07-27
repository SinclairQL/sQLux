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

  Current strategy is: 
                - find out which instr caused the exception: search backwards
		  for 'code', sanity check
		- get approximate location of faulting address, predecr&postincr
		  make it to costly to find out the exact place
		  - for SCREEN access, mark & unptotect it
		  - for HDREG access, undo is reliable so redo the instr
		    by pure emulation
  */


void test_opcodes();

/* test ALL possible opcodes that could cause a VM exception */

init_VM()
{
  test_opcodes();
}


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

/* shortcut for instrs with 1 imm and 1 ea operand */
int insn_undoimm1(int size,int m,int r1)  
{
  return insn_undo2(size,1,7,4,m,r1);
}

/**********************************************************/
vmtest_ori_b()
{
  return insn_undoimm1(SZ_B,(code>>3)&7,code&7);
}
vmtest_ori_w()
{
  return insn_undoimm1(SZ_W,(code>>3)&7,code&7);
}
vmtest_ori_l()
{
  return insn_undoimm1(SZ_L,(code>>3)&7,code&7);
}
vmtest_btst_d()
{
  short mxx=(code>>3)&7;
  if (mxx) 
    return insn_undo1(SZ_B,0,mxx,code&7);
  else return bad_iu();
  }
vmtest_bchg_d()
{
  short mxx=(code>>3)&7;
  if (mxx) 
    return insn_undo1(SZ_B,1,mxx,code&7);
  else return bad_iu();
}
vmtest_btst_s()
{
  short mxx=(code>>3)&7;
  if (mxx) 
    return insn_undo2(SZ_B,0, 7,4,  mxx,code&7);
  else return bad_iu();
  }
vmtest_bchg_s()
{
  short mxx=(code>>3)&7;
  if (mxx) 
    return insn_undo2(SZ_B,1, 7,4, mxx,code&7);
  else return bad_iu();
}
vmtest_move_b()
{
  return insn_undo2(SZ_B,1, (code>>3)&7,code&7,  (code>>6)&7,((code>>9)&7));
}
vmtest_move_w()
{
  return insn_undo2(SZ_W,1, (code>>3)&7,code&7,  (code>>6)&7,((code>>9)&7));
}
vmtest_move_l()
{
  return insn_undo2(SZ_L,1, (code>>3)&7,code&7,  (code>>6)&7,((code>>9)&7));
}
vmtest_andi_b()
{
  return insn_undoimm1(SZ_B, (code>>3)&7,code&7);
}
vmtest_andi_w()
{
  return insn_undoimm1(SZ_W, (code>>3)&7,code&7);
}
vmtest_andi_l()
{
  return insn_undoimm1(SZ_L, (code>>3)&7,code&7);
}
vmtest_neg_b()
{
  return insn_undo1(SZ_B,1, (code>>3)&7,code&7);
}
vmtest_neg_w()
{
  return insn_undo1(SZ_W,1, (code>>3)&7,code&7);
}
vmtest_neg_l()
{
  return insn_undo1(SZ_L,1, (code>>3)&7,code&7);
}
vmtest_test_b()
{
  return insn_undo1(SZ_B,0, (code>>3)&7,code&7);
}
vmtest_test_w()
{
  return insn_undo1(SZ_W,0, (code>>3)&7,code&7);
}vmtest_test_l()
{
  return insn_undo1(SZ_L,0, (code>>3)&7,code&7);
}
vmtest_pea()
{
  return stackdec_undo(4);
}
vmtest_unlk()
{
  return stackop_undo(4);
}
vmtest_rte()
{
  return stackop_undo(6);
}
vmtest_cmpm_b()
{
  return insn_undo2(SZ_B,0, 3,code&7, 3,(code>>9)&7);
}
vmtest_cmpm_w()
{
  return insn_undo2(SZ_W,0, 3,code&7, 3,(code>>9)&7);
}
vmtest_cmpm_l()
{
  return insn_undo2(SZ_L,0, 3,code&7, 3,(code>>9)&7);
}

vmtest_addx_b_m()
{
  return insn_undo2(SZ_B,1, 4,code&7, 4,(code>>9)&7);
}
vmtest_addx_w_m()
{
  return insn_undo2(SZ_W,1, 4,code&7, 4,(code>>9)&7);
}
vmtest_addx_l_m()
{
  return insn_undo2(SZ_L,1, 4,code&7, 4,(code>>9)&7);
}
vmtest_movep_w_mr()
{
  return movep_mr_undo();
}
vmtest_movep_l_mr()
{
  return movep_mr_undo();
}
vmtest_movep_w_rm()
{
  return movep_mr_undo();
}
vmtest_movep_l_rm()
{
  return movep_mr_undo();
}
vmtest_movem_load_w()
{
  return movem_undo();
}
vmtest_movem_load_l()
{
  return movem_undo();
}
vmtest_movem_save_w()
{
  return movem_undo();
}
vmtest_movem_save_l()
{
  return movem_undo();
}
vmtest_link()
{
  return insn_undo_link();
}




/*******************************************************************/
int (**rx_table)(void);    /* ?? */
struct undo_exp *rxu_table;   /* pointers to undo values*/

int vm_undo()
{
  struct undo_exp *undo;
  /*printf("vm_undo_instruction : code %d\n",code);*/
  if ( (undo=rxa_table[code])!=0 )
    {
      aReg[undo->regnum1]+=undo->chng1;
      aReg[undo->regnum2]+=undo->chng2;
      pc+=undo->pcchange;
      return 1;
    }
  
  else vmerr();
  return 0;
}

/* *************************************************************************** */
uw32 faultaddr;
/* test if this instr caused a fault, set fault addr and return 0 on success */
int test_fault()
{
}

/* RAW fn to guess the initial PC of faulting instruction */
int vm_find_xpc()
{
  save_state(initial_state);
  
  for (i=0; i<7 ;i++)
    if( pc[-i]==code) break; /* assume we have it.. */

  if (i>6) vmerr();
  pc-=i;
  if ( !test_fault())
    for (j=0; i<7 ;i++,j++)             /* second and last try */
	if( pc[-i]==code) break; /* assume we have it.. */
  if (i>6) vmerr();
  pc-=j;
  if ( !test_fault())
    vmerr();
}

/* main entry point*/
void vm_handle_fault()
{
  if (vm_test((char*)pc-(char*)theROM,M_READ))
    vmerrmsg("PC at read protected area?!\n"); /* must be pc reading from hwreg area ... */

  vm_find_xpc();

  if (faultaddr<128*1024)
    {
      vm_undo();  /* iexl_general.c does the redo for us */
      vm_ljback(); /* longjmp out of here */
      return 0; /* bogus return */
    }

screen:  
  /* else it must be screen.. */
  vmMarkScreen(faultaddr);
  return 0;   /* retry instruction */
}



/* generate all possible VM faults..*/
void test_all_opcodes()
{
  uw32 opcode;
  int i,j;

  /* set xlarge VM areas to distinguish safely .... */
  mprotect(); /* protect everything below 0x20000 */
  mprotect(); /* unprotect everything else */

  pc=(uw16*)((char*)theROM+0x28000+4096);


  for(opcode=0;opcode<65536;opcode++)  /* for ALL 68000 instructions .... */
    {
#if 0 /* primitive setup seems far to costly, also a problem with x(PC,Rn) */
      /* test1: all Aregs to VM-fault area */
      aReg[0]=aReg[1]=aReg[2]=aReg[3]=aReg[4]=aReg[5]=aReg[6]=aReg[7]=0x4000;
      /* adjust for possible x(An,Dn)*/
      reg[0]=reg[1]=reg[2]=reg[3]=reg[4]=reg[5]=reg[6]=reg[7]=0;
#else
      set_safe_regs();
      i=insn_fargs(opcode);
      if (i==0)
	continue;

      regs_setup_vmfault1(opcode);
#endif
      save_state(&init_test_state);

      if (sigsetjmp(test_catch,1))
	goto xx_test_done;
      /* do the test */
      itable[opcode](opcode);

      printf("warning: no exception in test_opcodes, opcode %d\n",opcode);

    xx_test1_done:
      save_undo_diff(opcode,&init_test_state,ARG1)

      if (i==1) continue;

      regs_setup_vmfault2(opcode);
      set_safe_regs();
      save_state(&init_test_state);
      
      if (sigsetjmp(test_catch,1))
	goto xx_test2_done;
      /* do the test */
      itable[opcode](opcode);
      
      printf("warning: no exception in test_opcodes, opcode %d\n",opcode);
      continue; 

    xx_test2_done:
      save_undo_diff(opcode,&init_test_state,ARG2)
      
    }
}


void init_ixtable()
{
  rx_table=malloc(65536*sizeof(void*));
  
        SetTable(rx_table, "0000000000xxxxxx", vmtest_ori_b);
        SetTable(rx_table, "0000000001xxxxxx", vmtest_ori_w);
        SetTable(rx_table, "0000000010xxxxxx", vmtest_ori_l);
        SetTable(rx_table, "0000000000111100", vmnotest/*vmtest_ori_to_ccr*/);
        SetTable(rx_table, "0000000001111100", vmnotest/*vmtest_ori_to_sr*/);
        SetTable(rx_table, "0000xxx100xxxxxx", vmtest_btst_d);
        SetTable(rx_table, "0000xxx101xxxxxx", vmtest_bchg_d);
        SetTable(rx_table, "0000xxx110xxxxxx", vmtest_bchg_d/*clr*/);
        SetTable(rx_table, "0000xxx111xxxxxx", vmtest_bchg_d/*set*/);
        SetTable(rx_table, "0000xxx100001xxx", vmtest_movep_w_mr);
        SetTable(rx_table, "0000xxx101001xxx", vmtest_movep_l_mr);
        SetTable(rx_table, "0000xxx110001xxx", vmtest_movep_w_rm);
        SetTable(rx_table, "0000xxx111001xxx", vmtest_movep_l_rm);
        SetTable(rx_table, "0000001000xxxxxx", vmtest_andi_b);
        SetTable(rx_table, "0000001001xxxxxx", vmtest_andi_w);
        SetTable(rx_table, "0000001010xxxxxx", vmtest_andi_l);
        SetTable(rx_table, "0000001000111100", vmnotest/*vmtest_andi_to_ccr*/);
        SetTable(rx_table, "0000001001111100", vmnotest/*vmtest_andi_to_sr*/);
        SetTable(rx_table, "0000010000xxxxxx", vmtest_andi_b/*vmtest_subi_b*/);
        SetTable(rx_table, "0000010001xxxxxx", vmtest_andi_w/*vmtest_subi_w*/);
        SetTable(rx_table, "0000010010xxxxxx", vmtest_andi_l/*vmtest_subi_l*/);
        SetTable(rx_table, "0000011000xxxxxx", vmtest_andi_b/*vmtest_addi_b*/);
        SetTable(rx_table, "0000011001xxxxxx", vmtest_andi_w/*vmtest_addi_w*/);
        SetTable(rx_table, "0000011010xxxxxx", vmtest_andi_l/*vmtest_addi_l*/);
        SetTable(rx_table, "0000100000xxxxxx", vmtest_btst_s);
        SetTable(rx_table, "0000100001xxxxxx", vmtest_bchg_s);
        SetTable(rx_table, "0000100010xxxxxx", vmtest_bchg_s);
        SetTable(rx_table, "0000100011xxxxxx", vmtest_bchg_s);
        SetTable(rx_table, "0000101000xxxxxx", vmtest_andi_b/*vmtest_eori_b*/);
        SetTable(rx_table, "0000101001xxxxxx", vmtest_andi_w/*vmtest_eori_w*/);
        SetTable(rx_table, "0000101010xxxxxx", vmtest_andi_l/*vmtest_eori_l*/);
        SetTable(rx_table, "0000101000111100", vmnotest/*vmtest_eori_to_ccr*/);
        SetTable(rx_table, "0000101001111100", vmnotest/*vmtest_eori_to_sr*/);
        SetTable(rx_table, "0000110000xxxxxx", vmtest_andi_b/*vmtest_cmpi_b*/);
        SetTable(rx_table, "0000110001xxxxxx", vmtest_andi_w/*vmtest_cmpi_w*/);
        SetTable(rx_table, "0000110010xxxxxx", vmtest_andi_l/*vmtest_cmpi_l*/);
	/*printf("***** start vmtest_move_b *****\n");*/
	/*p_stt=0;*/
	
        SetTable(rx_table, "0001xxxxxxxxxxxx", vmtest_move_b);
        SetTable(rx_table, "0001xxxxxx000xxx", vmtest_move_b/*_from_dn*/);
        SetTable(rx_table, "0001xxx000xxxxxx", vmtest_move_b/*_to_dn*/);
        SetTable(rx_table, "0001xxx000000xxx", vmtest_move_b/*_reg*/);
	/*printf("***** start vmtest_move_l *****\n");*/
        SetTable(rx_table, "0010xxxxxxxxxxxx", vmtest_move_l);
        SetTable(rx_table, "0010xxxxxx000xxx", vmtest_move_l/*_from_dn*/);
        SetTable(rx_table, "0010xxx000xxxxxx", vmtest_move_l/*_to_dn*/);
        SetTable(rx_table, "0010xxx000000xxx", vmtest_move_l/*_reg*/);
	/*printf("***** end vmtest_move_l *****\n");*/
	/*p_stt=0;*/
	
        SetTable(rx_table, "0010xxx001xxxxxx", vmtest_test_l/*vmtest_movea_l*/);
        SetTable(rx_table, "0010xxx001001xxx", vmnotest/*vmtest_movea_l_an*/);
        SetTable(rx_table, "0011xxxxxxxxxxxx", vmtest_move_w);
        SetTable(rx_table, "0011xxxxxx000xxx", vmtest_move_w/*_from_dn*/);
        SetTable(rx_table, "0011xxx000xxxxxx", vmtest_move_w/*_to_dn*/);
        SetTable(rx_table, "0011xxx000000xxx", vmtest_move_w/*_reg*/);
        SetTable(rx_table, "0011xxx001xxxxxx", vmtest_test_w/*vmtest_movea_w*/);
        SetTable(rx_table, "0100000000xxxxxx", vmtest_neg_b/*vmtest_negx_b*/);
        SetTable(rx_table, "0100000001xxxxxx", vmtest_neg_w/*vmtest_negx_w*/);
        SetTable(rx_table, "0100000010xxxxxx", vmtest_neg_l/*vmtest_negx_l*/);
        SetTable(rx_table, "0100000011xxxxxx", vmtest_neg_w/*vmtest_move_from_sr*/);
        SetTable(rx_table, "0100xxx110xxxxxx", vmtest_test_w/*vmtest_chk*/);
        SetTable(rx_table, "0100xxx111xxxxxx", vmnotest/*vmtest_lea*/);
	/**/
  SetTable(rx_table, "0100001000xxxxxx", vmtest_neg_b/*vmtest_clr_b*/);
        SetTable(rx_table, "0100001001xxxxxx", vmtest_neg_w/*vmtest_clr_w*/);
        SetTable(rx_table, "0100001010xxxxxx", vmtest_neg_l/*vmtest_clr_l*/);
        SetTable(rx_table, "0100010000xxxxxx", vmtest_neg_b);
        SetTable(rx_table, "0100010001xxxxxx", vmtest_neg_w);
        SetTable(rx_table, "0100010010xxxxxx", vmtest_neg_l);
        SetTable(rx_table, "0100010011xxxxxx", vmtest_test_w/*vmtest_move_to_ccr*/);
        SetTable(rx_table, "0100011000xxxxxx", vmtest_neg_b/*vmtest_not_b*/);
        SetTable(rx_table, "0100011001xxxxxx", vmtest_neg_w/*vmtest_not_w*/);
        SetTable(rx_table, "0100011010xxxxxx", vmtest_neg_l/*vmtest_not_l*/);
        SetTable(rx_table, "0100011011xxxxxx", vmtest_test_w/*vmtest_move_to_sr*/);
        SetTable(rx_table, "0100100000xxxxxx", vmtest_neg_b/*vmtest_nbcd*/);
        SetTable(rx_table, "0100100001xxxxxx", vmtest_pea);
        SetTable(rx_table, "0100100001000xxx", vmnotest/*vmtest_swap*/);
        SetTable(rx_table, "0100100010xxxxxx", vmtest_movem_save_w);
        SetTable(rx_table, "0100100010000xxx", vmnotest/*vmtest_ext_w*/);
        SetTable(rx_table, "0100100011xxxxxx", vmtest_movem_save_l);
        SetTable(rx_table, "0100100011000xxx", vmnotest/*vmtest_ext_l*/);
        SetTable(rx_table, "0100101000xxxxxx", vmtest_test_b/*vmtest_tst_b*/);
        SetTable(rx_table, "0100101001xxxxxx", vmtest_test_w/*vmtest_tst_w*/);
        SetTable(rx_table, "0100101010xxxxxx", vmtest_test_l/*vmtest_tst_l*/);
        SetTable(rx_table, "0100101011xxxxxx", vmtest_neg_b/*vmtest_tas*/);
        SetTable(rx_table, "0100101011111xxx", vmnotest/*vmtest_InvalidCode*/);
        SetTable(rx_table, "010010101111100x", vmtest_neg_b/*vmtest_tas*/);
        SetTable(rx_table, "0100101011111100", vmnotest/*vmtest_illegal*/);
        SetTable(rx_table, "0100110010xxxxxx", vmtest_movem_load_w);
        SetTable(rx_table, "0100110011xxxxxx", vmtest_movem_load_l);
        SetTable(rx_table, "010011100100xxxx", vmnotest/*vmtest_trap*/);


        SetTable(rx_table, "0100111001010xxx", vmtest_link/*vmtest_link*/);
        SetTable(rx_table, "0100111001011xxx", vmtest_unlk);
        SetTable(rx_table, "0100111001100xxx", vmtest_test_l/*vmtest_move_to_usp*/);
        SetTable(rx_table, "0100111001101xxx", vmtest_neg_l/*vmtest_move_from_usp*/);
        SetTable(rx_table, "0100111001110000", vmnotest/*vmtest_reset*/);
        SetTable(rx_table, "0100111001110001", vmnotest/*vmtest_nop*/);
        SetTable(rx_table, "0100111001110010", vmnotest/*vmtest_stop*/);
        SetTable(rx_table, "0100111001110011", vmtest_rte);
        SetTable(rx_table, "0100111001110101", vmtest_unlk/*vmtest_rts*/);
        SetTable(rx_table, "0100111001110110", vmnotest/*vmtest_trapv*/);
        SetTable(rx_table, "0100111001110111", vmtest_rte/*vmtest_rtr*/);
        SetTable(rx_table, "0100111010xxxxxx", vmtest_link/*vmtest_jsr*/);
        SetTable(rx_table, "0100111010111010", vmtest_link/*vmtest_jsr_displ*/);
        SetTable(rx_table, "0100111011xxxxxx", vmnotest/*vmtest_jmp*/);
        SetTable(rx_table, "0101xxx000xxxxxx", vmtest_neg_b/*vmtest_addq_b*/);
        SetTable(rx_table, "0101xxx001xxxxxx", vmtest_neg_w/*vmtest_addq_w*/);
        SetTable(rx_table, "0101xxx010xxxxxx", vmtest_neg_l/*vmtest_addq_l*/);
        SetTable(rx_table, "0101xxx001001xxx", vmnotest/*vmtest_addq_an*/);
        SetTable(rx_table, "0101xxx010001xxx", vmnotest/*vmtest_addq_an*/);
        SetTable(rx_table, "0101100001001xxx", vmnotest/*vmtest_addq_4_an*/);
        SetTable(rx_table, "0101100010001xxx", vmnotest/*vmtest_addq_4_an*/);
	/**/
       SetTable(rx_table, "0101xxxx11xxxxxx", vmtest_neg_b/*vmtest_scc*/);

        SetTable(rx_table, "0101xxxx11001xxx", vmnotest/*vmtest_dbcc*/);
   
        SetTable(rx_table, "0101xxx100xxxxxx", vmtest_neg_b/*vmtest_subq_b*/);
        SetTable(rx_table, "0101xxx101xxxxxx", vmtest_neg_w/*vmtest_subq_w*/);
        SetTable(rx_table, "0101xxx110xxxxxx", vmtest_neg_l/*vmtest_subq_l*/);

        SetTable(rx_table, "0110xxxxxxxxxxx0", vmnotest/*vmtest_bcc_s*/);
        SetTable(rx_table, "0110xxxxxxxxxxx1", vmnotest/*vmtest_bcc_bad*/);

        SetTable(rx_table, "01100001xxxxxxxx", vmtest_link/*vmtest_bsr*/);
        SetTable(rx_table, "0111xxx0xxxxxxxx", vmnotest/*vmtest_moveq*/);
        SetTable(rx_table, "1000xxx000xxxxxx", vmtest_test_b/*vmtest_or_b_dn*/);
        SetTable(rx_table, "1000xxx001xxxxxx", vmtest_test_w/*vmtest_or_w_dn*/);
        SetTable(rx_table, "1000xxx010xxxxxx", vmtest_test_l/*vmtest_or_l_dn*/);
        SetTable(rx_table, "1000xxx100xxxxxx", vmtest_neg_b/*vmtest_or_b_ea*/);
        SetTable(rx_table, "1000xxx101xxxxxx", vmtest_neg_w/*vmtest_or_w_ea*/);
        SetTable(rx_table, "1000xxx110xxxxxx", vmtest_neg_l/*vmtest_or_l_ea*/);
        SetTable(rx_table, "1000xxx011xxxxxx", vmtest_test_w/*vmtest_divu*/);
        SetTable(rx_table, "1000xxx10000xxxx", vmtest_addx_b_m/*vmtest_sbcd*/);
        SetTable(rx_table, "1000xxx111xxxxxx", vmtest_test_w/*vmtest_divs*/);
        SetTable(rx_table, "1001xxx000xxxxxx", vmtest_test_b/*vmtest_sub_b_dn*/);
        SetTable(rx_table, "1001xxx001xxxxxx", vmtest_test_w/*vmtest_sub_w_dn*/);
        SetTable(rx_table, "1001xxx010xxxxxx", vmtest_test_l/*vmtest_sub_l_dn*/);
        SetTable(rx_table, "1001xxx100xxxxxx", vmtest_neg_b/*vmtest_sub_b_ea*/);
        SetTable(rx_table, "1001xxx101xxxxxx", vmtest_neg_w/*vmtest_sub_w_ea*/);
        SetTable(rx_table, "1001xxx110xxxxxx", vmtest_neg_l/*vmtest_sub_l_ea*/);
        SetTable(rx_table, "1001xxx011xxxxxx", vmtest_test_w/*vmtest_sub_w_an*/);
        SetTable(rx_table, "1001xxx111xxxxxx", vmtest_test_l/*vmtest_sub_l_an*/);
        SetTable(rx_table, "1001xxx100000xxx", vmnotest/*vmtest_subx_b_r*/);
        SetTable(rx_table, "1001xxx101000xxx", vmnotest/*vmtest_subx_w_r*/);
        SetTable(rx_table, "1001xxx110000xxx", vmnotest/*vmtest_subx_l_r*/);
        SetTable(rx_table, "1001xxx100001xxx", vmtest_test_b/*vmtest_subx_b_m*/);
        SetTable(rx_table, "1001xxx101001xxx", vmtest_test_w/*vmtest_subx_w_m*/);
        SetTable(rx_table, "1001xxx110001xxx", vmtest_test_l/*vmtest_subx_l_m*/);
        SetTable(rx_table, "1011xxx000xxxxxx", vmtest_test_b/*vmtest_cmp_b*/);
        SetTable(rx_table, "1011xxx001xxxxxx", vmtest_test_w/*vmtest_cmp_w*/);
        SetTable(rx_table, "1011xxx010xxxxxx", vmtest_test_l/*vmtest_cmp_l*/);
        SetTable(rx_table, "1011xxx000000xxx", vmnotest/*vmtest_cmp_b_dn*/);
        SetTable(rx_table, "1011xxx000101xxx", vmnotest/*vmtest_cmp_b_dan*/);
        SetTable(rx_table, "1011xxx001000xxx", vmnotest/*vmtest_cmp_w_dn*/);
        SetTable(rx_table, "1011xxx010000xxx", vmnotest/*vmtest_cmp_l_dn*/);
        SetTable(rx_table, "1011xxx011xxxxxx", vmtest_test_w/*vmtest_cmpa_w*/);
        SetTable(rx_table, "1011xxx111xxxxxx", vmtest_test_l/*vmtest_cmpa_l*/);
        SetTable(rx_table, "1011xxx111001xxx", vmnotest/*vmtest_cmpa_l_an);
        SetTable(rx_table, "1011xxx100xxxxxx", vmtest_neg_b/*vmtest_eor_b*/);
        SetTable(rx_table, "1011xxx101xxxxxx", vmtest_neg_w/*vmtest_eor_w*/);
        SetTable(rx_table, "1011xxx110xxxxxx", vmtest_neg_l/*vmtest_eor_l*/);
	/**/
       SetTable(rx_table, "1011xxx100001xxx", vmtest_cmpm_b);
        SetTable(rx_table, "1011xxx101001xxx", vmtest_cmpm_w);
        SetTable(rx_table, "1011xxx110001xxx", vmtest_cmpm_l);
        SetTable(rx_table, "1100xxx000xxxxxx", vmtest_test_b/*vmtest_and_b_dn*/);
        SetTable(rx_table, "1100xxx001xxxxxx", vmtest_test_w/*vmtest_and_w_dn*/);
        SetTable(rx_table, "1100xxx010xxxxxx", vmtest_test_l/*vmtest_and_l_dn*/);
        SetTable(rx_table, "1100xxx010000xxx", wmnotest/*vmtest_and_l_dn_dn*/);
        SetTable(rx_table, "1100xxx100xxxxxx", vmtest_neg_b/*vmtest_and_b_ea*/);
        SetTable(rx_table, "1100xxx101xxxxxx", vmtest_neg_w/*vmtest_and_w_ea*/);
        SetTable(rx_table, "1100xxx110xxxxxx", vmtest_neg_l/*vmtest_and_l_ea*/);
        SetTable(rx_table, "1100xxx011xxxxxx", vmtest_test_w/*vmtest_mulu*/);
        SetTable(rx_table, "1100xxx10000xxxx", vmtest_addx_b_m/*vmtest_abcd*/);
        SetTable(rx_table, "1100xxx101000xxx", wmnotest/*vmtest_exg_d*/);
        SetTable(rx_table, "1100xxx101001xxx", wmnotest/*vmtest_exg_a*/);
        SetTable(rx_table, "1100xxx110001xxx", wmnotest/*vmtest_exg_ad*/);
        SetTable(rx_table, "1100xxx111xxxxxx", vmtest_test_w/*vmtest_muls*/);
        SetTable(rx_table, "1101xxx000xxxxxx", vmtest_test_b/*vmtest_add_b_dn*/);
        SetTable(rx_table, "1101xxx001xxxxxx", vmtest_test_w/*vmtest_add_w_dn*/);
        SetTable(rx_table, "1101xxx010xxxxxx", vmtest_test_l/*vmtest_add_l_dn*/);
        SetTable(rx_table, "1101xxx000000xxx", vmnotest/*vmtest_add_b_dn_dn*/);
        SetTable(rx_table, "1101xxx001000xxx", vmnotest/*vmtest_add_w_dn_dn*/);
        SetTable(rx_table, "1101xxx010000xxx", vmnotest/*vmtest_add_l_dn_dn*/);
        SetTable(rx_table, "1101xxx100xxxxxx", vmtest_neg_b/*vmtest_add_b_ea*/);
        SetTable(rx_table, "1101xxx101xxxxxx", vmtest_neg_w/*vmtest_add_w_ea*/);
        SetTable(rx_table, "1101xxx110xxxxxx", vmtest_neg_l/*vmtest_add_l_ea*/);
        SetTable(rx_table, "1101xxx011xxxxxx", vmtest_test_w/*vmtest_add_w_an*/);
        SetTable(rx_table, "1101xxx111xxxxxx", vmtest_test_l/*vmtest_add_l_an*/);
        SetTable(rx_table, "1101xxx011000xxx", vmnotest/*vmtest_add_w_an_dn*/);
        SetTable(rx_table, "1101xxx111000xxx", vmnotest/*vmtest_add_l_an_dn*/);
        SetTable(rx_table, "1101xxx100000xxx", vmnotest/*vmtest_addx_b_r*/);
        SetTable(rx_table, "1101xxx101000xxx", vmnotest/*vmtest_addx_w_r*/);
        SetTable(rx_table, "1101xxx110000xxx", vmnotest/*vmtest_addx_l_r*/);
        SetTable(rx_table, "1101xxx100001xxx", vmtest_addx_b_m);
        SetTable(rx_table, "1101xxx101001xxx", vmtest_addx_w_m);
        SetTable(rx_table, "1101xxx110001xxx", vmtest_addx_l_m);
	/**/
 

        SetTable(rx_table, "1110xxx000001xxx", vmnotest/*vmtest_lsr_b_i*/);
        SetTable(rx_table, "1110xxx100001xxx", vmnotest/*vmtest_lsl_b_i*/);
        SetTable(rx_table, "1110001000001xxx", vmnotest/*vmtest_lsr1_b*/);
        SetTable(rx_table, "1110001100001xxx", vmnotest/*vmtest_lsl1_b*/);
        SetTable(rx_table, "1110xxx001001xxx", vmnotest/*vmtest_lsr_w_i*/);
        SetTable(rx_table, "1110xxx101001xxx", vmnotest/*vmtest_lsl_w_i*/);
        SetTable(rx_table, "1110001001001xxx", vmnotest/*vmtest_lsr1_w*/);
        SetTable(rx_table, "1110001101001xxx", vmnotest/*vmtest_lsl1_w*/);
        SetTable(rx_table, "1110xxx010001xxx", vmnotest/*vmtest_lsr_l_i*/);
        SetTable(rx_table, "1110xxx110001xxx", vmnotest/*vmtest_lsl_l_i*/);
        SetTable(rx_table, "1110001010001xxx", vmnotest/*vmtest_lsr1_l*/);
        SetTable(rx_table, "1110001110001xxx", vmnotest/*vmtest_lsl1_l*/);
        SetTable(rx_table, "1110010110001xxx", vmnotest/*vmtest_lsl2_l*/);
        SetTable(rx_table, "1110xxx000101xxx", vmnotest/*vmtest_lsr_b_r*/);
        SetTable(rx_table, "1110xxx100101xxx", vmnotest/*vmtest_lsl_b_r*/);
        SetTable(rx_table, "1110xxx001101xxx", vmnotest/*vmtest_lsr_w_r*/);
        SetTable(rx_table, "1110xxx101101xxx", vmnotest/*vmtest_lsl_w_r*/);
        SetTable(rx_table, "1110xxx010101xxx", vmnotest/*vmtest_lsr_l_r*/);
        SetTable(rx_table, "1110xxx110101xxx", vmnotest/*vmtest_lsl_l_r*/);
        SetTable(rx_table, "1110xxx000000xxx", vmnotest/*vmtest_asr_b_i*/);
        SetTable(rx_table, "1110xxx100000xxx", vmnotest/*vmtest_asl_b_i*/);
        SetTable(rx_table, "1110xxx001000xxx", vmnotest/*vmtest_asr_w_i*/);
        SetTable(rx_table, "1110xxx101000xxx", vmnotest/*vmtest_asl_w_i*/);
        SetTable(rx_table, "1110xxx010000xxx", vmnotest/*vmtest_asr_l_i*/);
        SetTable(rx_table, "1110xxx110000xxx", vmnotest/*vmtest_asl_l_i*/);
        SetTable(rx_table, "1110xxx000100xxx", vmnotest/*vmtest_asr_b_r*/);
        SetTable(rx_table, "1110xxx100100xxx", vmnotest/*vmtest_asl_b_r*/);
        SetTable(rx_table, "1110xxx001100xxx", vmnotest/*vmtest_asr_w_r*/);
        SetTable(rx_table, "1110xxx101100xxx", vmnotest/*vmtest_asl_w_r*/);
        SetTable(rx_table, "1110xxx010100xxx", vmnotest/*vmtest_asr_l_r*/);
        SetTable(rx_table, "1110xxx110100xxx", vmnotest/*vmtest_asl_l_r*/);
        SetTable(rx_table, "1110xxx000010xxx", vmnotest/*vmtest_roxr_b_i*/);
        SetTable(rx_table, "1110xxx100010xxx", vmnotest/*vmtest_roxl_b_i*/);
        SetTable(rx_table, "1110xxx001010xxx", wmnotest/*vmtest_roxr_w_i*/);
        SetTable(rx_table, "1110xxx101010xxx", wmnotest/*vmtest_roxl_w_i*/);
        SetTable(rx_table, "1110xxx010010xxx", wmnotest/*vmtest_roxr_l_i*/);
        SetTable(rx_table, "1110xxx110010xxx", wmnotest/*vmtest_roxl_l_i*/);
        SetTable(rx_table, "1110xxx000110xxx", wmnotest/*vmtest_roxr_b_r*/);
        SetTable(rx_table, "1110xxx100110xxx", wmnotest/*vmtest_roxl_b_r*/);
        SetTable(rx_table, "1110xxx001110xxx", wmnotest/*vmtest_roxr_w_r*/);
        SetTable(rx_table, "1110xxx101110xxx", wmnotest/*vmtest_roxl_w_r*/);
        SetTable(rx_table, "1110xxx010110xxx", wmnotest/*vmtest_roxr_l_r*/);
        SetTable(rx_table, "1110xxx110110xxx", wmnotest/*vmtest_roxl_l_r*/);
        SetTable(rx_table, "1110xxx000011xxx", wmnotest/*vmtest_ror_b_i*/);
        SetTable(rx_table, "1110xxx100011xxx", wmnotest/*vmtest_rol_b_i*/);
        SetTable(rx_table, "1110xxx001011xxx", wmnotest/*vmtest_ror_w_i*/);
        SetTable(rx_table, "1110xxx101011xxx", wmnotest/*vmtest_rol_w_i*/);
        SetTable(rx_table, "1110xxx010011xxx", wmnotest/*vmtest_ror_l_i*/);
        SetTable(rx_table, "1110xxx110011xxx", wmnotest/*vmtest_rol_l_i*/);
        SetTable(rx_table, "1110xxx000111xxx", wmnotest/*vmtest_ror_b_r*/);
        SetTable(rx_table, "1110xxx100111xxx", wmnotest/*vmtest_rol_b_r*/);
        SetTable(rx_table, "1110xxx001111xxx", wmnotest/*vmtest_ror_w_r*/);
        SetTable(rx_table, "1110xxx101111xxx", wmnotest/*vmtest_rol_w_r*/);
        SetTable(rx_table, "1110xxx010111xxx", wmnotest/*vmtest_ror_l_r*/);
        SetTable(rx_table, "1110xxx110111xxx", wmnotest/*vmtest_rol_l_r*/);
        SetTable(rx_table, "1110000011xxxxxx", vmtest_test_w/*vmtest_asr_m*/); 
        SetTable(rx_table, "1110000111xxxxxx", vmtest_test_w/*vmtest_asl_m*/);
        SetTable(rx_table, "1110001011xxxxxx", vmtest_test_w/*vmtest_lsr_m*/);
        SetTable(rx_table, "1110001111xxxxxx", vmtest_test_w/*vmtest_lsl_m*/);
        SetTable(rx_table, "1110010011xxxxxx", vmtest_test_w/*vmtest_roxr_m*/);
        SetTable(rx_table, "1110010111xxxxxx", vmtest_test_w/*vmtest_roxl_m*/);
        SetTable(rx_table, "1110011011xxxxxx", vmtest_test_w/*vmtest_ror_m*/);
        SetTable(rx_table, "1110011111xxxxxx", vmtest_test_w/*vmtest_rol_m*/);
        SetTable(rx_table, "1010xxxxxxxxxxxx", wmnotest/*vmtest_code1010*/);
        SetTable(rx_table, "1111xxxxxxxxxxxx", wmnotest/*vmtest_code1111*/);
}
#endif
