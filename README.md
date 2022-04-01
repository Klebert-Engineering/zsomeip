# zsomeip = zserio + SOME/IP

Reference implementation with examples for sending and receiving [zserio-encoded](https://zserio.org/) data over [SOME/IP](https://some-ip.com/). 

Have a look at the **Getting started** section to run a small weather service, or the **Building your own app** section below for instructions on how to use zsomeip in a custom client-service or publish-subscribe app.

For a nice introduction to the SOME/IP reference implementation used by this project, refer to the [vsomeip in 10 minutes](https://github.com/COVESA/vsomeip/wiki/vsomeip-in-10-minutes) tutorial.

## Getting started

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

The weather example uses SOME/IP service discovery, which relies on IP multicast to make applications aware of each other.

To use IP multicast, the route must first be added. On Linux this can be done by:
```bash
route add -net 224.0.0.0/4 dev eth0
```

Other OSes may have different ways to do this.

Source: [vsomeip documentation](https://github.com/COVESA/vsomeip/blob/master/documentation/multicast.txt)

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

## Building your own app

### Choosing the right zserio version

Service interfaces changed with the zserio 2.5.0 release. While zsomeip is currently configured to work with zserio 2.4.2 out of the box, you can build using zserio >= 2.5.0 by setting the following option in your project's ``CMakeLists.txt``:

```cmake
set(ZSOMEIP_USE_ZSERIO_2_5_0_SERVICE_INTERFACE ON)
```
In the future, zserio 2.4.2 support will be removed.

If you change the zserio version of an existing CMake build, you must also delete the build directory and build from scratch.
The build error below happens when already built zserio tools conflict with the new version:
```bash
java.lang.IncompatibleClassChangeError: Found class zserio.tools.ExtensionParameters, but interface was expected
```

### Basic components

Check the ``weather-example`` subdirectory for code references.
The rough implementation steps are as follows:

1. Define your pubsub or service interface in Zserio. Refer to Zserio documentation on [service](https://github.com/ndsev/zserio/#services) and [pubsub](https://github.com/ndsev/zserio/#pubsub) types for further information, and [Weather.zs](weather-example/Weather.zs) in the demo.

2. Integrate Zserio sources into your project - refer to the weather-example library in [CMakeLists.txt](CMakeLists.txt). After building with CMake, the Zserio-generated C++ files can be found in your build directory under ``<cmake-project-name>.zserio-gen``.

3. If you defined a service interface, implement any ``get<methodName>Impl`` service functions. Refer to ``getTemperatureInImpl`` in the demo [weather service](weather-example/client-service.cpp).

4. If you defined a publish-subscribe interface, implement any ``<pubsubName>Callback`` classes. Refer to ``MyWeatherCallback`` in the demo [weather subscriber](weather-example/publish-subscribe.cpp).

5. Adjust the configuration files to your scenario. Sample configurations are stored under ``conf``, but you can create new ones anywhere. For an explanation of the IDs, see below.

6. Implement the client-service or publish-subscribe controller apps - refer to code snippets below for basic usage.

7. Start your program executables using:
   ```bash
   LD_LIBRARY_PATH=<path/to/deps/vsomeip> \
   VSOMEIP_CONFIGURATION=<path/to/your/configuration.json> \
   ./<executable_name>
   ```

### Basic zsomeip configuration

zsomeip invocations in C++ require a service ID and an instance ID. Services and publishers use these IDs to register themselves as SOME/IP content providers, while clients and subscribers use the ID to look up their required services or publishers.

Publish-subscribe applications also require event group and event IDs. Publishers send messages with a specific event ID, whereas subscribers sign up for an event group and receive notifications for all events in the group. For the simplest case, use zsomeip with event groups containing a single event.

Application names can be mapped to IDs in the configuration files. If you use the same identifier in your code (see APP_NAME below), your app is assigned the matching ID, making it easier to understand logs. Make sure that app names are unique for all running zsomeip instances.

SOME/IP supports UDP (unreliable) and TCP (reliable) transport protocols. In the configuration files, service port configuration determines the protocol. It must match the reliability setting of the method definition in your code, see USE_TCP below. 

Application names are not related to offered services, event groups, or other configuration elements, with one exception: you can specify which application should serve as routing manager by using the optional "routing" key in the configuration files.

Make sure the IDs used in your zsomeip controller are the same as in the configuration file, e.g. by creating the necessary defines:

```cpp
#define APP_NAME                "<appName>"

#define SAMPLE_SERVICE_ID       0x1111
#define SAMPLE_INSTANCE_ID      0x2222

// For client-service
#define SAMPLE_METHOD_ID        0x3333
#define USE_TCP                 false

// For publish-subscribe
#define SAMPLE_EVENTGROUP_ID    0x4444
#define SAMPLE_EVENT_ID         0x5555
```

### Basic client-service controller

```cpp
// Initialize method definition.
zserio::StringView serviceMethod("get<methodName>");
zsomeip::AgentDefinition defaultAgent{SAMPLE_SERVICE_ID, SAMPLE_INSTANCE_ID};
std::shared_ptr<zsomeip::MethodDefinition> methodDef(
    new zsomeip::MethodDefinition(serviceMethod, defaultAgent, SAMPLE_METHOD_ID, USE_TCP));

// Service usage:
<MyServiceImpl> myService{};
zsomeip::ZsomeIpService zsomeIpService(APP_NAME, methodDef, myService);
// Run in loop to process messages.
 
// Client usage:
zsomeip::ZsomeIpClient zsomeIpClient(APP_NAME, methodDef);
<ZserioGeneratedService::Client> myClient(zsomeIpClient);
response = myClient.get<methodName>Method(<params>);

// Final cleanup for both client and service.
zsomeIpService.shutdown();
   ``` 

### Basic publish-subscribe controller

 ```cpp
// Initialize publish-subscribe app.
zsomeip::ZsomeIpPubsub zsomeIpPubsub(APP_NAME, false);
MyZerioTopics myTopics(zsomeIpPubsub);

// Initialize topic definition.
zserio::StringView topicString("zserio/topic");
zsomeip::AgentDefinition defaultAgent{SAMPLE_SERVICE_ID, SAMPLE_INSTANCE_ID};
std::set<vsomeip_v3::eventgroup_t> defaultEventGroups = {SAMPLE_EVENTGROUP_ID};

std::shared_ptr<zsomeip::TopicDefinition> myTopicDefinition(
    new zsomeip::TopicDefinition({topicString, defaultAgent, SAMPLE_EVENT_ID, defaultEventGroups}));

// Publisher usage:
zsomeIpPubsub.addPublisher(myTopicDefinition);
auto topicItem = ... // Build the object to be sent via zsomeip. 
weatherTopics.publish<topicName>(topicItem, static_cast<void *>(&myTopicDefinition));

// Subscriber usage:
std::shared_ptr<ZserioGeneratedTopics::ZserioGeneratedTopicsCallback<CallbackType>> callback(
    new <pubsubName>Callback());
myTopics.subscribe<topicName>(callback, static_cast<void *>(&myTopicDefinition));
// Run in loop to receive messages.

// Final cleanup for both publisher and subscriber.
zsomeIpPubsub.shutdown();
 ```
