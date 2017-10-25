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

Version Number is the version we use (i.e., BGW2 or BGW3 and which optimizations to use) . List of the different versions:
[Put list here]

All parties must use the same version number.

size: 169.5 kb