#include <iostream>
#include <random>
#include <csignal>
#include <thread>

#include "ZsomeIpService.h"
#include "weather/CurrentTemperatureService.h"

#define SAMPLE_SERVICE_ID       0x1111
#define SAMPLE_INSTANCE_ID      0x2222
#define SAMPLE_METHOD_ID        0x3333

namespace {
    volatile std::sig_atomic_t signalStatus;
}

void storeSignal(std::sig_atomic_t signal){
    signalStatus = signal;
}

class MyTemperatureService : public weather::CurrentTemperatureService::Service {
public:
    MyTemperatureService() : random_engine_(rd_()), random_distribution_(-40,50) {};

private:
    weather::Temperature getTemperatureInImpl(const ::weather::City& request, void* context) override {
        auto temperature = random_distribution_(random_engine_);
        std::cout << "Sending temperature in " << request.getName() << ": " << temperature << std::endl;
        return weather::Temperature(temperature);
    }

    std::random_device rd_;
    std::mt19937 random_engine_;
    std::uniform_int_distribution<int> random_distribution_;
};


int main(int argc, char **argv) {
    std::string serviceMode = "--service";
    std::string clientMode = "--client";

    bool runAsService = false;
    if (argc == 2 && argv[1] == serviceMode) {
        runAsService = true;
    }
    else if (argc != 2 || argv[1] != clientMode) {
        std::cout << "Usage:" << std::endl
                  << "LD_LIBRARY_PATH=./deps/vsomeip \\" << std::endl
                  << "VSOMEIP_CONFIGURATION=../conf/service-local.json \\" << std::endl
                  << "./zsomeip_service_demo --<service|client>" << std::endl;
        exit(1);
    }

    signal (SIGINT,storeSignal);
    std::cout << "Press Ctrl+C to stop SOME/IP app..." << std::endl;

    zserio::StringView serviceMethod("getTemperatureIn");
    zsomeip::AgentDefinition defaultAgent{SAMPLE_SERVICE_ID, SAMPLE_INSTANCE_ID};
    std::shared_ptr<zsomeip::MethodDefinition> methodDef(
            new zsomeip::MethodDefinition(serviceMethod, defaultAgent, SAMPLE_METHOD_ID));

    std::string appName = runAsService ? "zsomeip_service" : "zsomeip_client";

    if (runAsService) {
        MyTemperatureService service{};
        zsomeip::ZsomeIpService zsomeIpService(appName, methodDef, service);
        zsomeIpService.offerService();

        while (signalStatus != SIGINT) {
            std::this_thread::sleep_for(std::chrono::seconds{1});
        }

        std::cout << "Stopping zsomeip service..." << std::endl;
        zsomeIpService.shutdown();
    }
    else {
        zsomeip::ZsomeIpClient zsomeIpClient(appName, methodDef);

        weather::CurrentTemperatureService::Client weatherClient(zsomeIpClient);
        weather::City location("Tallinn");

        while (signalStatus != SIGINT) {
            std::cout << *methodDef << " Requesting... (timeout 10 seconds)" << std::endl;
            weather::Temperature currentTemperature;
            bool done = false;
            bool errored = false;
            std::thread weatherFetch([&] {
                try {
                    currentTemperature = weatherClient.getTemperatureInMethod(location);
                    done = true;
                } catch (const std::exception& e) {
                    errored = true;
                    std::cout << e.what();
                }
            });
            for (auto i = 0; i < 10; i++) {
                if (!done && signalStatus != SIGINT) {
                    std::this_thread::sleep_for(std::chrono::seconds{1});
                } else {
                    break;
                }
            }
            if (done) {
                weatherFetch.join();
                std::cout << "Current temperature in Tallinn: " << currentTemperature.getValue() << std::endl;
            } else {
                std::cout << *methodDef << " Request errored or timed out" << std::endl;
                weatherFetch.detach();
            }
        }
        std::cout << "Stopping zsomeip client..." << std::endl;
        zsomeIpClient.shutdown();
    }
}