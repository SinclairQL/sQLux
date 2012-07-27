main(argc,argv)
     char *argv[];
{
  int i=0;
  
  while(argc-->0) printf("arg[%d]= %s\n",i,argv[i++]);
}
