integer, parameter :: nx=500,dx=30,nlag=4096,nchan=60,ncorr=nchan*(nchan+1)/2
!integer, parameter :: nx=1000,dx=5,nlag=4096,nchan=60,ncorr=nchan*(nchan+1)/2
real, dimension(-nx:nx,-nx:nx) :: chimap
real, dimension(2,30) :: xy
integer, dimension(2) :: locchimin
real, dimension(nlag,ncorr) :: flag
integer, dimension(nchan) :: ipolmap,iantenna,idifflag
integer, dimension(3,nchan) :: idifflagraw
character*255 fnlag,fnpref
character*2 fn2
character*79 argv
data xy /687.88 ,  -21.19, 326.43 ,  -42.67, 0 ,  0, -372.72  , 141.29, -565.94  , 130.54, 67.82 , -260.80 , -31.44 , -233.29 , 280.67 , -423.40 , 41.92 , -160.39 , -164.88 , -621.34 , -603.28 , -340.09 , 174.85 , -672.16 , -639.53 ,  -1182.34 , -473.71 ,  -664.85 , -1591.94 ,  625.39, -3099.41 , 1501.18, -5199.90 , 3066.16 , -7039.03 , 5359.52, -8103.13 , 8267.57 , -11245.60 , 9430.02 , 2814.55 , 1008.62 , 4576.00 , 2043.81, 7780.69  ,  3070.58, 10200.00 , 3535.84 , 12073.46 , 4804.91 , 633.92 ,-2967.61 , -367.30 ,-4525.73 , 333.03 , -6774.02 , 947.68 , -9496.90 , -369.04 , -14153.59/
iantenna(:28)=(/(i,i=1,28)/)
iantenna(29:58)=(/(i,i=1,30)/)
iantenna(59:60)=(/29,30/)

icount=iargc()
if (icount .ne. 3) then
   write(*,*) 'Usage: rfiposlag.x fnlag outprefix'
   stop
endif
write(*,*) 'nx=',nx,'dx=',dx
if (icount .eq. 3) then
   call getarg(1,fnlag)
   call getarg(2,fnpref)
   call getarg(3,argv)
   read(argv,*) modenum
   write(*,*) 'fnlag=',trim(adjustl(fnlag))
endif
write(fn2,'(I2.2)') modenum


open(10,file='delaydiff3.dat',status='old')
read(10,*) idifflagraw
idifflag=idifflagraw(2,:)

!do wcut=0.2,0.6,0.01
!do wcut=4000,5000,1000
locchimin=0
open(10,file=trim(adjustl(fnlag)),form='binary',status='old')
read(10) flag
!$omp parallel do default(shared) private(x,y,chi2,iy,i,j,k,delay,ilag)
do ix=-nx,nx
   if (mod(ix,100).eq. 0) write(*,*) ix
   do iy=-nx,nx
      x=ix*dx
      y=iy*dx
      chi2=0
      k=0
      do i=1,nchan
         do j=i,nchan
            k=k+1
            if (iantenna(i) < 15 .and. iantenna(j)<15) cycle
            delay=(sqrt(sum(((/x,y/)-xy(:,iantenna(i)))**2))-sqrt(sum(((/x,y/)-xy(:,iantenna(j)))**2)))/9
! delay is zero halfway, negative toward antenna i, positive toward antenna j
            if (abs(delay)>2000) cycle
            ilag=-delay+nlag/2
            ilag=ilag+idifflag(i)-idifflag(j)
! idifflag tends to be negative
! the new tables make the lag move towards antenna j
! if we had no delay compensation, and signals moved at the speed of light
! a source at the far antenna would have zero lag
! and sources on the ground would all have negative lag
! So we want to add idifflag(j) to the delay
            if (ilag < 1 .or. ilag>nlag) cycle
            chi2=chi2+abs(flag(ilag,k))
!            write(*,*) chi2
         end do
      end do
      chimap(ix,-iy)=chi2
   end do
end do
locchimin= (maxloc(chimap)-nx)*dx
locchimin(2)=-locchimin(2)
write(*,'(2i8,10x,1h@,f9.6,1h,,f9.6)')locchimin,locchimin(2)/111111.+19.092778,locchimin(1)/104965.+74.050278
chimap=chimap-minval(chimap)
!chimap=-sqrt(chimap)
call pmap(trim(trim(adjustl(fnpref))//'.chimap.'//trim(adjustl(fn2))//'.pgm'),chimap,2*nx+1,2*nx+1,1)
end

subroutine pmap(fn,rmap1,nx,ny,iscale)
  real rmap(nx,ny),rmap1(nx,ny)
  integer*1 imap(nx,ny)
  character*255 fn
  integer npix,mypos

  npix=min(ny/2-1,nx/2-1,300)
  
  
  rmap=rmap1
  write(*,*) 'rms=',sqrt(sum(rmap(nx/2-npix:nx/2+npix,ny/2-npix:ny/2+npix)**2)/npix**2/4)
  if (iscale .eq. 2) rmap=sign(sqrt(abs(rmap)),rmap)
  
  rmax=maxval(rmap)
  rmin=minval(rmap)
  write(*,*) trim(adjustl(fn)),rmax,rmin
  imap=127*(rmap-rmin)/(rmax-rmin)
  open(10,file=trim(adjustl(fn)))
  write(10,'(2hP5)')
  write(10,*)nx,ny
  write(10,*) 127
!  INQUIRE(UNIT=10, POS=mypos)
  close(10)
  open(10,file=trim(adjustl(fn)), form='binary',access='append')
!  write(10,pos=mypos) imap
  write(10) imap
  close(10)
end
