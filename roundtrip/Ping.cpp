#include <ace/Log_Msg.h>
#include <ctime>
#include <iostream>
using namespace std;
#include "entities.cpp"
#include "RoundTripTypeSupportImpl.h"


int
ACE_TMAIN(int argc, ACE_TCHAR *argv[])
{

  try {
    unsigned long payloadSize = 8192;
    unsigned long burstInterval = 0;//us
    int burstSize = 1;
    
    ACE_Time_Value startTime = ACE_Time_Value(0,0);
    ACE_Time_Value preWriteTime = ACE_Time_Value(0,0);
    ACE_Time_Value postWriteTime = ACE_Time_Value(0,0);
    ACE_Time_Value preTakeTime = ACE_Time_Value(0,0);
    ACE_Time_Value postTakeTime = ACE_Time_Value(0,0);


    if(argc > 3) 
       payloadSize = atoi(argv[3]); //The size of the payload in bytes
    if(argc > 4)
       burstInterval = atoi(argv[4]); //The time interval between each burst in ms
    if(argc > 5)
       burstSize = atoi(argv[5]); //The number of samples to send each burst


    cout << "payloadSize: " << payloadSize << " | burstInterval: " << burstInterval
         << " | burstSize: " << burstSize << "\n" << endl;

    Entities e(argc,argv,"ping","pong");
    // Write samples
    RoundTripModule::DataType data;
    data.payload.length(payloadSize);
    for(unsigned long i=0;i<payloadSize;i++)
        data.payload[i] = 'a';

    RoundTripModule::DataTypeSeq samples;
    DDS::SampleInfoSeq info;

    std::cout << "# Warming up to stabilise performance..." << endl;

    startTime = ACE_OS::gettimeofday();
    DDS::ConditionSeq conditions;
    DDS::Duration_t waitTimeout = {1,0};
    
    while(1) {  
        ACE_Time_Value warmUpTime = ACE_OS::gettimeofday() - startTime; 
        if(warmUpTime.sec()+(double)warmUpTime.usec()/1000000 > 5) 
           break;
        e.message_writer->write(data, DDS::HANDLE_NIL);
        DDS::ReturnCode_t error = e.ws->wait(conditions, waitTimeout);
        conditions.replace(0, 0, NULL, false);
        if(error != DDS::RETCODE_TIMEOUT) {
           e.reader_i->take(samples, info, DDS::LENGTH_UNLIMITED,DDS::ANY_SAMPLE_STATE, DDS::ANY_VIEW_STATE, DDS::ANY_INSTANCE_STATE);
           e.reader_i->return_loan(samples, info);
        }
    }

    cout << "# Warm up complete.\n\n"
         << "# Round trip measurements (in us)\n"
         << "#             Round trip time [us]         Write-access time [us]       Read-access time [us]\n"
         << "# Seconds     Count   median      min      Count   median      min      Count   median      min" 
         << endl;


    startTime = ACE_OS::gettimeofday();
    unsigned long elapsed = 0;
    bool terminated = false;
    while(!terminated) {
        /** Write a sample that pong can send back */
        preWriteTime = ACE_OS::gettimeofday();
        e.message_writer->write(data, DDS::HANDLE_NIL);
        postWriteTime = ACE_OS::gettimeofday();
    

        /** Wait for response from pong */
        DDS::ReturnCode_t status = e.ws->wait(conditions, waitTimeout);
        conditions.replace(0, 0, NULL, false);
        if(status != DDS::RETCODE_TIMEOUT) {
            /** Take sample and check that it is valid */
            preTakeTime = ACE_OS::gettimeofday();
            e.reader_i->take(samples, info, DDS::LENGTH_UNLIMITED,DDS::ANY_SAMPLE_STATE, DDS::ANY_VIEW_STATE, DDS::ANY_INSTANCE_STATE);
            postTakeTime = ACE_OS::gettimeofday();

            e.reader_i->return_loan(samples, info);

            /** Update stats */
            e.writeAccess += exampleTimevalToMicroseconds(postWriteTime - preWriteTime);
            e.readAccess += exampleTimevalToMicroseconds(postTakeTime - preTakeTime);
            e.roundTrip += exampleTimevalToMicroseconds(postTakeTime - preWriteTime);
            e.writeAccessOverall += exampleTimevalToMicroseconds(postWriteTime - preWriteTime);
            e.readAccessOverall += exampleTimevalToMicroseconds(postTakeTime - preTakeTime);
            e.roundTripOverall += exampleTimevalToMicroseconds(postTakeTime - preWriteTime);

            /** Print stats each second */
            if(exampleTimevalToMicroseconds(postTakeTime - startTime) > US_IN_ONE_SEC ) {
                printf ("%9lu %9lu %8.0f %8lu %10lu %8.0f %8lu %10lu %8.0f %8lu\n",
                         elapsed + 1,
                         e.roundTrip.count,
                         exampleGetMedianFromTimeStats(e.roundTrip),
                         e.roundTrip.min,
                         e.writeAccess.count,
                         exampleGetMedianFromTimeStats(e.writeAccess),
                         e.writeAccess.min,
                         e.readAccess.count,
                         exampleGetMedianFromTimeStats(e.readAccess),
                         e.readAccess.min);

                /* Reset stats for next run */
                exampleResetTimeStats(e.roundTrip);
                exampleResetTimeStats(e.writeAccess);
                exampleResetTimeStats(e.readAccess);

                /** Set values for next run */
                startTime = ACE_OS::gettimeofday();
                elapsed++;
            }
        }
    }   
  } catch (const CORBA::Exception& e) {
    e._tao_print_exception("Exception caught in main():");
    return -1;
  } 
  return 0;
}

