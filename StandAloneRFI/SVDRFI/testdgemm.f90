integer*4, parameter:: nnode=16,nsample=4096,nt=16000,nchunk=61,nchan=4*15
! restrictions:  n1/n must be an integer, so ncorr/nchunk must divide
integer*4, parameter:: n=nsample/nnode,ncorr=nchan*(nchan+1)/2,n1=ncorr*n/nchunk,maxnvec=100
!integer, parameter :: n1=10000, nt=n1
real*8, dimension(n1,nt) :: data8
real*8, dimension(nt,nt) :: smat8

        one=1
        zero=0
	call omp_set_num_threads(8)
	data8=0
	smat8=0
	do i=1,10
        write(*,*) i
        call dgemm('T','N',nt,nt,n1,one,data8,n1,data8,n1,one,smat8,nt)
	enddo
	end
