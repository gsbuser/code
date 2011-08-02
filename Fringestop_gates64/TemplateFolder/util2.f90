!        accumulate to float fcross[NCORR][FFTLEN/2][2]
subroutine int2float(NFOLD, NCORR, FFTLEN, NPOD, NNODE, fcross, icross)
integer :: NFOLD, NCORR, FFTLEN, NPOD, NNODE,  irank
real fcross(2,FFTLEN/NNODE/2,NCORR,NFOLD)
integer icross(8,2,NCORR,FFTLEN/NNODE/16,NFOLD)
integer  i,j,iot,io,jo,jl,ic,jc,is;
!write(*,*) icross(1,:,:,1,1)
  do is=1,NFOLD
    do iot=1,NCORR
      do j=1,FFTLEN/NNODE/2
               do i=1,2
        fcross(i,j,iot,is)=icross(mod(j-1,8)+1,i,iot,(j-1)/8+1,is)
        icross(mod(j-1,8)+1,i,iot,(j-1)/8+1,is)=0
               enddo
      enddo
  enddo
enddo
!write(*,*) fcross(:,:10,1,1)
!stop
end
!        accumulate to float fcross[NCORR][FFTLEN/2][2]
subroutine shuffle(NFOLD, NCORR, FFTLEN, NPOD, NNODE, fcross, icross)
integer :: NFOLD, NCORR, FFTLEN, NPOD, NNODE,  irank
real fcross(2,FFTLEN/NNODE/2,NCORR,NFOLD)
real icross(8,2,NCORR,FFTLEN/NNODE/16,NFOLD)
integer  i,j,iot,io,jo,jl,ic,jc,is;
!write(*,*) icross(1,:,:,1,1)
  do is=1,NFOLD
!$omp parallel do default(none) private(j,i) shared(fcross,icross,NCORR,FFTLEN,NNODE)
    do iot=1,NCORR
      do j=1,FFTLEN/NNODE/2
               do i=1,2
        fcross(i,j,iot,is)=icross(mod(j-1,8)+1,i,iot,(j-1)/8+1,is)
        icross(mod(j-1,8)+1,i,iot,(j-1)/8+1,is)=0
               enddo
      enddo
  enddo
enddo
!write(*,*) fcross(:,:10,1,1)
!stop
end
