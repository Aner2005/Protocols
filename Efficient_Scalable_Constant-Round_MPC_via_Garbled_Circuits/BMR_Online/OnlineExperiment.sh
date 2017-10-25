for k in {1..5}; do
    ./OnlineTest.out -1 3Party/AESout 3Party/aesP0 parties 18902841701238799991456410093efe3 0
  sleep 3
    ./OnlineTest.out -1 P100/100FakeSha P100/shaP0 parties 18902841701238799991456410093efe3 0
  sleep 3
done
 
