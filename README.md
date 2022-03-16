# zsomeip = zserio + SOME/IP

Reference implementation with examples for sending and receiving [zserio-encoded](https://zserio.org/) data
over [SOME/IP](https://some-ip.com/).

## Get started

Install prerequisites for [vsomeip](https://github.com/COVESA/vsomeip/):

```bash
sudo apt-get install libboost-system1.55-dev libboost-thread1.55-dev libboost-log1.55-dev
```

If not yet available on your system, install ``ant`` and ``java`` that are required by the
[zserio-cmake-helper](https://github.com/Klebert-Engineering/zserio-cmake-helper).
Java versions 8 and 11 are supported, please note that Java 17 does not work.

Clone this repository and run in the directory:
```bash
git submodule update --init --recursive
```
Then build with CMake. Setting GTEST_ROOT is not required for the weather example.

### Firewall configuration

To communicate with zsomeip across devices, allow incoming messages past the firewall.
To run publish-subscribe on two Linux machines, both using ufw, run:

```bash
ufw allow from <other-device-ip>
```

Service Discovery messages are passed bidirectionally, so this must be done on both devices.

Alternatively, you can enable communication over the ports in your configuration.

### Multicast route

Source: [vsomeip documentation](https://github.com/COVESA/vsomeip/blob/master/documentation/multicast.txt)

To use IP multicast, the route must be added. In Linux this can be done
by:
```bash
route add -net 224.0.0.0/4 dev eth0
```

Other OSes may have different ways to do this.

### Configuration

Adjust configuration files under ``conf``.
If you are running the example on separate devices, the unicast address must be set
to the host's IP address on all devices.

### Run publish-subscribe

Starting the subscriber:

```bash
LD_LIBRARY_PATH=./deps/vsomeip \
VSOMEIP_CONFIGURATION=../conf/pubsub-local.json \
./zsomeip_pubsub_demo --subscribe
```

Starting the publisher:

```bash
LD_LIBRARY_PATH=./deps/vsomeip \
VSOMEIP_CONFIGURATION=../conf/pubsub-local.json \
./zsomeip_pubsub_demo --publish
```

### Run client-service

Starting the service:

```bash
LD_LIBRARY_PATH=./deps/vsomeip \
VSOMEIP_CONFIGURATION=../conf/service-local.json \
./zsomeip_service_demo --service
```

Starting the client:

```bash
LD_LIBRARY_PATH=./deps/vsomeip \
VSOMEIP_CONFIGURATION=../conf/service-local.json \
./zsomeip_service_demo --client
```

## Use in your own project

Check the ``weather-example`` subdirectory for code references.
The rough steps are as follows:

1. Define your pubsub or service interface in Zserio. Refer to Zserio documentation on [service](https://github.com/ndsev/zserio/#services) and [pubsub](https://github.com/ndsev/zserio/#pubsub) types for further information, and [Weather.zs](weather-example/Weather.zs) in the demo.
2. Integrate Zserio sources into your project - refer to the weather-example library in [CMakeLists.txt](CMakeLists.txt). After building with CMake, the Zserio-generated C++ files can be found in your build directory under ``<cmake-project-name>.zserio-gen``.
3. If you defined a service interface, implement any ``get<methodName>Impl`` service functions. Refer to ``getTemperatureInImpl`` in the demo [weather service](weather-example/client-service.cpp).

   Basic client-service usage:
   ```cpp
   // Initialize method definition.
   zserio::StringView serviceMethod("get<methodName>");
   zsomeip::AgentDefinition defaultAgent{SAMPLE_SERVICE_ID, SAMPLE_INSTANCE_ID};
    std::shared_ptr<zsomeip::MethodDefinition> methodDef(
            new zsomeip::MethodDefinition(serviceMethod, defaultAgent, SAMPLE_METHOD_ID));
   
   // Service usage:
   <MyServiceImpl> myService{};
   zsomeip::ZsomeIpService zsomeIpService(APP_NAME, methodDef, myService);
   // Run in loop to process messages.
    
   // Client usage:
   zsomeip::ZsomeIpClient zsomeIpClient(APP_NAME, methodDef);
   <MyZserioGeneratedService::Client> myClient(zsomeIpClient);
   response = myClient.get<methodName>Method(<params>);
   
   // Final cleanup for both client and service.
   zsomeIpService.shutdown();
   ```
   
4. If you defined a publish-subscribe interface, implement any ``<pubsub-name>Callback`` classes. Refer to ``MyWeatherCallback`` in the demo [weather subscriber](weather-example/publish-subscribe.cpp).

    Basic publish-subscribe usage:

    ```cpp
   // Initialize publish-subscribe app.
    zsomeip::ZsomeIpPubsub zsomeIpPubsub(APP_NAME, false);
    MyZerioTopics myTopics(zsomeIpPubsub);
   
   // Initialize topic definition.
    zserio::StringView topicName("zserio/topic");
    zsomeip::AgentDefinition defaultAgent{SAMPLE_SERVICE_ID, SAMPLE_INSTANCE_ID};
    std::set<vsomeip_v3::eventgroup_t> defaultEventGroups = {SAMPLE_EVENTGROUP_ID};
   
    std::shared_ptr<zsomeip::TopicDefinition> myTopicDefinition(
        new zsomeip::TopicDefinition({topicName, defaultAgent, SAMPLE_EVENT_ID, defaultEventGroups}));
   
    // Publisher usage:
    zsomeIpPubsub.addPublisher(myTopicDefinition);
    weatherTopics.publish<topicName>(<topicItem>, static_cast<void *>(&myTopicDefinition));
   
   // Subscriber usage:
   std::shared_ptr<ZserioGeneratedTopics::ZserioGeneratedTopicsCallback<CallbackType>> callback(new <MyZserio>Callback());
   myTopics.subscribe<topicName>(callback, static_cast<void *>(&myTopicDefinition));
    // Run in loop to receive messages.
   
    // Final cleanup for both publisher and subscriber.
    zsomeIpPubsub.shutdown();
    ```
