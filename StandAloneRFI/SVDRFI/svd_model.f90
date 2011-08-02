use hexprint
integer, parameter:: nnode=16,nsample=4096,nt=768,nchunk=61,nchan=4*15
! restrictions:  n1/n must be an integer, so ncorr/nchunk must divide
integer, parameter:: n=nsample/nnode,ncorr=nchan*(nchan+1)/2,n1=ncorr*n/nchunk,maxnvec=50
integer, parameter :: lwork=100*nt
real*8, dimension(lwork)::work8
real, dimension(n*ncorr) :: data1,data1a
real, dimension(n*ncorr,maxnvec) :: v
real*8, dimension(nt) :: tvec8
real*8, dimension(nt) :: eval8
real*8, dimension(nt,maxnvec) :: evec8
real, dimension(n*ncorr,maxnvec) :: coef
integer*1, dimension(n)::ibuf1
real, dimension(n)::rbuf
complex, dimension(n/2)::cbuf
equivalence(rbuf,cbuf)
!character*200 fn,argv,fnbase,fnin,snode
character*200 fn,argv,fnin,snode
character*255 fntvec,fnrfi,fncorrout
include 'mpif.h'
real*8 one,zero
!character*1 fnnum1
!character*2 fnnum2
character*3 fnnum3
logical lrestart

lrestart=.true.

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
   write(*,*) 'Usage: svd_model.x svdtvec fnrfi fncorrout'
   stop
endif

if (iarg .eq. 3) then
     call getarg(1,fntvec)
     call getarg(2,fnrfi)
     call getarg(3,fncorrout)
endif


!call omp_set_num_threads(8)
call omp_set_num_threads(2)
	if (irank == 0) then
!        open(40,file='/cita/scratch/cottontail/pen/Template/'//trim(fnbase)//'.svdtvec.dat',form='binary',status='old')
!        open(40,file='/cita/d/raid-cita/paciga/Template/'//trim(fnbase)//'.svdtvec.dat',form='binary',status='old')
           open(40,file=trim(adjustl(fntvec)),form='binary',status='old')
        read(40) nvec
	write(*,*) 'nvec=',nvec
!	nvec=nvec/2
        read(40) evec8(:,:nvec)
        close(40)
	endif
! broadcast eigenvectors of interest
call MPI_Bcast(nvec,1,MPI_INTEGER,0,MPI_COMM_WORLD,ierr)
if (ierr .ne. 0) write(*,*) 'MPI_Bcast nvec: ierr=',ierr
call MPI_Bcast(evec8,nvec*nt,MPI_DOUBLE_PRECISION,0,MPI_COMM_WORLD,ierr)
if (ierr .ne. 0) write(*,*) 'MPI_Bcast: ierr=',ierr
! subtract SVD modes
!write(fnnum1,'(Z1)') irank
!open(40,file='/mnt/node_scratch/paciga/Template/'//trim(fnbase)//'.svdrfi.node'//fnnum1,form='binary')
open(40,file=trim(adjustl(hexprintf(fnrfi,irank))),form='binary')
read(40) nvec
read(40) coef(:,:nvec)
close(40)
if (irank .eq. 0) write(*,*) 'starting 3rd file read and final write'
!subtract the singular RFI vectors
iunit=50
!open(iunit,file='/mnt/node_scratch/paciga/Model/'//trim(fnbase)//'.corrout.dat.node'//fnnum1,form='binary',err=200)
open(iunit,file=trim(adjustl(hexprintf(fncorrout,irank))),form='binary',err=200)
goto 300
200 continue
write(*,*) 'error opening /cita/d/raid-lobster6/Aug10_4/pen/corrout.dat.node'//fnnum3
300 continue
do it=1,nt
   nlen=ncorr
   data1a=0
!$omp parallel do default(shared)
   do ivec=1,nvec
!	data1=data1-coef(:,ivec)*evec8(it,ivec)
	data1a=data1a+coef(:,ivec)*evec8(it,ivec)
   enddo
	call dump1bit(data1a)
!	call dump1bit(data1)
enddo
call MPI_Finalize(ierr)
if (ierr .ne. 0) write(*,*) 'MPI_Finalize: ierr=',ierr
contains
	subroutine dump1bit(data)
   	real, dimension(n*ncorr)::data

   	nlen=ncorr
	do istride=1,nlen
		rbuf=data((istride-1)*n+1:istride*n)
		fmax=max(1.e-30,maxval(abs(rbuf)))
		rbuf=sqrt(abs(rbuf/fmax))*sign(127.,rbuf)
		ibuf1=nint(rbuf)
		write(iunit) fmax,ibuf1
	enddo
	end subroutine
end


