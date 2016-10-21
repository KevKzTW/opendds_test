/*
 *
 *
 * Distributed under the OpenDDS License.
 * See: http://www.opendds.org/license.html
 */

#include <ace/Log_Msg.h>
#include <iostream>
#include <iomanip>
#include <map>
using namespace std;

#include <dds/DdsDcpsInfrastructureC.h>
#include <dds/DdsDcpsSubscriptionC.h>

#include <dds/DCPS/Marked_Default_Qos.h>
#include <dds/DCPS/Service_Participant.h>
#include <dds/DCPS/WaitSet.h>

#include "dds/DCPS/StaticIncludes.h"

#include "ThroughputTypeSupportImpl.h"

#define BYTES_PER_SEC_TO_MEGABITS_PER_SEC 125000
#define DOMAIN_ID 42

unsigned long long sampleReceived(map<DDS::InstanceHandle_t,unsigned long long> &count1,
			          map<DDS::InstanceHandle_t,unsigned long long> &count2,	
				  bool preCount = false)
{
  unsigned long long total = 0;
  for(map<DDS::InstanceHandle_t,unsigned long long>::iterator it = count1.begin();it !=count1.end();it++) {
     total += it->second - count2[it->first];
     if(preCount) 
	count2[it->first] = it->second;
  }
  return total;
}


int
ACE_TMAIN(int argc, ACE_TCHAR *argv[])
{
  try {
    ACE_Time_Value time = ACE_Time_Value(0,0);
    ACE_Time_Value preTime = ACE_Time_Value(0,0);
    ACE_Time_Value startTime = ACE_Time_Value(0,0);
    streamsize ss = cout.precision();
    // Initialize DomainParticipantFactory
    DDS::DomainParticipantFactory_var dpf = TheParticipantFactoryWithArgs(argc, argv);

    // Create DomainParticipant
    DDS::DomainParticipant_var participant = dpf->create_participant(DOMAIN_ID,PARTICIPANT_QOS_DEFAULT,0,0);

    if(!participant) 
      ACE_ERROR_RETURN((LM_ERROR,ACE_TEXT("ERROR: %N:%l: main() -")ACE_TEXT(" create_participant failed!\n")), -1);

    // Register Type 
    Throughput::DataTypeTypeSupport_var ts = new Throughput::DataTypeTypeSupportImpl;
    if(ts->register_type(participant, "") != DDS::RETCODE_OK) 
      ACE_ERROR_RETURN((LM_ERROR,ACE_TEXT("ERROR: %N:%l: main() -")ACE_TEXT(" register_type failed!\n")), -1);

    // Create Topic 
    CORBA::String_var type_name = ts->get_type_name();
    DDS::Topic_var topic = participant->create_topic("Throughput",type_name,TOPIC_QOS_DEFAULT,0,OpenDDS::DCPS::DEFAULT_STATUS_MASK);
    if(!topic) 
      ACE_ERROR_RETURN((LM_ERROR,ACE_TEXT("ERROR: %N:%l: main() -")ACE_TEXT(" create_topic failed!\n")), -1);
    

    // Create Subscriber
    DDS::SubscriberQos subQos;
    participant->get_default_subscriber_qos(subQos);
    subQos.partition.name.length(1);
    subQos.partition.name[0] = "Throughput example";
    DDS::Subscriber_var subscriber = participant->create_subscriber(subQos,0,OpenDDS::DCPS::DEFAULT_STATUS_MASK);
    if (!subscriber) 
      ACE_ERROR_RETURN((LM_ERROR,ACE_TEXT("ERROR: %N:%l: main() -")ACE_TEXT(" create_subscriber failed!\n")), -1);

    // Configure DataWriter QoS policies.
    DDS::DataReaderQos drQos;
    subscriber->get_default_datareader_qos (drQos);
    drQos.reliability.kind  = DDS::RELIABLE_RELIABILITY_QOS;
    drQos.reliability.max_blocking_time = {10,0};
    drQos.resource_limits.max_samples_per_instance = 400;
    drQos.history.kind  = DDS::KEEP_ALL_HISTORY_QOS;

    // Create DataReader
    DDS::DataReader_var reader = subscriber->create_datareader(topic,drQos,0,OpenDDS::DCPS::DEFAULT_STATUS_MASK);
    if (!reader) 
      ACE_ERROR_RETURN((LM_ERROR,ACE_TEXT("ERROR: %N:%l: main() -")ACE_TEXT(" create_datareader failed!\n")), -1);


    Throughput::DataTypeDataReader_var reader_i = Throughput::DataTypeDataReader::_narrow(reader);
    if (!reader_i) 
      ACE_ERROR_RETURN((LM_ERROR,ACE_TEXT("ERROR: %N:%l: main() -")ACE_TEXT(" _narrow failed!\n")),-1);


    // Block until Publisher completes
    DDS::StatusCondition_var condition = reader->get_statuscondition();
    condition->set_enabled_statuses(DDS::DATA_AVAILABLE_STATUS);

    DDS::WaitSet_var ws = new DDS::WaitSet;
    ws->attach_condition(condition);

    map<DDS::InstanceHandle_t,unsigned long long> count;
    map<DDS::InstanceHandle_t,unsigned long long> startCount;
    map<DDS::InstanceHandle_t,unsigned long long> preCount;
    unsigned long long outOfOrder = 0;
    unsigned long long received = 0;
    unsigned long long preReceived = 0;
    unsigned long payloadSize;

    Throughput::DataTypeSeq samples(400);
    DDS::SampleInfoSeq infos(400);
    cout << "waiting\n";

    startTime = ACE_OS::gettimeofday();
    while(1) {

      DDS::ConditionSeq conditions;
      DDS::Duration_t timeout = { 10, 0 };
      ws->wait(conditions, timeout);    

      DDS::ReturnCode_t error = reader_i->take(samples,infos,400,DDS::ANY_SAMPLE_STATE,DDS::ANY_VIEW_STATE,DDS::ANY_INSTANCE_STATE);
      if(error != DDS::RETCODE_OK) {
        ACE_ERROR((LM_ERROR,ACE_TEXT("ERROR: %N:%l: on_data_available() -")ACE_TEXT(" take_next_sample failed!\n")));
	break;
      }

      for(unsigned long i=0;i<samples.length();i++) {
        if(infos[i].valid_data) {
 	   DDS::InstanceHandle_t ph = infos[i].publication_handle;
	   if(!startCount[ph] && !count[ph]) {
	      startCount[ph] = samples[i].count;
	      count[ph] = samples[i].count;
	   }
	   if(samples[i].count != count[ph])
              outOfOrder++;
	      count[ph] = samples[i].count+1;
              payloadSize = samples[i].payload.length();
              received += payloadSize+8;
	   }  
        }

      time = ACE_OS::gettimeofday()-startTime;
      if( time.sec()+(double)time.usec()/1000000 > preTime.sec()+(double)preTime.usec()/1000000 + 1.0) {
	if(preTime.sec()>0) {
	   unsigned long long deltaReceived = received - preReceived;
	   ACE_Time_Value deltaTime = time - preTime;
	   double deltaTime_sec = deltaTime.sec() + (double)deltaTime.usec()/1000000;

 	   cout << fixed << setprecision(2)
		<< "Payload size :"<< payloadSize << " | "
	   	<< "Total received :"<< sampleReceived(count,startCount) << " samples, "
	   	<< received << " bytes | "
    	   	<< "Out of order: " << outOfOrder << " samples | "
	   	<< "Transfer rate: " << (double)sampleReceived(count,preCount,true)/deltaTime_sec<< " samples/s , "
	   	<< ((double)deltaReceived / BYTES_PER_SEC_TO_MEGABITS_PER_SEC) / deltaTime_sec << " Mbits/s"
	   	<< endl;
	}

 	preCount = count;
	preReceived = received;
	preTime = time;
      }
      reader_i->return_loan(samples,infos);
    }

   
    ws->detach_condition(condition);

    ACE_Time_Value deltaTime = time;
    double deltaTime_sec = deltaTime.sec() + (double)deltaTime.usec()/1000000;
    cout << fixed << setprecision(2)
	 << "\nTotal received :"<< sampleReceived(count,startCount) << " samples, "
	 << received << " bytes\n"
    	 << "Out of order: " << outOfOrder << " samples\n"
	 << "Average transfer rate: " << (double)sampleReceived(count,startCount)/deltaTime_sec << " samples/s , "
	 << ((double)received / BYTES_PER_SEC_TO_MEGABITS_PER_SEC) / deltaTime_sec << " Mbits/s"
	 << endl
     	 << resetiosflags(ios::fixed) << setprecision(ss);
     
   
    

    // Clean-up!
    participant->delete_contained_entities();
    dpf->delete_participant(participant);

    TheServiceParticipant->shutdown();

  } catch (const CORBA::Exception& e) {
    e._tao_print_exception("Exception caught in main():");   
    return -1;
  }
  return 0;
}
