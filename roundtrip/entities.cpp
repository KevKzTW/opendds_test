#include <ace/Log_Msg.h>
#include <ctime>
#include <iostream>
using namespace std;

#include <dds/DdsDcpsInfrastructureC.h>
#include <dds/DdsDcpsPublicationC.h>
#include <dds/DdsDcpsSubscriptionC.h>

#include <dds/DCPS/Marked_Default_Qos.h>
#include <dds/DCPS/Service_Participant.h>
#include <dds/DCPS/WaitSet.h>

#include "dds/DCPS/StaticIncludes.h"

#include "RoundTripTypeSupportImpl.h"
#include "example_utilities.h"

class Entities 
{
public:
Entities(int argc, ACE_TCHAR *argv[],const char *pubPartition, const char *subPartition)
{
    // Initialize DomainParticipantFactory
    dpf = TheParticipantFactoryWithArgs(argc, argv);

    // Create DomainParticipant
    participant = dpf->create_participant(42,PARTICIPANT_QOS_DEFAULT,0,0);  

    // Register TypeSupport 
    ts = new RoundTripModule::DataTypeTypeSupportImpl;
    ts->register_type(participant, ""); 

    // Create Topic 
    CORBA::String_var type_name = ts->get_type_name();
    topic = participant->create_topic("RoundTrip",type_name,TOPIC_QOS_DEFAULT,0,OpenDDS::DCPS::DEFAULT_STATUS_MASK);

    // Create Publisher
    DDS::PublisherQos pubQos;
    participant->get_default_publisher_qos(pubQos);
    pubQos.partition.name.length(1);
    pubQos.partition.name[0] = pubPartition;
    publisher = participant->create_publisher(pubQos,0,OpenDDS::DCPS::DEFAULT_STATUS_MASK);
    

    // Configure DataWriter QoS policies.
    DDS::DataWriterQos dwQos;
    publisher->get_default_datawriter_qos (dwQos);
    dwQos.reliability.kind  = DDS::RELIABLE_RELIABILITY_QOS;
    dwQos.reliability.max_blocking_time = {10,0};
    dwQos.writer_data_lifecycle.autodispose_unregistered_instances = false;

    // Create DataWriter
    DDS::DataWriter_var writer = publisher->create_datawriter(topic,dwQos,0,OpenDDS::DCPS::DEFAULT_STATUS_MASK);
   
    message_writer = RoundTripModule::DataTypeDataWriter::_narrow(writer);


    // Create Subscriber
    DDS::SubscriberQos subQos;
    participant->get_default_subscriber_qos(subQos);
    subQos.partition.name.length(1);
    subQos.partition.name[0] = subPartition;
    subscriber = participant->create_subscriber(subQos,0,OpenDDS::DCPS::DEFAULT_STATUS_MASK);

    // Configure DataReader QoS policies.
    DDS::DataReaderQos drQos;
    subscriber->get_default_datareader_qos(drQos);
    drQos.reliability.kind = DDS::RELIABLE_RELIABILITY_QOS;
    drQos.reliability.max_blocking_time.sec = 10;

    // Create DataReader
    DDS::DataReader_var reader = subscriber->create_datareader(topic,drQos,NULL,OpenDDS::DCPS::DEFAULT_STATUS_MASK);  

    reader_i = RoundTripModule::DataTypeDataReader::_narrow(reader);

    dataAvailable = reader->get_statuscondition();
    dataAvailable->set_enabled_statuses(DDS::DATA_AVAILABLE_STATUS);

    ws = new DDS::WaitSet;

    ws->attach_condition(dataAvailable);

    roundTrip = exampleInitTimeStats();
    writeAccess = exampleInitTimeStats();
    readAccess = exampleInitTimeStats();
    roundTripOverall = exampleInitTimeStats();
    writeAccessOverall = exampleInitTimeStats();
    readAccessOverall = exampleInitTimeStats();
}
~Entities() 
{
    ws->detach_condition(dataAvailable);
    participant->delete_contained_entities();
    dpf->delete_participant(participant);

    exampleDeleteTimeStats (roundTrip);
    exampleDeleteTimeStats (writeAccess);
    exampleDeleteTimeStats (readAccess);
    exampleDeleteTimeStats (roundTripOverall);
    exampleDeleteTimeStats (writeAccessOverall);
    exampleDeleteTimeStats (readAccessOverall);

    TheServiceParticipant->shutdown();
}
public:
    /** The DomainParticipantFactory used by ping and pong */
    DDS::DomainParticipantFactory_var dpf;
    /** The DomainParticipant used by ping and pong */
    DDS::DomainParticipant_var participant;
    /** The TypeSupport for the sample */
    RoundTripModule::DataTypeTypeSupport_var ts;
    /** The Topic used by ping and pong */
    DDS::Topic_var topic;
    /** The Publisher used by ping and pong */
    DDS::Publisher_var publisher;
    /** The DataWriter used by ping and pong */
    RoundTripModule::DataTypeDataWriter_var message_writer;
    /** The Subscriber used by ping and pong */
    DDS::Subscriber_var subscriber;
    /** The DataReader used by ping and pong */
    RoundTripModule::DataTypeDataReader_var reader_i;
    /** The WaitSet used by ping and pong */
    DDS::WaitSet_var ws;
    /** The StatusCondition used by ping and pong,
     * triggered when data is available to read */
    DDS::StatusCondition_var dataAvailable;

    ExampleTimeStats roundTrip;
    ExampleTimeStats writeAccess;
    ExampleTimeStats readAccess;
    ExampleTimeStats roundTripOverall;
    ExampleTimeStats writeAccessOverall;
    ExampleTimeStats readAccessOverall;
};
    
