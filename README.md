# zsomeip = zserio + SOME/IP

Reference implementation with examples for sending and receiving [zserio-encoded](https://zserio.org/) data
over [SOME/IP](https://some-ip.com/).

## Getting started

Install prerequisites for [vsomeip](https://github.com/COVESA/vsomeip/):

```bash
sudo apt-get install libboost-system1.55-dev libboost-thread1.55-dev libboost-log1.55-dev
```

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

## Run client-service

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
