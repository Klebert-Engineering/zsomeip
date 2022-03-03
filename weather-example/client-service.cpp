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

// TODO move app handling into client/service implementation
void runSomeIP(const std::shared_ptr<vsomeip::application>& app) {
    app->init();
    app->start();
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

    zserio::StringView serviceMethod("getTemperatureIn");
    zsomeip::AgentDefinition defaultAgent{SAMPLE_SERVICE_ID, SAMPLE_INSTANCE_ID};
    std::shared_ptr<zsomeip::MethodDefinition> methodDef(
            new zsomeip::MethodDefinition(serviceMethod, defaultAgent, SAMPLE_METHOD_ID));

    std::string appName = runAsService ? "zsomeip_service" : "zsomeip_client";
    std::shared_ptr<vsomeip::application> app = vsomeip::runtime::get()->create_application(appName);

    signal (SIGINT,storeSignal);
    std::cout << "Press Ctrl+C to stop SOME/IP app..." << std::endl;

    if (runAsService) {
        MyTemperatureService service{};
        zsomeip::ZsomeIpService zsomeIpService(app, methodDef, service);
        std::thread someIpThread(std::bind(&runSomeIP, app));

        // TODO use state handler instead of sleep
        std::this_thread::sleep_for(std::chrono::seconds{1});
        zsomeIpService.start();

        while(signalStatus != SIGINT) {
            std::this_thread::sleep_for(std::chrono::seconds{1});
        }

        app->clear_all_handler();
        app->stop();
        someIpThread.join();
    }
    else {
        zsomeip::ZsomeIpPubsub zsomeIpClient(app, methodDef);
        std::thread someIpThread(std::bind(&runSomeIP, app));

        // TODO use state handler instead of sleep
        std::this_thread::sleep_for(std::chrono::seconds{1});

        weather::CurrentTemperatureService::Client weatherClient(zsomeIpClient);
        weather::City location("Tallinn");

        while(signalStatus != SIGINT) {
            try {
                std::cout << *methodDef << " Requesting..." << std::endl;
                // TODO add request timeout to recover from errors
                auto currentTemperature = weatherClient.getTemperatureInMethod(location);
                std::cout << "Current temperature in Tallinn: " << currentTemperature.getValue() << std::endl;
            } catch (const std::exception& e) {
                std::cout << e.what() << std::endl;
                std::cout << "Stopping zsomeip client..." << std::endl;
                break;
            }
            std::this_thread::sleep_for(std::chrono::seconds{2});
        }
        app->clear_all_handler();
        app->stop();
        someIpThread.join();
    }
}