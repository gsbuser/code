character*200 fnin,fnout,argv

icount=iargc()
if (icount .ne. 5) then
   write(*,*) 'iargc=',icount
   write(*,*) 'usage: debias.x nepoch 2*n nchan fnin fnout'
   stop
end if
call getarg(1,argv)
read(argv,*) nepoch
call getarg(2,argv)
read(argv,*) n
call getarg(3,argv)
read(argv,*) nchan
ncorr=nchan*(nchan+1)/2
call getarg(4,fnin)
call getarg(5,fnout)
!call omp_set_num_threads(8)
write(*,*) 'sigmacut_debias fnin  =',fnin
write(*,*) 'sigmacut_debias fnout =',fnout
call debias(nepoch,n,ncorr,fnin,fnout)
end


subroutine debias(nepoch,n,ncorr,fnin,fnout)
character*200 fnin,fnout
real, dimension(n,nepoch) :: corr
real, dimension(n) :: corr0
real, dimension(n,ncorr) :: corr1
integer*1 iline1(n)
integer*1 ilineall(n,ncorr,nepoch)
real fmax,fmaxall(ncorr,nepoch)

nchan=nint((sqrt(8.*ncorr+1)-1)/2.)
open(10,file=fnin,form='binary')
ncut=0
do i=1,nepoch
   do j=1,ncorr
      read(10) fmaxall(j,i),ilineall(:,j,i)
   enddo
   corr1=sign(spread(fmaxall(:,i),1,n)*(ilineall(:,:,i)/127.)**2,ilineall(:,:,i)*1.)
   call sigmacut(corr1,n/2,nchan)
   where(corr1 .eq. 0) ilineall(:,:,i)=0
end do
ncut=count(ilineall .ne. 0)
ncorr1=nchan*(nchan+1)/2-nchan-nchan/2
write(*,*) 'cut fraction=',1-ncut*1./(n*nepoch*ncorr1)
do j=1,ncorr
   do i=1,nepoch
      corr(:,i)=sign(fmaxall(j,i)*(ilineall(:,j,i)/127.)**2,ilineall(:,j,i)*1.)
   end do
   call rmedian(corr1(1,j),corr,nepoch,n)
end do
open(10,file=fnout,form='binary')
do i=1,nepoch
   do j=1,ncorr
      corr0=sign(fmaxall(j,i)*(ilineall(:,j,i)/127.)**2,ilineall(:,j,i)*1.)
      where(corr0 .ne. 0) corr0=corr0-corr1(:,j)
      fmax=maxval(corr0)
      if (fmax .eq. 0) fmax=1
      iline1=nint(127*sign(sqrt(abs(corr0/fmax)),corr0))
      write(10) fmax,iline1
   end do
end do
end

subroutine rmedian(a,b,nt,n)
real, dimension(n,nt) :: b
real, dimension(n) :: a
real, dimension(nt) :: t
integer, dimension(nt) :: iorder


!  a=sum(b,2)
!  return
!$omp parallel do default(shared) private(j,i,t,iorder)
  do in=1,n
     j=0
     do i=1,nt
        if (b(in,i) .ne. 0) then
           j=j+1
           t(j)=b(in,i)
        endif
     end do
     if (j .eq. 0) then
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

subroutine sigmacut(corr,n,nchan)
complex, dimension(n,nchan*(nchan+1)/2) :: corr
! locals
integer, dimension(nchan) :: ipolmap,iantenna
integer iktable(nchan,nchan)
real, dimension(n) :: r1,r2
integer, dimension(2,nchan/2) :: maptable

! 8 MSample per buffer @ 4096 samples per buffer = 2000 buffers
! the diagonal is the sum of two reals, so 2 \sigma^2
! cross divided by diagonal is 

threshold=2.5/sqrt(2047.)  ! 2.5 sigma cut
ipolmap=0
ipolmap(29:58)=1
iantenna(:28)=(/(i,i=1,28)/)
iantenna(29:58)=(/(i,i=1,30)/)
iantenna(59:60)=(/29,30/)
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
! flag on cross polar correlation
!$omp parallel do default(shared) private(j,r1,r2)
do i=1,nchan/2
   do j=i,nchan/2
      if (iantenna(i) .eq. iantenna(j)) cycle
      r1=abs(corr(:,iktable(maptable(1,i),maptable(2,j)))/sqrt(corr(:,iktable(maptable(1,i),maptable(1,i)))*corr(:,iktable(maptable(2,j),maptable(2,j)))))
      r2=abs(corr(:,iktable(maptable(2,i),maptable(1,j)))/sqrt(corr(:,iktable(maptable(2,i),maptable(2,i)))*corr(:,iktable(maptable(1,j),maptable(1,j)))))
      where(spread(r1 > threshold .or. r2>threshold,2,2)) 
         corr(:,iktable(maptable(:,i),maptable(1,j)))=0
         corr(:,iktable(maptable(:,i),maptable(2,j)))=0
      end where
   enddo
enddo
! delete intra antenna correlations
do i=1,nchan/2
   corr(:,iktable(maptable(:,i),maptable(1,i)))=0
   corr(:,iktable(maptable(:,i),maptable(2,i)))=0
end do
end

