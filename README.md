# opendds_test

This program was modified from VortexOpenSplice/examples/dcps/throughput/cpp/ and supported by openDDS .

Please put the folder into openDDS root folder  

Example  
/OpenDDS-3.9/opendds_test 

###  
MAKE  
$ cd /OpenDDS-3.9/opendds_test/throughput  
$ make

###  
Publisher command in RTPS mode  
$ ./publisher -DCPSConfigFile rtps.ini   
or
$ ./publisher -DCPSConfigFile rtps.ini [payloadSize] [burstInterval] [burstSize]  

And press ctrl+c to stop , after publication match . 
  
###  
Subscriber command in RTPS mode  
$ ./subscriber -DCPSConfigFile rtps.ini  

Remember to change local_addre value to your network address in the rtps.ini   
