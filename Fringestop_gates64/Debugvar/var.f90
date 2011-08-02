integer, parameter :: n=128, nchan=60, ncorr=nchan*(nchan+1)/2
integer*1, dimension(2,n) :: ibuf1
real, dimension(2,n,ncorr) :: rbuf,rbuftmp
real, dimension(n,ncorr) :: var
real, dimension(n/16,ncorr) :: varbin

open(10,file='/mnt/d/pen/b0823+26_ac_dec14_8.node15.fs0',status='old',form='binary')
open(20,file='var.test',form='binary')
do i=1,3600*4/64
   var=0
   do j=1,64
        do k=1,ncorr
        read(10) fmax,ibuf1
        rbuf(:,:,k)=sign(fmax,ibuf1*1.)*(ibuf1/127.)**2
        enddo
	if (mod(j,2) .eq. 1) then
		rbuftmp=rbuf
	else
		var=var+sum((rbuf-rbuftmp)**2,1)
	endif
   enddo
   varbin=0
   do j=1,16
      varbin=varbin+var(j::16,:)
   enddo
   write(20) varbin
   write(*,*) i
enddo
end
