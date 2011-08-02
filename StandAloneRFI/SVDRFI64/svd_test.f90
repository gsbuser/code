integer, parameter:: nnode=16,nsample=4096,nt=14336,nchunk=65,nchan=4*16
! restrictions:  n1/n must be an integer, so ncorr/nchunk must divide
integer, parameter:: n=nsample/nnode,ncorr=nchan*(nchan+1)/2,n1=ncorr*n/nchunk,maxnvec=50
integer, parameter :: lwork=1000*nt
real*8, dimension(lwork)::work8
real, dimension(n1,nt) :: data
real, dimension(n*ncorr) :: data1,data1a
real*8, dimension(n1,nt) :: data8
!real, dimension(n*ncorr,maxnvec) :: v
real*8, dimension(nt,nt) :: evec8,smatsum8
real*8, dimension(nt) :: tvec8
real*8, dimension(nt) :: eval8
!real, dimension(n*ncorr,maxnvec) :: coef
integer*1, dimension(n)::ibuf1
real, dimension(n)::rbuf
complex, dimension(n/2)::cbuf
equivalence(rbuf,cbuf)
equivalence(smatsum8,evec8)
character*80 fn,argv,fnbase,fnin,snode
real*8 one,zero
character*1 fnnum1
character*2 fnnum2
character*3 fnnum3
logical lrestart


nvec=maxnvec/2
if (2*nvec>min(maxnvec,nt)) then
	write(*,*) 'error: nvec>maxnvec,nt',nvec,maxnvec,nt
	stop
endif
!cmax=1000
cmax=1.e38
fn='corr.dat'

if (irank .eq. 0) write(*,*) ' master: size=',numtasks
iarg=iargc()
if (iarg .eq. 1) then
        call getarg(1,fnbase)
endif
irank=0

!fnin='/mnt/d/tchang/j2219_aug9_5.node'//fnnum1//'.fs0'
write(fnnum1,'(Z1)') irank
!fnin='/cita/scratch/cottontail/odegova/Masked/dec.17.07.b0823.1.node'//fnnum1//'.masked.fs0'
fnin='/mnt/node_scratch/paciga/Masked/'//trim(fnbase)//'.node'//fnnum1//'.masked.fs0'
if (irank .eq. 0) write(*,*)'opening ',fnin
call omp_set_num_threads(4)
smatsum8=0
!!$omp parallel do default(none) private(it,data,data8,one,zero) reduction(+:smatsum8)
cutfrac=0
savefrac=0
cutfrac=tmp
savefrac=tmp
if (irank.eq.0) write(*,*) 'cut=',cutfrac,'  rest=',savefrac
!evec8=0
if (irank .eq. 0) then
!        write(*,*) 'after reduce smatsum8=',smatsum8
	! diagonalize
        call random_number(evec8)
	write(*,*) 'starting diagonalization'
	call dsyev('V','U',nt,evec8,nt,eval8,work8,lwork,info)
  	if (info .ne. 0) write(*,*) 'ssyev: info=',info
!	write(*,*) sum(evec8**2,1)  ! eigenvectors are normalized
	eval8=eval8(nt:1:-1)	! eigenvalues are ascending
	do i=1,2*nvec
	   evec8(:,i)=evec8(:,nt+1-i)
	enddo
        write(*,*) 'eval=',eval8(:2),eval8(nt-1:nt)
endif
stop
end

