! Module to convert %X to a hex digit, similar to C's sprintf
! Put 'use hexprint' at the begining of your program

module hexprint

contains
  
  function hexprintf(input, irank)
    implicit none
    character*127 input,fn,hexprintf
    integer irank
    character*1 node
    write(node,'(Z1)') irank
    fn=input
    do while (index(fn,'%X') .gt. 0)
       fn=trim(adjustl(fn(:index(fn,'%X')-1)))//node//trim(adjustl(fn(index(fn,'%X')+2:)))
    enddo
    hexprintf=fn
    return
  end function hexprintf
  
end module hexprint
