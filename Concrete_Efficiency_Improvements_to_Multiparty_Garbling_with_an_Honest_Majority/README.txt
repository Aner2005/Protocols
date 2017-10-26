DESCRIPTION

Implementation of semi-honest secure constant round protocols of the paper "Concrete Efficiency Improvements for Multiparty Garbling with an Honest Majority" by Aner Ben-Efraim and Eran Omri.

COMPILATION

Compilation is done by the command "bash build"

RUNNING

Running is done by executing

"BMRPassive.out <Partynum> <CircuitFile> <InputsFile> <IPFile> <Key> <Version Number>"

Partynum is the party number. This number should also match the location of the party's IP in the IP file. For non-local execution, it is possible to use "-1" and the party number is taken as the location in the IP file.

CircuitFile is the file of the circuit. All parties must use the exact same file.

InputsFile contains the inputs for the input wires of the party.

IPFile is the IPs (IPv4) of the parties. The file contains the IPs written row after row, without spaces. The IP of party p should appear at row p+1 (party 0 written on the first row, etc.). All parties must use the exact same file.

Key should be a random secret key.

Version Number is the version we use (i.e., BGW2 or BGW3 and which optimizations to use). All parties must use the same version number.

List of the different versions:

10 - BGW3 protocol without optimizations (original protocol but one evaluator)
11 - BGW3 with main optimization of share conversion (quadratic computational complexity)
12 - BGW3 with main optimization and with superseed of length only t+1 (~n/2)
13 - BGW3 with main optimization, superseed of length t+1, and zero-shares are locally generated
14 - BGW3 with main optimization, superseed of length t+1, locally generated zero-shares, and distribution of workload in last round

20 - BGW2 protocol without optimizations (original protocol but one evaluator)
21 - BGW2 with main optimization of share conversion (quadratic computational complexity)
22 - BGW2 with main optimization and with superseed of length only t+1 (~n/3)
23 - BGW2 with main optimization, superseed of length t+1, and zero-shares are locally generated
24 - BGW2 with main optimization, superseed of length t+1, locally generated zero-shares, and distribution of workload in last round

