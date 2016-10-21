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
    Entities e(argc,argv,"pong","ping");
    cout << "Waiting for samples from ping to send back..." << std::endl;

    DDS::Duration_t waitTimeout = {60,0};
    DDS::ConditionSeq conditions;
    RoundTripModule::DataTypeSeq samples;
    DDS::SampleInfoSeq info;
    RoundTripModule::DataType data;
    bool terminate = false;
    while(!terminate) {
        /** Wait for a sample from ping */
        e.ws->wait(conditions, waitTimeout);
        conditions.replace(0, 0, NULL, false);
        /** Take samples */
        e.reader_i->take(samples, info, DDS::LENGTH_UNLIMITED,DDS::ANY_SAMPLE_STATE, DDS::ANY_VIEW_STATE, DDS::ANY_INSTANCE_STATE);
        for (unsigned long i = 0; i < samples.length(); i++) {
             /** If writer has been disposed terminate pong */
             if(info[i].instance_state == DDS::NOT_ALIVE_DISPOSED_INSTANCE_STATE || info[i].instance_state == DDS::NOT_ALIVE_NO_WRITERS_INSTANCE_STATE) {
                cout << "Received termination request. Terminating." << endl;
                terminate = true;
                break;
             }
             /** If sample is valid, send it back to ping */
             else if(info[i].valid_data)
                e.message_writer->write(samples[i], DDS::HANDLE_NIL);
            }
            e.reader_i->return_loan(samples, info);
        }

  }catch (const CORBA::Exception& e) {
    e._tao_print_exception("Exception caught in main():");
    return -1;
  }
  return 0;
}

