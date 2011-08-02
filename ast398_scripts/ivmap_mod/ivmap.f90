! make polarization maps at different rotation measures
!
implicit none
integer, parameter  :: nchan=60,n=64,ngrid=1024, ndim=3,nl=2,nt=448!7+10-2
complex, dimension(ngrid,ngrid,nl) :: uvgridqu,uvgridi
real, dimension(ngrid,ngrid,nl) :: varnoise
real, dimension(ngrid,ngrid) :: rmap
complex, dimension(ngrid,ngrid) :: uv1

integer, dimension(ngrid,ngrid,nl) :: hoc
complex, dimension(n) :: coneline
real, dimension(n) :: oneline
!real, parameter :: freq0=156, dfreq=-16.666666/n,umaxcut=200,umax=umaxcut!12000/2
real, parameter :: freq0=156, dfreq=-16.666666/n, umaxcutdefault=200,umaxdefault=umaxcutdefault
real :: umaxcut,umax
real fmax,dfgrid,u,v,fmin,uthis,vthis,utot, freq, rlambda2,amplitude,sn,sigmacut,varamplitude,anorm,w
integer it,i,j,ifreq,il,iu,iv,ib,jb,icountcut
integer, dimension(ndim)::nn
character*80 fn
character*20 fn1
integer, dimension(2) :: mloc2
integer, dimension(nchan) :: ipolmap,iantenna
integer, dimension(2,nchan/2) :: maptable
logical dofilter

! for dynamic range calculation (slightly redundant with pmap routine)
real peak_flux, rms_ivimap
integer bpix, nbpix, npix


! read in arguments
character*8 umaxarg
character*8 filterarg
character*8 umaxcutarg

logical,dimension(nchan/2,nt,n) :: la3_mastermask
write(*,*) "Defining master mask"
call definemask(nchan,nt,n,la3_mastermask)



! umaxcut refers to the uv range used
! umax refers to the image size

call getarg(1,umaxcutarg)
call getarg(3,filterarg)
call getarg(2,umaxarg)

if ( LEN(TRIM(umaxcutarg)) > 0 ) then
   ! set umaxcut to the command line argument
   read(umaxcutarg,*) umaxcut
else
   umaxcut=umaxcutdefault
endif

if ( LEN(TRIM(umaxarg)) > 0 ) then
   read(umaxarg,*) umax
else
   umax=umaxdefault
endif

write(*,*) "umaxcut=", umaxcut
write(*,*) "umax=", umax

if ( LEN(TRIM(filterarg)) > 0 ) then
   dofilter=.true.
else
   dofilter=.false.
endif
write(*,*) "dofilter=", dofilter, filterarg

ipolmap=0
ipolmap(29:58)=1
iantenna(:28)=(/(i,i=1,28)/)
iantenna(29:58)=(/(i,i=1,30)/)
iantenna(59:60)=(/29,30/)

do i=1,nchan
  maptable(ipolmap(i)+1,iantenna(i))=i
enddo


varamplitude=109173.2*10000

fmax=156			! MHz
fmin=fmax-16.666666666		! MHz

dfgrid=(fmax-fmin)/nl


!open(10,file='datacalall.dat.pulsar',form='binary',status='old')
!open(10,file='clean1.dat.all',form='binary',status='old')
open(10,file='datacalall.dat',form='binary',status='old')
!open(10,file='dataQUpeel_14.dat',form='binary',status='old')
!open(10,file='peelall.dat',form='binary',status='old')
!open(10,file='uvpoint.peelall.dat.0',form='binary',status='old')
!open(10,file='3c200suv.dat',form='binary',status='old')
!open(15,file='noisecaldiag.dat.pulsar',form='binary',status='old')
open(15,file='noisecaldiag.dat',form='binary',status='old')
open(20,file='generic.mat',status='old')
uvgridqu=0
varnoise=0
icountcut=0
do it=1,nt
   do i=1,nchan
      do j=i,nchan
         read(20,*) ib,jb,u,v,w
!         if (it .eq. 56) cycle
         read(10) coneline
         if ( dofilter )then
            if ( filterarg == "old") then
               call old_filter(coneline,n,8) ! subtract mean (foreground)
            else
               call filter(coneline,n,8)
            endif
         endif
!         coneline=coneline-0.00225*1.05
         read(15) oneline
        coneline(:4)=0
!	oneline=1

        !if (it.gt.190.and.it.le.201) cycle
        !if (it.gt.201.and.it.le.430) cycle
        if (it.gt.430.and.it.le.448) cycle
       ! if (iantenna(i).gt. 10.and.iantenna(i).le.14)cycle 
        if (iantenna(i).eq.3.or.iantenna(j).eq.3)cycle
        if (iantenna(i).eq.5.or.iantenna(j).eq.5)cycle 
        if (iantenna(i).eq.11.or.iantenna(j).eq.11)cycle
        if (iantenna(i).eq.26.or.iantenna(j).eq.26)cycle
        
        if (iantenna(i).eq.18.or.iantenna(j).eq.18)cycle
       ! if (iantenna(i).eq.49.or.iantenna(j).eq.49)cycle
       ! if (iantenna(i).eq.50.or.iantenna(j).eq.50)cycle
       ! if (iantenna(i).eq.51.or.iantenna(j).eq.51)cycle
       ! if (iantenna(i).eq.52.or.iantenna(j).eq.52)cycle

         if (i .eq. j) cycle
         if (ipolmap(i) .ne. ipolmap(j)) cycle
!         if (ipolmap(i) .eq. ipolmap(j)) cycle
!         if (it<320) cycle
!         if (it<4*56) cycle
!         if (it>5*56) cycle
         !if (it>334) cycle
!         if (it<234) cycle
!         if (it>334) cycle
!         if (it>240 .and. it<390) cycle

!         if (abs(w)>200) cycle

!	 icountcut=icountcut+count(abs(coneline)**2>varamplitude*oneline)
!         where (abs(coneline)**2>varamplitude*oneline) 
!		coneline=0
!		oneline=0
!	 endwhere
         do ifreq=1,n
           ! if (ifreq.gt.1.and.ifreq.le.30) cycle
            if (ifreq.gt.1.and.ifreq.le.6) cycle
            if (ifreq.gt.22.and.ifreq.le.28) cycle 

	    freq=(ifreq-0.5)*dfreq+freq0
            uthis=u*freq/freq0
            vthis=v*freq/freq0
            if (abs(vthis)<20) cycle

            if (ipolmap(i) .eq. 1) then  ! LL=I+V RR=I-V
		uthis=-uthis  ! map rr into ll
		vthis=-vthis
		coneline(ifreq)=conjg(coneline(ifreq))  ! the real rmap will be Q
! for purely imaginary map i U(x), iU(k)=i\int dk exp(-i k x)  U(x)
! so iU(-k) = i conjg U(k)
! this is like reversing the sign on the real component.
	    endif
	    utot=sqrt(uthis**2+vthis**2)
!	    if (utot<1000 .or. utot>umax*2) cycle !!!
	    if (utot>umaxcut) cycle
            iu=nint(uthis*ngrid/2/umax+ngrid/2)+1
            iv=nint(vthis*ngrid/2/umax+ngrid/2)+1
            if (iu<1 .or. iu>ngrid .or. iv<1 .or. iv>ngrid) cycle
	    il=(freq-fmin)/dfgrid+1
	    if ( il<1 .or. il>nl ) then
		write(*,*) il,ifreq
	    endif
            if (iv.eq.548.and.iu.eq.144) write(*,*) it, iantenna(i), iantenna(j), ifreq
               uvgridqu(iu,iv,il)=uvgridqu(iu,iv,il)+coneline(ifreq)/oneline(ifreq)
               varnoise(iu,iv,il)=varnoise(iu,iv,il)+1/oneline(ifreq)
         enddo
      enddo
   enddo
enddo
write(*,*) 'cut',icountcut,' out of ',nt*n*nchan*(nchan+1)/2/2
sigmacut=50
sn=sum(abs(uvgridqu)**2)/sum(varnoise)
write(*,*) 'S/N=',sn
write(*,*) sigmacut,' sigma=',count(abs(uvgridqu)**2>sigmacut**2*sn*varnoise)
write(*,*) 'total count=',count(abs(uvgridqu)>0)
where(abs(uvgridqu)**2>sigmacut**2*sn*varnoise)
        uvgridqu=0
        varnoise=0
endwhere
anorm=sum(varnoise)
write(*,*) 'norm=',anorm
write(*,*) 'pulsar flux=',sum(uvgridqu)/anorm
sn=sum(abs(uvgridqu)**2)/anorm
write(*,*) 'S/N=',sn
uvgridqu=uvgridqu/sum(varnoise)
rmap=sum(real(uvgridqu),3)
fn='ivvis_re.pgm'
call pmap(fn,rmap,ngrid,ngrid,2)
rmap=sum(aimag(uvgridqu),3)
fn='ivvis_im.pgm'
call pmap(fn,rmap,ngrid,ngrid,2)

nn=ngrid
nn(3)=nl
uvgridqu=cshift(uvgridqu,ngrid/2,1)
uvgridqu=cshift(uvgridqu,ngrid/2,2)
call fourn(uvgridqu,nn,ndim,-1)
uvgridi=uvgridqu
uvgridqu=cshift(uvgridqu,ngrid/2,1)
uvgridqu=cshift(uvgridqu,ngrid/2,2)
rmap=transpose(real(uvgridqu(:,:,1)))
write(*,*) 'peaks='
do i=1,10*0
mloc2=maxloc(rmap)
write(*,*) mloc2,rmap(mloc2(1),mloc2(2))/anorm
rmap(mloc2(1)-3:mloc2(1)+3,mloc2(2)-3:mloc2(2)+3)=0
enddo
do i=1,nl
write(fn1,'(i20)') i
rmap=transpose(real(uvgridqu(:,:,i)))
fn='ivrmap'//trim(adjustl(fn1))//'.pgm'
call pmap(fn,rmap,ngrid,ngrid,1)

! before going on, get the peak value of the ivrmap
if (abs(maxval(rmap)).gt.abs(minval(rmap))) then 
   peak_flux = abs(maxval(rmap))
else 
   peak_flux = abs(minval(rmap))
end if
write(*,*) 'peak_flux = ', peak_flux

rmap=transpose(aimag(uvgridqu(:,:,i)))
fn='ivimap'//trim(adjustl(fn1))//'.pgm'
call pmap(fn,rmap,ngrid,ngrid,1)

! and now get the rms of the ivimap around the edge
npix = min(ngrid/2-1,109) ! for umax=2667
bpix = min(ngrid/4,npix)
nbpix = 4*bpix*ngrid - 4*bpix**2
! sum whole map squared, minus sum inner square squared to get sum of edge squared, divide by number of pixels in the border, and divide by 4
rms_ivimap=sqrt((sum(rmap(:,:)**2)-sum(rmap(bpix:ngrid-bpix,bpix:ngrid-bpix)**2))/nbpix/4.0)
write(*,*) 'rms(border, ivimap)=', rms_ivimap 
write(*,*) 'IVRMAP / RMS_IVIMAP DYNAMIC RANGE:', peak_flux/rms_ivimap

uv1=uvgridi(:,:,i)
call fourn(uv1,nn,2,1)
uv1=cshift(uv1,ngrid/2,1)
uv1=cshift(uv1,ngrid/2,2)
rmap=real(transpose(uv1))
fn='vismapre'//trim(adjustl(fn1))//'.pgm'
call pmap(fn,rmap,ngrid,ngrid,2)
rmap=aimag(transpose(uv1))
fn='vismapim'//trim(adjustl(fn1))//'.pgm'
call pmap(fn,rmap,ngrid,ngrid,2)
enddo


end


subroutine pmap(fn,rmap,nx,ny,iscale)
  real rmap(nx,ny)
  integer*1 imap(nx,ny)
  character*80 fn
  integer npix,mypos

  npix=min(nx/2-1,10)
  
  
  write(*,*) 'rms=',sqrt(sum(rmap(nx/2-npix:nx/2+npix,ny/2-npix:ny/2+npix)**2)/npix**2/4)
  if (iscale .eq. 2) rmap=sign(sqrt(abs(rmap)),rmap)
  
  rmax=maxval(rmap)
  rmin=minval(rmap)
  write(*,*) trim(fn),rmax,rmin,sqrt(sum(rmap(nx/2-npix:nx/2+npix,ny/2-npix:ny/2+npix)**2)/npix**2/4)/rmax
  imap=127*(rmap-rmin)/(rmax-rmin)
  open(10,file=fn)
  write(10,'(2hP5)')
  write(10,*)nx,ny
  write(10,*) 127
!  INQUIRE(UNIT=10, POS=mypos)
  close(10)
!  open(10,file=fn, access='stream')
  open(10,file=fn, access='append', form='binary')
  write(10) imap
  close(10)
end

!old filter subroutine
subroutine old_filter(c,n,nred)
complex, dimension(n/nred,nred) :: c
complex ctmp

do i=1,nred
call old_rmedian(ctmp,c(1,i),n/nred,2)
c(:,i)=c(:,i)-ctmp
enddo
end subroutine old_filter

subroutine old_rmedian(a,b,nt,n)
real, dimension(n,nt) :: b
real, dimension(n) :: a
real, dimension(nt) :: t
integer, dimension(nt) :: iorder


!  a=sum(b,2)/nt
!  return
  do in=1,n
     t=b(in,:)
     call quick_sort(t,iorder,nt)
     if (mod(nt,2) .eq. 0) then
        a(in)=(t(nt/2)+t(nt/2+1))/2
     else
        a(in)=t((nt+1)/2)
     endif
  end do
end subroutine old_rmedian


!new filter subroutine
subroutine filter(c,n,nred)
implicit none
integer n,nred
complex, dimension(n/nred,nred) :: c
logical, dimension(n/nred,nred) :: lmask
complex ctmp(n/nred)
!locals
integer i,j,nj
real dx,w

lmask=c.eq.0

do i=1,nred
   call rmedian(ctmp(i),c(1,i),n/nred,2)
!   c(:,i)=c(:,i)-ctmp(i)
end do
!return

nj=n/nred/2
i=1
do j=1,nj
   dx=(2*nj-j-0.5)/nj
   w=(dx+1)/2
!   w=1
   c(j,i)=c(j,i)-(ctmp(i)*w+ctmp(i+1)*(1-w))
end do
i=nred
do j=nj+1,2*nj
   dx=(j-.5)/nj
   w=(1-dx)/2
!   w=0
   c(j,i)=c(j,i)-(ctmp(i-1)*w+ctmp(i)*(1-w))
end do
do i=1,nred
   if (i>1) then
      do j=1,nj
         dx=(j-.5)/nj
         w=(1-dx)/2
!         w=0
         c(j,i)=c(j,i)-(ctmp(i-1)*w+ctmp(i)*(1-w))
      end do
   endif
   if (i .eq. nred) cycle
   do j=nj+1,2*nj
      dx=(2*nj-j-0.5)/nj
      w=(dx+1)/2
!      w=1
      c(j,i)=c(j,i)-(ctmp(i)*w+ctmp(i+1)*(1-w))
   end do
   
enddo
where(lmask)c=0
end

subroutine rmedian(a,b,nt,n)
real, dimension(n,nt) :: b
real, dimension(n) :: a
real, dimension(nt) :: t
integer, dimension(nt) :: iorder


!  a=sum(b,2)
!  return
  do in=1,n
     j=0
     do i=1,nt
        if (b(in,i) .ne. 0) then
           j=j+1
           t(j)=b(in,i)
        endif
     end do
     if (j .eq. 0) then
!        write(*,*) 'empty data ',in
        a(in)=0
        cycle
     end if
!     t=b(in,:)
     call quick_sort(t,iorder,j)
     if (mod(j,2) .eq. 0) then
        a(in)=(t(j/2)+t(j/2+1))/2
     else
        a(in)=t((j+1)/2)
     endif
  end do
end subroutine rmedian

!=======
!== Defines mask for chosen antenna, timestamp, and freq
!== See file: 'ivmap.params' which must be present
!== written by Joshua Albert
!=======
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
