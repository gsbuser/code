program rfipos
integer, parameter :: nmaxcorr=10000,nx=1000,dx=20
integer, dimension(3,nmaxcorr) :: lags
real, dimension(-nx:nx,-nx:nx) :: chimap
real, dimension(2,32) :: xy
integer, dimension(2) :: locchimin

data xy /687.88 ,  -21.19, 326.43 ,  -42.67, 0 ,  0, -372.72  , 141.29, -565.94  , 130.54, 67.82 , -260.80 , -31.44 , -233.29 , 280.67 , -423.40 , 41.92 , -160.39 , -164.88 , -621.34 , -603.28 , -340.09 , 174.85 , -672.16 , -639.53 ,  -1182.34 , -473.71 ,  -664.85 , -1591.94 ,  625.39, -3099.41 , 1501.18, -5199.90 , 3066.16 , -7039.03 , 5359.52, -8103.13 , 8267.57 , -11245.60 , 9430.02 , 2814.55 , 1008.62 , 4576.00 , 2043.81, 7780.69  ,  3070.58, 10200.00 , 3535.84 , 12073.46 , 4804.91 , 633.92 ,-2967.61 , -367.30 ,-4525.73 , 333.03 , -6774.02 , 947.68 , -9496.90 , -369.04 , -14153.59, 0, 0, 0, 0/

write(*,*) 'nx=',nx,'dx=',dx

!do wcut=0.2,0.6,0.01
!do wcut=4000,5000,1000
locchimin=0
do wcut=500,100000,1000
   open(10,file='lagnoshift.dat',status='old')
   do i=1,nmaxcorr
      do
         read(10,*,end=100) lags(:,i),w
         !if (minval(lags(:2,i))<15) cycle
         if (maxval(sum(xy(:,lags(:2,i))**2,1))>2500**2) cycle
	 !imax=maxval(lags(:2,i))
         !if (imax>15) cycle
         !      if (.not. any(lags(:2,i) .eq. 30)) cycle
         if (w>wcut) exit
      end do
   end do
100 continue
   rewind(10)
   neqn=i-1
   write(*,*) neqn,wcut
   !if (neqn > 200) cycle
   if (neqn > 800) cycle
   if (neqn < 50 .and. any(locchimin .ne. 0)) exit
   !$omp parallel do default(shared) private(x,y,chi2,iy,i)
   do ix=-nx,nx
      do iy=-nx,nx
         x=ix*dx
         y=iy*dx
         chi2=0
         do i=1,neqn
            if (sqrt(sum((xy(:,lags(2,i))-xy(:,lags(1,i)))**2))>9*1700) cycle
!            if (lags(1,i).lt.15.and.lags(2,i).lt.15) cycle ! skip correlations within central square
            if (lags(1,i).gt.30.or.lags(2,i).gt.30) cycle ! skip antennas 31 and 32
            chi2=chi2+sqrt((sqrt(sum(((/x,y/)-xy(:,lags(1,i)))**2))-sqrt(sum(((/x,y/)-xy(:,lags(2,i)))**2))+9*lags(3,i))**2)
         end do
         chimap(ix,-iy)=chi2
      end do
   end do
   locchimin= (minloc(chimap)-nx)*dx
   locchimin(2)=-locchimin(2)
!   write(*,'(2i8,10x,1h@,f9.6,1h,,f9.6)')locchimin,locchimin(2)/111111.+19.092778,locchimin(1)/104965.+74.050278
enddo
write(*,'(2i8,10x,1h@,f9.6,1h,,f9.6)')locchimin,locchimin(2)/111111.+19.092778,locchimin(1)/104965.+74.050278
chimap=chimap-minval(chimap)
chimap=-sqrt(chimap)
call pmap('chimap.pgm',chimap,2*nx+1,2*nx+1,2)

end program rfipos

  subroutine pmap(fn,rmap1,nx,ny,iscale)
    real rmap(nx,ny),rmap1(nx,ny)
    integer*1 imap(nx,ny)
    character*80 fn
    integer npix,mypos
    
    npix=min(ny/2-1,nx/2-1,300)
    
    
    rmap=rmap1
    write(*,*) 'rms=',sqrt(sum(rmap(nx/2-npix:nx/2+npix,ny/2-npix:ny/2+npix)**2)/npix**2/4)
    if (iscale .eq. 2) rmap=sign(sqrt(abs(rmap)),rmap)
    
    rmax=maxval(rmap)
    rmin=minval(rmap)
    write(*,*) adjustl(trim(fn)),rmax,rmin
    imap=127*(rmap-rmin)/(rmax-rmin)
    open(10,file=fn)
    write(10,'(2hP5)')
    write(10,*)nx,ny
    write(10,*) 127
    !  INQUIRE(UNIT=10, POS=mypos)
    close(10)
    open(10,file=fn, form='binary',access='append')
    !  write(10,pos=mypos) imap
    write(10) imap
    close(10)
  end subroutine pmap
  

