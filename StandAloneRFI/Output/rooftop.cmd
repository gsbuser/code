*********************************************************************
* 30 antennas Azimuth position for Mr. Ue-Li-Pen's RFI experiment.
* Set all antennas to the following set of az and el coordinates.
* These are the coordinates for file -92.7
*********************************************************************

cmode 1
lnkndasq
subar 4

suba 4
shsub 4
stabct


discmdmon

* C00
ante 1 7
cp 0;defs 0;suba 0
amv(117.5921d,18d)
sleep 2

* C01
ante 1 6
cp 0;defs 0;suba 0
amv(135.6166d,18d)
sleep 2

* C02
ante 1 5
cp 0;defs 0;suba 0
amv(166.5596d,18d)
sleep 2

* C03
ante 1 1
cp 0;defs 0;suba 0
amv(-132.2453d,18d)
sleep 2

* C04
ante 1 3
cp 0;defs 0;suba 0
amv(-119.1986d,18d)
sleep 2

* C05
ante 1 19
cp 0;defs 0;suba 0
amv(165.8026d,18d)
sleep 2

* C06
ante 1 20
cp 0;defs 0;suba 0
amv(174.2510d,18d)
sleep 2

* C08
ante 1 24
cp 0;defs 0;suba 0
amv(154.6540d,18d)
sleep 2

* C09
ante 1 4
cp 0;defs 0;suba 0
amv(165.9905d,18d)
sleep 2

* C10
ante 1 12
cp 0;defs 0;suba 0
amv(-175.7678d,18d)
sleep 2

* C11
ante 1 9
cp 0;defs 0;suba 0
amv(-144.2584d,18d)
sleep 2

* C12
ante 1 2
cp 0;defs 0;suba 0
amv(165.3495d,18d)
sleep 2

* C13
ante 1 11
cp 0;defs 0;suba 0
amv(-160.0418d,18d)
sleep 2

* C14
ante 1 10
cp 0;defs 0;suba 0
amv(-159.4533d,18d)
sleep 2

* W01
ante 1 8
cp 0;defs 0;suba 0
amv(-82.27316d,18d)
sleep 2

* W02
ante 1 13
cp 0;defs 0;suba 0
amv(-71.06482d,18d)
sleep 2

* W03
ante 1 14
cp 0;defs 0;suba 0
amv(-63.81665d,18d)
sleep 2

* W04
ante 1 15
cp 0;defs 0;suba 0
amv(-56.03249d,18d)
sleep 2

* W05
ante 1 16
cp 0;defs 0;suba 0
amv(-47.15575d,18d)
sleep 2

* W06
ante 1 25
cp 0;defs 0;suba 0
amv(-52.59570d,18d)
sleep 2

* E02
ante 1 17
cp 0;defs 0;suba 0
amv(78.98751d,18d)
sleep 2

* E03
ante 1 18
cp 0;defs 0;suba 0
amv(71.69408d,18d)
sleep 2

* E04
ante 1 21
cp 0;defs 0;suba 0
amv(72.28236d,18d)
sleep 2

* E05
ante 1 22
cp 0;defs 0;suba 0
amv(73.97986d,18d)
sleep 2

* E06
ante 1 23
cp 0;defs 0;suba 0
amv(71.14291d,18d)
sleep 2

* S01
ante 1 26
cp 0;defs 0;suba 0
amv(167.1830d,18d)
sleep 2

* S02
ante 1 27
cp 0;defs 0;suba 0
amv(-176.6327d,18d)
sleep 2

* S03
ante 1 28
cp 0;defs 0;suba 0
amv(176.4138d,18d)
sleep 2

* S04
ante 1 29
cp 0;defs 0;suba 0
amv(173.6625d,18d)
sleep 2

* S06
ante 1 30
cp 0;defs 0;suba 0
amv(-178.8503d,18d)
sleep 2


* Configuring all antennas to suba 4.
allant
cp 0;defs 4;suba 4
sndsacant

enacmdmon

/bell
end

