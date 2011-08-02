real xy0(2)
character*79 fn,argv
icount=iargc()
write(*,*) 'iargc=',icount
if (icount .ne. 2) then
        write(*,*) 'usage: antaz x y (in meters)'
        stop
endif

call getarg(1,argv)
read(argv,*) xy0(1)
call getarg(2,argv)
read(argv,*) xy0(2)

!xy0=(/  2380  , -460 /)

call az(xy0)
end

! fringestop corr to position x,y
! xy in meters from C02
! freq0,df in MHz
!
subroutine az(xy)
  implicit none
  integer, parameter :: nchan=60
  integer nfreq
  real xy(2),dx(2)
! locals
  logical firsttime
  data firsttime /.true./
  real antxy(2,nchan),delay,pi,antxy0(2,nchan/2),theta
  integer i,j,k,if
  character*5 chant(30)
  save firsttime,pi,antxy
  data antxy0 /687.88,  -20.04,  326.43,  -40.35,    0.00,    0.00,&
       -372.72,  133.59, -565.94,  123.43,   67.82, -246.59,  -31.44, -220.58,&
       280.67, -400.33,   41.92, -151.65, -164.88, -587.49, -603.28, -321.56,&
       174.85, -635.54, -639.53,-1117.92, -473.71, -628.63,-1591.94,  591.32,&
       -3099.41,1419.39,-5199.90, 2899.11,-7039.03, 5067.53,-8103.13, 7817.14,&
       -11245.60, 8916.26, 2814.55,  953.67, 4576.00, 1932.46,7780.69,2903.29,&
       10200.00,3343.20,12073.46, 4543.13,  633.92,-2805.93, -367.30,-4279.16,&
       333.03,-6404.96,  947.68,-8979.50, -369.04,-13382.50/

  if (firsttime) then
     firsttime=.false.
     pi=4*atan(1.)
     antxy(:,:28)=antxy0(:,:28)
     antxy(:,29:58)=antxy0
     antxy(:,59:)=antxy0(:,29:)
!     write(*,'(2f20.10)') antxy
     chant=(/'C00','C01','C02','C03','C04','C05','C06','C08','C09','C10', &
        'C11','C12','C13','C14','W01','W02','W03','W04','W05','W06','E02',&
        'E03','E04','E05','E06','S01','S02','S03','S04','S06'/)
  end if
  k=0
!  write(*,*) 'source south of E02 in antenna coordinates:'
  do i=1,30
     dx=xy-antxy0(:,i)
     theta=modulo(270+atan2(dx(2),dx(1))*180/pi,360.)
     write(*,*) i,chant(i),180-theta
  end do
end subroutine


