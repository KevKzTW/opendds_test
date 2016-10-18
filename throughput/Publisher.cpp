#include <ace/Log_Msg.h>
#include <ctime>
#include <iostream>
using namespace std;


#include <dds/DdsDcpsInfrastructureC.h>
#include <dds/DdsDcpsPublicationC.h>

#include <dds/DCPS/Marked_Default_Qos.h>
#include <dds/DCPS/Service_Participant.h>
#include <dds/DCPS/WaitSet.h>

#include "dds/DCPS/StaticIncludes.h"

#include "ThroughputTypeSupportImpl.h"
/*
 * Function to handle Ctrl-C presses.
 * @param fdwCtrlType Ctrl signal type
 */
static bool terminated;
struct sigaction oldAction;
static void CtrlHandler(int fdwCtrlType)
{
    terminated = true;
}

int
ACE_TMAIN(int argc, ACE_TCHAR *argv[])
{
  struct sigaction sat;
  sat.sa_handler = CtrlHandler;
  sigemptyset(&sat.sa_mask);
  sat.sa_flags = 0;
  sigaction(SIGINT,&sat,&oldAction);

  try {
    unsigned long payloadSize = 8192;
    unsigned long burstInterval = 0;//us
    int burstSize = 1;
    terminated = false;



     if(argc > 3) 
       payloadSize = atoi(argv[3]); //The size of the payload in bytes
     if(argc > 4)
       burstInterval = atoi(argv[4]); //The time interval between each burst in ms
     if(argc > 5)
       burstSize = atoi(argv[5]); //The number of samples to send each burst


        cout << "payloadSize: " << payloadSize << " | burstInterval: " << burstInterval
            << " | burstSize: " << burstSize << "\n" << endl;

    ACE_Time_Value Interval = ACE_Time_Value(0,burstInterval);
    ACE_Time_Value time = ACE_Time_Value(0,0);
    ACE_Time_Value pubStart = ACE_Time_Value(0,0);
    ACE_Time_Value burstStart = ACE_Time_Value(0,0);


    // Initialize DomainParticipantFactory
    DDS::DomainParticipantFactory_var dpf =
      TheParticipantFactoryWithArgs(argc, argv);

    // Create DomainParticipant
    DDS::DomainParticipant_var participant =
      dpf->create_participant(42,
                              PARTICIPANT_QOS_DEFAULT,
                              0,
                              OpenDDS::DCPS::DEFAULT_STATUS_MASK);

    if (!participant) {
      ACE_ERROR_RETURN((LM_ERROR,
                        ACE_TEXT("ERROR: %N:%l: main() -")
                        ACE_TEXT(" create_participant failed!\n")),
                       -1);
    }

    // Register TypeSupport (Messenger::Message)
    Throughput::DataTypeTypeSupport_var ts =
      new Throughput::DataTypeTypeSupportImpl;

    if (ts->register_type(participant, "") != DDS::RETCODE_OK) {
      ACE_ERROR_RETURN((LM_ERROR,
                        ACE_TEXT("ERROR: %N:%l: main() -")
                        ACE_TEXT(" register_type failed!\n")),
                       -1);
    }

    // Create Topic (Movie Discussion List)
    CORBA::String_var type_name = ts->get_type_name();
    DDS::Topic_var topic =
      participant->create_topic("Throughput",
                                type_name,
                                TOPIC_QOS_DEFAULT,
                                0,
                                OpenDDS::DCPS::DEFAULT_STATUS_MASK);

    if (!topic) {
      ACE_ERROR_RETURN((LM_ERROR,
                        ACE_TEXT("ERROR: %N:%l: main() -")
                        ACE_TEXT(" create_topic failed!\n")),
                       -1);
    }

    // Create Publisher
    DDS::PublisherQos pubQos;
    participant->get_default_publisher_qos(pubQos);
    pubQos.partition.name.length(1);
    pubQos.partition.name[0] = "Throughput example";
    DDS::Publisher_var publisher =
      participant->create_publisher(pubQos,
                                    0,
                                    OpenDDS::DCPS::DEFAULT_STATUS_MASK);

    if (!publisher) {
      ACE_ERROR_RETURN((LM_ERROR,
                        ACE_TEXT("ERROR: %N:%l: main() -")
                        ACE_TEXT(" create_publisher failed!\n")),
                       -1);
    }

    // Configure DataWriter QoS policies.
      DDS::DataWriterQos dw_qos;
      publisher->get_default_datawriter_qos (dw_qos);
      dw_qos.reliability.kind  = DDS::RELIABLE_RELIABILITY_QOS;
      dw_qos.reliability.max_blocking_time = {10,0};
      dw_qos.resource_limits.max_samples_per_instance = 400;
      dw_qos.history.kind  = DDS::KEEP_ALL_HISTORY_QOS;

    // Create DataWriter
    DDS::DataWriter_var writer =
      publisher->create_datawriter(topic,
                                   dw_qos,
                                   0,
                                   OpenDDS::DCPS::DEFAULT_STATUS_MASK);

    if (!writer) {
      ACE_ERROR_RETURN((LM_ERROR,
                        ACE_TEXT("ERROR: %N:%l: main() -")
                        ACE_TEXT(" create_datawriter failed!\n")),
                       -1);
    }

    Throughput::DataTypeDataWriter_var message_writer =
      Throughput::DataTypeDataWriter::_narrow(writer);

    if (!message_writer) {
      ACE_ERROR_RETURN((LM_ERROR,
                        ACE_TEXT("ERROR: %N:%l: main() -")
                        ACE_TEXT(" _narrow failed!\n")),
                       -1);
    }

    // Block until Subscriber is available
    DDS::StatusCondition_var condition = writer->get_statuscondition();
    condition->set_enabled_statuses(DDS::PUBLICATION_MATCHED_STATUS);

    DDS::WaitSet_var ws = new DDS::WaitSet;
    ws->attach_condition(condition);

    while (true) {
      DDS::PublicationMatchedStatus matches;
      if (writer->get_publication_matched_status(matches) != ::DDS::RETCODE_OK) {
        ACE_ERROR_RETURN((LM_ERROR,
                          ACE_TEXT("ERROR: %N:%l: main() -")
                          ACE_TEXT(" get_publication_matched_status failed!\n")),
                         -1);
      }

      if (matches.current_count >= 1) {
        break;
      }

      DDS::ConditionSeq conditions;
      DDS::Duration_t timeout = { 60, 0 };
      if (ws->wait(conditions, timeout) != DDS::RETCODE_OK) {
        ACE_ERROR_RETURN((LM_ERROR,
                          ACE_TEXT("ERROR: %N:%l: main() -")
                          ACE_TEXT(" wait failed!\n")),
                         -1);
      }
    }

    ws->detach_condition(condition);

    // Write samples
    Throughput::DataType sample;
    sample.count = 0;
    sample.payload.length(payloadSize);
    for(unsigned long i=0;i<payloadSize;i++)
      sample.payload[i] = 'a';

    DDS::InstanceHandle_t handle = message_writer->register_instance(sample);
    cout << "Writing\n";

    int burstCount = 0;
    bool timeOut = false;
    pubStart = ACE_OS::gettimeofday();

    while(!terminated) {
   
      if(burstCount < burstSize) {
        DDS::ReturnCode_t error = message_writer->write(sample, DDS::HANDLE_NIL);
        if (error != DDS::RETCODE_OK) {
        ACE_ERROR((LM_ERROR,
                   ACE_TEXT("ERROR: %N:%l: main() -")
                   ACE_TEXT(" write returned %d!\n"), error));
		break;
        }
        sample.count++;
        burstCount++;
      }
      else if(burstInterval) {
        time = ACE_OS::gettimeofday()-pubStart;
        ACE_Time_Value deltaTime = time - burstStart;
	if(deltaTime < Interval) {
         	ACE_OS::sleep(Interval-deltaTime);
	}
	burstStart = ACE_OS::gettimeofday()-pubStart;
        burstCount = 0;
      }
      else {
        burstCount = 0;
      }
    }
    if(terminated)
      cout << "Terminated, " << sample.count << " samples written." << endl;
    else
      cout << "Error stop, " << sample.count << " samples written." << endl;

    // Clean-up!
    participant->delete_contained_entities();
    dpf->delete_participant(participant);

    TheServiceParticipant->shutdown();

  } catch (const CORBA::Exception& e) {
    e._tao_print_exception("Exception caught in main():");
    return -1;
  }
  sigaction(SIGINT,&oldAction, 0);
  return 0;
}
