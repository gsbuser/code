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
amv(-62.4079d,18d)
sleep 2

* C01
ante 1 6
cp 0;defs 0;suba 0
amv(-44.3834d,18d)
sleep 2

* C02
ante 1 5
cp 0;defs 0;suba 0
amv(-13.4404d,18d)
sleep 2

* C03
ante 1 1
cp 0;defs 0;suba 0
amv(47.7547d,18d)
sleep 2

* C04
ante 1 3
cp 0;defs 0;suba 0
amv(60.8014d,18d)
sleep 2

* C05
ante 1 19
cp 0;defs 0;suba 0
amv(-14.1974d,18d)
sleep 2

* C06
ante 1 20
cp 0;defs 0;suba 0
amv(-5.7490d,18d)
sleep 2

* C08
ante 1 24
cp 0;defs 0;suba 0
amv(-25.3460d,18d)
sleep 2

* C09
ante 1 4
cp 0;defs 0;suba 0
amv(-14.0095d,18d)
sleep 2

* C10
ante 1 12
cp 0;defs 0;suba 0
amv(4.2322d,18d)
sleep 2

* C11
ante 1 9
cp 0;defs 0;suba 0
amv(35.7416d,18d)
sleep 2

* C12
ante 1 2
cp 0;defs 0;suba 0
amv(-14.6505d,18d)
sleep 2

* C13
ante 1 11
cp 0;defs 0;suba 0
amv(19.9582d,18d)
sleep 2

* C14
ante 1 10
cp 0;defs 0;suba 0
amv(20.5467d,18d)
sleep 2

* W01
ante 1 8
cp 0;defs 0;suba 0
amv(97.72684d,18d)
sleep 2

* W02
ante 1 13
cp 0;defs 0;suba 0
amv(108.93518d,18d)
sleep 2

* W03
ante 1 14
cp 0;defs 0;suba 0
amv(116.18335d,18d)
sleep 2

* W04
ante 1 15
cp 0;defs 0;suba 0
amv(123.96751d,18d)
sleep 2

* W05
ante 1 16
cp 0;defs 0;suba 0
amv(132.84425d,18d)
sleep 2

* W06
ante 1 25
cp 0;defs 0;suba 0
amv(127.4043d,18d)
sleep 2

* E02
ante 1 17
cp 0;defs 0;suba 0
amv(-101.01249d,18d)
sleep 2

* E03
ante 1 18
cp 0;defs 0;suba 0
amv(-108.30592d,18d)
sleep 2

* E04
ante 1 21
cp 0;defs 0;suba 0
amv(-107.71764d,18d)
sleep 2

* E05
ante 1 22
cp 0;defs 0;suba 0
amv(-106.02014d,18d)
sleep 2

* E06
ante 1 23
cp 0;defs 0;suba 0
amv(-108.85709d,18d)
sleep 2

* S01
ante 1 26
cp 0;defs 0;suba 0
amv(-12.8170d,18d)
sleep 2

* S02
ante 1 27
cp 0;defs 0;suba 0
amv(3.3673d,18d)
sleep 2

* S03
ante 1 28
cp 0;defs 0;suba 0
amv(-3.5862d,18d)
sleep 2

* S04
ante 1 29
cp 0;defs 0;suba 0
amv(-6.3375d,18d)
sleep 2

* S06
ante 1 30
cp 0;defs 0;suba 0
amv(1.1497d,18d)
sleep 2


* Configuring all antennas to suba 4.
allant
cp 0;defs 4;suba 4
sndsacant

enacmdmon

/bell
end

