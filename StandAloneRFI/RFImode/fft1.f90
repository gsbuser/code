subroutine fft1(c,r,n,isign)
complex c(n/2+1)
real r(n+2)
complex d(n)
real dr(n)

if (isign .eq. -1) then
d(:n/2+1)=c
d(n/2+2:)=conjg(c(n/2:2:-1))
call four1(d,n,isign)
r(:n)=real(d)/n
else
d=r
call four1(d,n,isign)
c=d(:n/2+1)
endif
return
end
