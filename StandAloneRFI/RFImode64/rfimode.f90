program rfimode64
use hexprint
integer, parameter :: n=2048,nnode=16,nchan=64,ncorr=nchan*(nchan+1)/2,nred=1!6
complex, dimension(n/nnode,nnode,ncorr) :: rfimode
complex, dimension(n,ncorr) :: rfimode2
complex, dimension(nred,n/nred,ncorr) :: rfimode3
equivalence(rfimode,rfimode2)
equivalence(rfimode,rfimode3)
complex, dimension(n/nred,ncorr) :: rfimodered
real, dimension(n,ncorr) :: rval
complex, dimension(n+1) :: ctmp
real, dimension(2*n+2) :: rtmp
equivalence(ctmp,rtmp)
real, dimension(2*n,ncorr) :: flag
real, dimension(nchan,nchan) :: covar
integer, dimension(nchan) :: ipolmap,iantenna
integer, dimension(2,nchan/2) :: maptable
logical, parameter :: localshift=.false.
character*1 fn1,fni1
character*2 fn2
integer iktable(nchan,nchan)
integer ilag(1)
character*79 argv
complex cphase
real xy0(2)
real xy(2,nchan/2)
character*127 fnrfi ! svdrfi input file
character*127 fnpref ! prefix for output files
integer filenum
!data xy /687.88 ,  -21.19, 326.43 ,  -42.67, 0 ,  0, -372.72  , 141.29, -565.94  , 130.54, 67.82 , -260.80 , -31.44 , -233.29 , 280.67 , -423.40 , 41.92 , -160.39 , -164.88 , -621.34 , -603.28 , -340.09 , 174.85 , -672.16 , -639.53 ,  -1182.34 , -473.71 ,  -664.85 , -1591.94 ,  625.39, -3099.41 , 1501.18, -5199.90 , 3066.16 , -7039.03 , 5359.52, -8103.13 , 8267.57 , -11245.60 , 9430.02 , 2814.55 , 1008.62 , 4576.00 , 2043.81, 7780.69  ,  3070.58, 10200.00 , 3535.84 , 12073.46 , 4804.91 , 633.92 ,-2967.61 , -367.30 ,-4525.73 , 333.03 , -6774.02 , 947.68 , -9496.90 , -369.04 , -14153.59/
data xy /687.88 ,  -21.19, 326.43 ,  -42.67, 0 ,  0, -372.72  , 141.29, -565.94  , 130.54, 67.82 , -260.80 , -31.44 , -233.29 , 280.67 , -423.40 , 41.92 , -160.39 , -164.88 , -621.34 , -603.28 , -340.09 , 174.85 , -672.16 , -639.53 ,  -1182.34 , -473.71 ,  -664.85 , -1591.94 ,  625.39, -3099.41 , 1501.18, -5199.90 , 3066.16 , -7039.03 , 5359.52, -8103.13 , 8267.57 , -11245.60 , 9430.02 , 2814.55 , 1008.62 , 4576.00 , 2043.81, 7780.69  ,  3070.58, 10200.00 , 3535.84 , 12073.46 , 4804.91 , 633.92 ,-2967.61 , -367.30 ,-4525.73 , 333.03 , -6774.02 , 947.68 , -9496.90 , -369.04 , -14153.59, 0, 0, 0, 0 /

filenum=1

pi=4*atan(1.)
xy0=(/   5700   ,   13780   /) ! dec17.1
xy0=(/   4950    ,   13050   /) ! dec17.2
!xy0=(/  -540 ,  -2640       /) ! dec17.7, mode 3
xy0=(/   -100    ,   360   /) ! dec17.2
xy0=(/   -92.7    ,   387.9   /) ! rooftop calibrator source
xy0=(/   2213.4    ,   -562.7   /) ! dec 13 noise source  at well
xy0=(/   2540,1840   /) ! dec 17 noise source  north of E02

!ipolmap=0
!ipolmap(29:58)=1
!iantenna(:28)=(/(i,i=1,28)/)
!iantenna(29:58)=(/(i,i=1,30)/)
!iantenna(59:60)=(/29,30/)

ipolmap=0
ipolmap(33:64)=1
iantenna(:32)=(/(i,i=1,32)/)
iantenna(33:64)=(/(i,i=1,32)/)

!write(*,*) ipolmap
!write(*,*) iantenna




k=0
do i=1,nchan
   do j=i,nchan
      k=k+1
      iktable(i,j)=k
      iktable(j,i)=k
   end do
end do

do i=1,nchan
  maptable(ipolmap(i)+1,iantenna(i))=i
enddo
!modenum=1
!iscan=2
icount=iargc()
if (icount .ne. 3) then
   write(*,*) 'Usage: rfimode.f90 svdrfi outprefix mode'
   stop
endif
if (icount .eq. 3) then
   call getarg(3,argv)
   read(argv,*) modenum
   call getarg(1,fnrfi)
   call getarg(2,fnpref)
   !read(argv,*) iscan
   write(*,*) 'modenum=',modenum
end if
!write(fni1,'(i1)') iscan
do inode=1,nnode
   !write(fn1,'(Z1)') inode-1
   write(*,*) 'inode=',inode
   write(*,*) 'opening fnrfi: ', trim(adjustl(hexprintf(fnrfi,inode-1)))
   open(10,file=trim(adjustl(hexprintf(fnrfi,inode-1))),form='binary',status='old')
!   open(10,file='/mnt/a/pen/Template/rfi.svdrfi.node'//fn1,form='binary',status='old')
!   open(10,file='/cita/d/scratch-3week/paciga/Template/dec.14.07.b0823.'//fni1//'.svdrfi.v1.node'//fn1,form='binary',status='old')
   read(10) nvec
   if (inode .eq. 1) write(*,*) nvec
   do i=1,modenum
      read(10) rfimode(:,inode,:)
   enddo
enddo
rfimode=conjg(rfimode)

rval=abs(rfimode2)
covar=0
k=0
do i=1,nchan
   do j=i,nchan
      if ((iantenna(i).lt.15) .and. (iantenna(j).lt.15)) cycle
      if ((iantenna(i).gt.30) .or. (iantenna(j).gt.30)) cycle
      k=k+1
      delay=sqrt(sum((xy0-xy(:,iantenna(i)))**2))-sqrt(sum((xy0-xy(:,iantenna(j)))**2))
      do ifreq=1,n
!         if (mod(ifreq,128).gt.120) cycle
         freq=156-ifreq*16.66666667/n
         alambda=300/freq
         cphase=exp(-2*pi*(0.,1.)*delay/alambda)
         if (abs(delay)/9<2048) then
            cphase=cphase*4096/(4096-abs(delay)/9)
         else
            cphase=0
         end if
         if (localshift) rfimode2(ifreq,k)=rfimode2(ifreq,k)*cphase
      enddo
      covar(i,j)=sum(rval(:,k))
   end do
end do
write(fn2,'(I2.2)') modenum
write(*,*) "fn2=",fn2
if (localshift) then
   !   open(10,file='~/scr2/GMRT_RFI/Dec14/modeshift.dat.'//trim(adjustl(fn2)),form='binary')
   open(10,file=trim(trim(adjustl(fnpref))//'.modeshift.'//trim(adjustl(fn2))//'.dat'),form='binary')
   rfimodered=sum(rfimode3,1)
   write(10) rfimodered
end if
call pmap(trim(trim(adjustl(fnpref))//'.covar.'//trim(adjustl(fn2))//'.pgm'),covar,nchan,nchan,2,filenum)
rval=log(max(rval,maxval(abs(rval))/1.e6))
call pmap(trim(trim(adjustl(fnpref))//'.ampll.'//trim(adjustl(fn2))//'.pgm'),rval,n,ncorr,1,filenum)
rval=atan2(aimag(rfimode2),real(rfimode2))
call pmap(trim(trim(adjustl(fnpref))//'.phase.'//trim(adjustl(fn2))//'.pgm'),rval,n,ncorr,1,filenum)
do i=1,ncorr
   ctmp=0
   ctmp(:n)=rfimode2(:,i)
   !   ctmp=ctmp/abs(ctmp+1.e-30)
   call fft1(ctmp,rtmp,2*n,-1)
   rtmp(1)=0
   flag(:,i)=cshift(rtmp(:2*n),n)
enddo
close(10) !!!
open(11,file=trim(trim(adjustl(fnpref))//'.lag.'//trim(adjustl(fn2))//'.dat'),form='binary')
write(11) flag
close(11)!!!
if (localshift) then
   open(12,file=trim(trim(adjustl(fnpref))//'.lagshift.'//trim(adjustl(fn2))//'.dat'))
else
   open(12,file=trim(trim(adjustl(fnpref))//'.lagnoshift.'//trim(adjustl(fn2))//'.dat'))
end if
k=0
do i=1,nchan/2
   do j=i,nchan/2
      k=k+1
      if (i .eq. j) cycle
      do ipol=1,2
         do jpol=1,2            
            k=iktable(maptable(ipol,i),maptable(jpol,j))
            ilag=maxloc(abs(flag(:,k)))-n
            if (maptable(ipol,i) >maptable(jpol,j)) ilag=-ilag
            write(12,*) i,j,ilag,maxval(abs(flag(:,k)))
         end do
      end do
   end do
end do
close(12) !!!
call pmap(trim(trim(adjustl(fnpref))//'.flagg.'//trim(adjustl(fn2))//'.pgm'),flag,2*n,ncorr,2,filenum)



contains
  
  subroutine pmap(fn,rmap1,nx,ny,iscale,filenum)
    real rmap(nx,ny),rmap1(nx,ny)
    integer*1 imap(nx,ny)
    character*127 fn
    integer npix,mypos
    integer filenum ! any number for file number so not always 10
    
    npix=min(ny/2-1,nx/2-1,300)
    
    rmap=rmap1
    write(*,*) 'rms=',sqrt(sum(rmap(nx/2-npix:nx/2+npix,ny/2-npix:ny/2+npix)**2)/npix**2/4)
    if (iscale .eq. 2) rmap=sign(sqrt(abs(rmap)),rmap)
    
    rmax=maxval(rmap)
    rmin=minval(rmap)
    !  write(*,*) trim(adjustl(fn)),rmax,rmin
    imap=127*(rmap-rmin)/(rmax-rmin)
    open(80+filenum,file=trim(adjustl(fn)))
    write(80+filenum,'(2hP5)')
    write(80+filenum,*)nx,ny
    write(80+filenum,*) 127
    !  INQUIRE(UNIT=10, POS=mypos)
    close(80+filenum)
    open(80+filenum,file=trim(adjustl(fn)), form='binary',access='append')
    !  write(10,pos=mypos) imap
    write(80+filenum) imap
    close(80+filenum)
    
    filenum=filenum+1
    
  end subroutine pmap

end program rfimode64
