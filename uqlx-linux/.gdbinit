# (c) 1998,1999 Richard Zidlicky
# GDB init for UQLX

# on some systems SIGSEGV is the normal way of 
# handling IO - no exception
 
handle SIGSEGV nostop noprint

# switch on/off 50 Hz timer alarm. Having it 'on' makes single
# stepping impossible but in some situations it must be enabled
# to let QDOS proceed

define ton
 handle SIGALRM nostop noprint pass
end

define toff
 handle SIGALRM nostop noprint nopass
end

# COMMAND 'qldbg', documented in docs/uqlx.texi

define qldbg
 set exception=(32+14)
 set extraFlag=1
 set nInst2=nInst
 set nInst=0
 set doPoll=0
 continue
end
