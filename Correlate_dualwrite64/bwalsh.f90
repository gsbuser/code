integer*1 walsh(64,64)

walsh=1
open(10,file='walsh.unformatted',form='unformatted',status='old')
read(10) walsh(:,:32)
read(10) walsh(:,33:)
open(10,file='walsh.char',form='binary')
write(10) walsh
end
