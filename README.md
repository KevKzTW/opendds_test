# opendds_test.
.
This program was modified from OpenSplice/examples/DCPS/cpp/throughput and supported by openDDS .
.
Please put the folder into openDDS root folder .
.
Example .
/OpenDDS-3.9/opendds_test ...

$ cd /OpenDDS-3.9/opendds_test/throughput
$ make
.
###.
Publisher command.
$ ./publisher -DCPSConfigFile rtps.ini  
or.
$ ./publisher -DCPSConfigFile rtps.ini [payloadSize] [burstInterval] [burstSize]
.
And press ctrl+c to stop , after publication match .
.
###.
Subscriber command.
$ ./subscriber -DCPSConfigFile rtps.ini
