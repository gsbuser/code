#!/usr/bin/awk -f

# With help from http://mydebian.blagdns.org/?p=242

BEGIN{
  ialphabet="abcdefghijklmnopqrstuvwxyz";
  oalphabet="0123456789";

  ibase=length(ialphabet);
  obase=length(oalphabet);

  if (num) {
    convert(num);
    exit 0;
  }
}

// {
  convert($1)
    }

function convert(num) {
  for (i=1;i<=length(num);i++) {
    number+=(index(ialphabet,substr(num,i,1))-1)*(ibase^(length(num)-i));
  }
  tmp=number;

  while(tmp>=obase) {
    nut=substr(oalphabet,tmp%obase+1,1) final;
    final = nut final;
    tmp=int(tmp/obase);
  }
  final=substr(oalphabet,tmp%obase+1,1) final;

  printf("%02d\n",final);
}

