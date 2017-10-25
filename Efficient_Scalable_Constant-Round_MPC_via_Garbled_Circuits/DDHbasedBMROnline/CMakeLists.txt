cmake_minimum_required(VERSION 2.8)
project(NTLWrapper)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -msse4.2 -maes -O3")

set(SOURCE_FILES main.cpp NTLWrapper.cpp NTLWrapper.h ZZPrecomputeExp.cpp ZZPrecomputeExp.h ZZPrecomputeExp_Sec.cpp ZZPrecomputeExp_Sec.h MPZPrecomputeExp.cpp MPZPrecomputeExp.h MPZPrecomputeExp_Sec.cpp MPZPrecomputeExp_Sec.h secCompMultiParty.cpp secCompMultiParty.h homBMR.cpp homBMR.h)
add_executable(HomOnlineTest.out ${SOURCE_FILES})

target_link_libraries(HomOnlineTest.out ntl gmpxx gmp crypto)
