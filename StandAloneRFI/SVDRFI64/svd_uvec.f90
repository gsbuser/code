use hexprint
integer, parameter:: nnode=16,nsample=4096,nt=768,nchunk=65,nchan=4*16
! restrictions:  n1/n must be an integer, so ncorr/nchunk must divide
integer, parameter:: n=nsample/nnode,ncorr=nchan*(nchan+1)/2,n1=ncorr*n/nchunk,maxnvec=50
integer, parameter :: lwork=100*nt
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
!character*80 fn,argv,fnbase,fnin,snode
character*80 fn,argv,fnin,snode
character*255 fnmasked, fntvec, fneval
include 'mpif.h'
real*8 one,zero
!character*1 fnnum1
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
call MPI_Init(ierr)
if (ierr .ne. 0) write(*,*) 'MPI_Init: ierr=',ierr
call MPI_COMM_RANK(MPI_COMM_WORLD, irank, ierr)
if (ierr .ne. 0) write(*,*) 'MPI_COMM_RANK: ierr=',ierr
call MPI_COMM_SIZE(MPI_COMM_WORLD, numtasks, ierr)
if (ierr .ne. 0) write(*,*) 'MPI_COMM_SIZE: ierr=',ierr
if (numtasks .ne. nnode) then
        write(*,*) 'error: nnode != numtasks',nnode,numtasks
        stop
endif

if (irank .eq. 0) write(*,*) ' master: size=',numtasks
   
iarg=iargc()
if (iarg .ne. 3) then
   write(*,*) 'Usage: svd_uvec.x masked svdtvec eval'
   stop
endif
if (iarg .eq. 3) then
        call getarg(1,fnmasked)
        call getarg(2,fntvec)
        call getarg(3,fneval)
endif

!fnin='/mnt/d/tchang/j2219_aug9_5.node'//fnnum1//'.fs0'
!write(fnnum1,'(Z1)') irank
!fnin='/cita/scratch/cottontail/odegova/Masked/dec.17.07.b0823.1.node'//fnnum1//'.masked.fs0'
!fnin='/mnt/node_scratch/paciga/Masked/'//trim(fnbase)//'.node'//fnnum1//'.masked.fs0'
fnin=trim(adjustl(hexprintf(fnmasked,irank)))
if (irank .eq. 0) write(*,*)'opening ',fnin
write(*,*)'opening ',fnin
!call omp_set_num_threads(8)
call omp_set_num_threads(2)
open(10,file=fnin,form='binary',status='old',access='direct',recl=n+4)
smatsum8=0
!!$omp parallel do default(none) private(it,data,data8,one,zero) reduction(+:smatsum8)
cutfrac=0
savefrac=0
do ichunk=1,nchunk
	if (mod(ichunk,2) .eq.1) write(*,*) 'irank=',irank,'  ichunk=',ichunk
	do it=1,nt
!		if (mod(it,30) .eq.1) write(*,*) it
		nlen=n1/n
		do istride=1,nlen
		   read(10,rec=(ichunk-1+(it-1)*nchunk)*nlen+istride) fmax,ibuf1
!			write(*,*) fmax
		   rbuf=sign(fmax,ibuf1*1.)*(ibuf1/127.)**2
		   cutfrac=cutfrac+count(abs(cbuf)>cmax)
		   savefrac=savefrac+count(abs(cbuf).le.cmax)
		   where(abs(cbuf)>cmax) cbuf=0
		   data8((istride-1)*n+1:istride*n,it)=rbuf
		enddo
	enddo
!	data8=data
	!smatsum8=smatsum8+matmul(transpose(data8),data8)
	one=1
	zero=0
	call dgemm('T','N',nt,nt,n1,one,data8,n1,data8,n1,one,smatsum8,nt)
!SUBROUTINE DGEMM ( TRANSA, TRANSB, M, N, K, ALPHA,  A,  LDA,  B,  LDB, BETA, C, LDC )
!c(1:m,1:n)=beta*c(1:m,1:n)+alpha*matmul(transpose(a(1:k,1:m)),b(1:k,1:n))     
enddo
close(10)
call MPI_Reduce(cutfrac,tmp,1,MPI_REAL,MPI_SUM,0,MPI_COMM_WORLD,ierr)
cutfrac=tmp
call MPI_Reduce(savefrac,tmp,1,MPI_REAL,MPI_SUM,0,MPI_COMM_WORLD,ierr)
savefrac=tmp
if (irank.eq.0) write(*,*) 'cut=',cutfrac,'  rest=',savefrac
!evec8=0
!call MPI_Reduce(smatsum8,evec8,nt*nt,MPI_DOUBLE_PRECISION,MPI_SUM,0,MPI_COMM_WORLD,ierr)
do i=1,nt
tvec8=0
call MPI_Reduce(smatsum8(1,i),tvec8,nt,MPI_DOUBLE_PRECISION,MPI_SUM,0,MPI_COMM_WORLD,ierr)
if (ierr .ne. 0) write(*,*) 'MPI_reduce: ierr=',ierr
!if (irank .eq. 0) write(*,*) smatsum8(:,i),tvec8
smatsum8(:,i)=tvec8
enddo
if (irank .eq. 0) then
!        write(*,*) 'after reduce smatsum8=',smatsum8
	! diagonalize
	write(*,*) 'starting diagonalization'
	call dsyev('V','U',nt,evec8,nt,eval8,work8,lwork,info)
  	if (info .ne. 0) write(*,*) 'ssyev: info=',info
!	write(*,*) sum(evec8**2,1)  ! eigenvectors are normalized
	eval8=eval8(nt:1:-1)	! eigenvalues are ascending
	do i=1,2*nvec
	   evec8(:,i)=evec8(:,nt+1-i)
	enddo
!	open(40,file='/cita/scratch/cottontail/pen/Template/'//trim(fnbase)//'.svdtvec.dat',form='binary')
!	open(40,file='/cita/d/raid-cita/paciga/Template/'//trim(fnbase)//'.svdtvec.dat',form='binary')
        open(40,file=trim(adjustl(fntvec)),form='binary') ! no node number for this one
	write(40) 2*nvec
	write(40) evec8(:,:2*nvec)
	close(40)
        write(*,*) 'eval=',eval8(:2),eval8(nt-1:nt)
!	open(70,file='/cita/scratch/cottontail/pen/Template/'//trim(fnbase)//'.eval.dat')
!	open(70,file='/cita/d/raid-cita/paciga/Template/'//trim(fnbase)//'.eval.dat')
        open(70,file=trim(adjustl(fneval)))
	write(70,*) eval8
	close(70)
endif
call MPI_Bcast(eval8,nvec,MPI_DOUBLE_PRECISION,0,MPI_COMM_WORLD,ierr)
if (ierr .ne. 0) write(*,*) 'MPI_Bcast: ierr=',ierr
call MPI_Finalize(ierr)
stop
end

