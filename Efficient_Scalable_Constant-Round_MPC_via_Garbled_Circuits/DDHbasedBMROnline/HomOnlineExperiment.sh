for i in {50..300..25}; do
  ./HomOnlineTest.out -1 3Party/AESout 3Party/aesP0 parties 18902841701238799991456410093efe3 0
  sleep 3
  ./HomOnlineTest.out -1 P500/500FakeSha P500/shaP0 parties 18902841701238799991456410093efe3 0
  sleep 3
done
 
