for i in {1..5}; do
  ./LWEOnline.out -1 3Party/AESout 3Party/aesP0 parties 18902841701238799991456410093efe3 0 #Test on AES circuit - output should match encryption of all zero input with all zero key
  sleep 3
  ./LWEOnline.out -1 P500/500FakeSha P500/shaP0 parties 18902841701238799991456410093efe3 0 #Test on fake circuit with 100'000 AND gates an 0 XOR gates - for timing
  echo "Done Iteration $i"
  sleep 3
done
 
