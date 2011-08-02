#ssh -n test1 /mnt/code/gsbuser/EoR/Analysis/analyze_short.sh b0823+26_feb19_1 19 1 a a raid0 >& /mnt/a/gsbuser/EoR/LOGS/LOGanalysis19_1 &
#ssh -n test1 /mnt/code/gsbuser/EoR/Analysis/analyze_short.sh b0823+26_feb19_2 19 2 b b raid0 >& /mnt/a/gsbuser/EoR/LOGS/LOGanalysis19_2 &
ssh -n test1 /mnt/code/gsbuser/EoR/Analysis/analyze_short.sh b0823+26_feb19_3 19 3 c c WD >& /mnt/a/gsbuser/EoR/LOGS/LOGanalysis19_3 &
ssh -n test1 /mnt/code/gsbuser/EoR/Analysis/analyze_short.sh b0823+26_feb19_4 19 4 d d WD >& /mnt/a/gsbuser/EoR/LOGS/LOGanalysis19_4 &
ssh -n test1 /mnt/code/gsbuser/EoR/Analysis/analyze_short.sh b0823+26_feb19_8 19 8 d d WD >& /mnt/a/gsbuser/EoR/LOGS/LOGanalysis19_8 &
