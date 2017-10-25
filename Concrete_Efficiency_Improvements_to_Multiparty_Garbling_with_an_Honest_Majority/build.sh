g++ -O3 -w -o BMRPassive.out -pthread OTExtnsion/*.cpp util/*.cpp -I 'util/' -std=c++11 -pthread -L./ -msse4.1 -pthread -maes -msse2 -mpclmul -fpermissive -fpic

