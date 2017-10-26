DESCRIPTION

Code for the online computation of the BMR protocol from "Optimizing Secure Multiparty Computation for the Internet" paper by Aner Ben-Efraim, Yehuda Lindell, and Eran Omri, ACM CCS 2016.

COMPILATION

Compilation is done by the command "bash build.sh"

RUNNING

Running is done by executing

"./OnlineTest.out <Partynum> <CircuitFile> <InputsFile> <IPFile> <Key> 0"

Partynum is the party number. This number should also match the location of the party's IP in the IP file. For non-local execution, it is possible to use "-1" and the party number is taken as the location in the IP file.

CircuitFile is the file of the circuit. All parties must use the exact same file.

InputsFile contains the inputs for the input wires of the party.

IPFile is the IPs (IPv4) of the parties. The file contains the IPs written row after row, without spaces. The IP of party p should appear at row p+1 (party 0 written on the first row, etc.). All parties must use the exact same file.

Key should be a random secret key.