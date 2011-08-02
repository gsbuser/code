program ivmap
implicit none
integer,parameter :: nchan=60,nt=448,nfreq=64
logical,dimension(nchan/2,nt,nfreq) :: la3_mastermask
integer a,t,f
write(*,*) "Defining master mask"
call definemask(nchan,nt,nfreq,la3_mastermask)
write(*,*) "Checking master mask"


do a=1,nchan/2
  do t=1,nt
    do f=1,nfreq
     !write(*,*)"a,t,f",a,",",t,",",f," - ",la3_mastermask(a,t,f)
    end do
  end do
end do

end program ivmap 

subroutine definemask(nchan,nt,nfreq,la3_mastermask)
integer :: nchan,nt,nfreq
integer :: a,t,f,pos,pos_,i,j
logical,dimension(nchan/2,nt,nfreq) :: la3_mastermask
logical,dimension(nchan/2) :: la1_antennamask
logical,dimension(nt) :: la1_timestampmask
logical,dimension(nfreq) :: la1_freqmask
logical :: ant,tim,frq
character(256) :: s_line,s_modline

write(*,*) "Initializing master mask (check)"
do a=1,nchan/2
  do t=1,nt
    do f=1,nfreq
      la3_mastermask(a,t,f)=.false.
    end do
  end do
end do
write(*,*) "opening ivmap.params"
open(111,file='ivmap.params',status='old')
do 
  read(111,'(1A)',end=222) s_line
  s_modline=ADJUSTL(TRIM(s_line))
  if (s_modline(1:1).eq."#") goto 888
  write(*,*) "Processing line, '",TRIM(s_line),"'"
  write(*,*) "Initializing individual range arrays."
  ! Initializing line masks
  do a=1,nchan/2
    do t=1,nt
      do f=1,nfreq
        la1_antennamask(a)=.false.
        la1_timestampmask(t)=.false.
        la1_freqmask(f)=.false.
      end do
    end do
  end do
  ant=.true.
  tim=.true.
  frq=.true.
  write(*,*) "Grabbing conditional ranges."
  ! Antenna part
  write(*,*) "Finding '[A]{' in ",TRIM(s_modline)
  pos=INDEX(s_modline,"[A]{")
  if (pos.eq.0) then
    write(*,*) "FORMAT ERROR: Couldn't find '[A]{': ignoring, '",TRIM(s_line),"'"
    goto 888
  end if
  write(*,*) "Found '[A]{'"
  s_modline=s_modline(pos+4:)
  write(*,*) "Finding '}' in ",TRIM(s_modline)
  pos=INDEX(s_modline,"}")
  pos_=pos
  if (pos.eq.0) then
    write(*,*) "FORMAT ERROR: Couldn't find '}': ignoring, '",TRIM(s_line),"'"
    goto 888
  else if (pos.eq.1) then
    write(*,*) "Found '}'. No antenna range. Jumping to timestamps"
    do a=1,nchan/2
      la1_antennamask(a)=.true.
    end do
    ant=.false.
    goto 333
  else ! Look for '-'
    write(*,*) "Looking for '-' in ", TRIM(s_modline)
    pos=INDEX(s_modline(:pos-1),"-")
    if (pos.eq.0) then ! seperate values
      write(*,*) "Could not find '-' in antenna range. Moving to comma seperated values."
      goto 233
    else if (pos.eq.1) then ! ALL
      write(*,*) "Found '-' only. Marking whole antenna range. Moving to timestamps."
      do a=1,nchan/2
        la1_antennamask(a)=.true.
      end do
      goto 333
    else
      write(*,*) "Found '-'. Getting numbers on either side of '-'."
      read(s_modline(:pos-1),'(I10)') i
      read(s_modline(pos+1:pos_-1),'(I10)') j
      write(*,*) "Found: ",i,"-",j,". Moving to timestamps."
      do a=i,j
        la1_antennamask(a)=.true.
      end do
      goto 333
    end if
  end if
  ! Seperate values
233 write(*,*) "Finding comma seperated values."
  do
    pos=INDEX(s_modline,",")
    if (pos.eq.0) then
      pos=INDEX(s_modline,"}")
      read(s_modline(:pos-1),'(I10)') i
      la1_antennamask(i)=.true.
      write(*,*) "Found, ",i,". No more commas. Moving to timestamps"
      goto 333
    else if (pos.eq.1) then 
      write(*,*) "FORMAT ERROR: Can't have ',' before number: ignoring, '",TRIM(s_line),"'"
      goto 888
    else
      read(s_modline(:pos-1),'(I10)') i
      la1_antennamask(i)=.true.
      write(*,*) "Found, ",i,". Cutting it off of, '",TRIM(s_modline),"'"
      s_modline=s_modline(pos+1:)
    end if 
  end do
  
  
  ! Timestamp part
333 write(*,*) "Finding '[T]{' in ",TRIM(s_modline)
  pos=INDEX(s_modline,"[T]{")
  if (pos.eq.0) then
    write(*,*) "FORMAT ERROR: Couldn't find '[T]{': ignoring, '",TRIM(s_line),"'"
    goto 888
  end if
  write(*,*) "Found '[T]{'"
  s_modline=s_modline(pos+4:)
  write(*,*) "Finding '}' in ",TRIM(s_modline)
  pos=INDEX(s_modline,"}")
  pos_=pos
  if (pos.eq.0) then
    write(*,*) "FORMAT ERROR: Couldn't find '}': ignoring, '",TRIM(s_line),"'"
    goto 888
  else if (pos.eq.1) then
    write(*,*) "Found '}'. No timestamp range. Jumping to freqs"
    do t=1,nt
      la1_timestampmask(t)=.true.
    end do
    tim=.false.
    goto 444
  else ! Look for '-'
    write(*,*) "Looking for '-' in ", TRIM(s_modline)
    pos=INDEX(s_modline(:pos-1),"-")
    if (pos.eq.0) then ! seperate values
      write(*,*) "Could not find '-' in timestamp range. Moving to comma seperated values."
      goto 344
    else if (pos.eq.1) then ! ALL
      write(*,*) "Found '-' only. Marking whole timestamp range. Moving to freqs."
      do t=1,nt
        la1_timestampmask(t)=.true.
      end do
      goto 444
    else
      write(*,*) "Found '-'. Getting numbers on either side of '-'."
      read(s_modline(:pos-1),'(I10)') i
      read(s_modline(pos+1:pos_-1),'(I10)') j
      write(*,*) "Found: ",i,"-",j,". Moving to freqs."
      do t=i,j
        la1_timestampmask(t)=.true.
      end do
      goto 444
    end if
  end if
  ! Seperate values
344 write(*,*) "Finding comma seperated values."
  do
    pos=INDEX(s_modline,",")
    if (pos.eq.0) then
      pos=INDEX(s_modline,"}")
      read(s_modline(:pos-1),'(I10)') i
      la1_timestampmask(i)=.true.
      write(*,*) "Found, ",i,". No more commas. Moving to freqs"
      goto 444
    else if (pos.eq.1) then 
      write(*,*) "FORMAT ERROR: Can't have ',' before number: ignoring, '",TRIM(s_line),"'"
      goto 888
    else
      read(s_modline(:pos-1),'(I10)') i
      la1_timestampmask(i)=.true.
      write(*,*) "Found, ",i,". Cutting it off of, '",TRIM(s_modline),"'"
      s_modline=s_modline(pos+1:)
    end if 
  end do


  ! Freq part
444 write(*,*) "Finding '[F]{' in ",TRIM(s_modline)
  pos=INDEX(s_modline,"[F]{")
  if (pos.eq.0) then
    write(*,*) "FORMAT ERROR: Couldn't find '[F]{': ignoring, '",TRIM(s_line),"'"
    goto 888
  end if
  write(*,*) "Found '[F]{'"
  s_modline=s_modline(pos+4:)
  write(*,*) "Finding '}' in ",TRIM(s_modline)
  pos=INDEX(s_modline,"}")
  pos_=pos
  if (pos.eq.0) then
    write(*,*) "FORMAT ERROR: Couldn't find '}': ignoring, '",TRIM(s_line),"'"
    goto 888
  else if (pos.eq.1) then
    write(*,*) "Found '}'. No freq range. Jumping to condensing statements."
    do f=1,nfreq
      la1_freqmask(f)=.true.
    end do
    frq=.false.
    goto 799
  else ! Look for '-'
    write(*,*) "Looking for '-' in ", TRIM(s_modline)
    pos=INDEX(s_modline(:pos-1),"-")
    if (pos.eq.0) then ! seperate values
      write(*,*) "Could not find '-' in freq range. Moving to comma seperated values."
      goto 455
    else if (pos.eq.1) then ! ALL
      write(*,*) "Found '-' only. Marking whole freq range. Moving to codnensing statements."
      do f=1,nfreq
        la1_freqmask(f)=.true.
      end do
      goto 799
    else
      write(*,*) "Found '-'. Getting numbers on either side of '-'."
      read(s_modline(:pos-1),'(I10)') i
      read(s_modline(pos+1:pos_-1),'(I10)') j
      write(*,*) "Found: ",i,"-",j,". Moving to mask ranges."
      do f=i,j
        la1_freqmask(f)=.true.
      end do
      goto 799
    end if
  end if
  ! Seperate values
455 write(*,*) "Finding comma seperated values."
  do
    pos=INDEX(s_modline,",")
    if (pos.eq.0) then
      pos=INDEX(s_modline,"}")
      read(s_modline(:pos-1),'(I10)') i
      la1_freqmask(i)=.true.
      write(*,*) "Found, ",i,". No more commas. Moving to condensing statements."
      goto 799
    else if (pos.eq.1) then 
      write(*,*) "FORMAT ERROR: Can't have ',' before number: ignoring, '",TRIM(s_line),"'"
      goto 888
    else
      read(s_modline(:pos-1),'(I10)') i
      la1_freqmask(i)=.true.
      write(*,*) "Found, ",i,". Cutting it off of, '",TRIM(s_modline),"'"
      s_modline=s_modline(pos+1:)
    end if 
  end do

 ! Condense info to mask
799 write(*,*) "Condensing the input into mast mask"
  ! Condtions list
  if (.not.ant.and..not.tim.and..not.frq) then
    write(*,*) "All conditions blanks. Goodbye sweet mind."
    goto 888
  end if
  do a=1,nchan/2 
    do t=1,nt
      do f=1,nfreq
        if (la1_freqmask(f).and.la1_timestampmask(t).and.la1_antennamask(a)) la3_mastermask(a,t,f)=.true.
      end do
    end do
  write(*,*) ceiling(200.*a/nchan),"% done"
  end do       
888 cycle
222 exit
end do
write(*,*) "closing ivmap.params"
close(111)
do a=1,nchan/2 
  do t=1,nt
    do f=1,nfreq
      !write(*,*)"a,t,f",a,",",t,",",f," - ",la3_mastermask(a,t,f)
      !write(*,*) la3_mastermask(a,t,f)
    end do
  end do
end do       

end subroutine definemask
