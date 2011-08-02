subroutine fft1(c1,r,n,isign)
complex c1(n/2)
complex c(n/2+1)
real r(n+2)
complex wsave(2*n+4)

c(:n/2)=c1
if (isign .eq. -1) then
c(1)=real(c1(1))
c(n/2+1)=aimag(c1(1))
call csfft1d(c,n,0,wsave)
call csfft1d(c,n,-isign,wsave)
c1=c(:n/2)
else
call scfft1d(c,n,0,wsave)
call scfft1d(c,n,-isign,wsave)
c1=c(:n/2)
c1(1)=cmplx(real(c(1)),real(c(n/2+1)))
endif
return
end

subroutine fft1a(c,r,n,isign)
integer, parameter :: nmax=16384
complex c(n/2+1)
real r(n+2)
complex wsave0(2*nmax+4),wsave1(2*nmax+4)
save wsave0,wsave1

if (isign .eq. 0) then
if (n>nmax) pause 'fft error: n>nmax'
call csfft1d(c,n,0,wsave0)
call scfft1d(c,n,0,wsave1)
return
endif
if (isign .eq. -1) then
call csfft1d(c,n,-isign,wsave0)
else
call scfft1d(c,n,-isign,wsave1)
endif
return
end

