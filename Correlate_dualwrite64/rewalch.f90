integer*1 walsh(64,64)

walsh=1
open(10,file='walsh60.char',form='binary',status='old')
read(10) walsh(:,:60)
walsh(:,33:62)=walsh(:,31:60)
open(10,file='walsh.char',form='binary')
write(10) walsh
end
