#include <set>
#include <thread>
#include <iostream>
#include <csignal>

#include "zserio/IPubsub.h"
#include "weather/WeatherTopics.h"
#include "ZsomeIpPubsub.h"

#define SAMPLE_SERVICE_ID       0x1234
#define SAMPLE_INSTANCE_ID      0x5678
#define SAMPLE_EVENTGROUP_ID    0x1357
#define SAMPLE_EVENT_ID         0x2468

namespace {
    volatile std::sig_atomic_t signalStatus;
}

void storeSignal(std::sig_atomic_t signal){
    signalStatus = signal;
}

class MyWeatherCallback : public weather::WeatherTopics::WeatherTopicsCallback<weather::WeatherDescription> {
public:
    void operator()(zserio::StringView topic, const weather::WeatherDescription &currentWeather) override {
        std::cout << "Weather is: " << currentWeather.getDescription() << std::endl;
    }
};

int main(int argc, char **argv) {
    std::string publishMode = "--publish";
    std::string subscribeMode = "--subscribe";

    bool runAsPublisher = false;
    if (argc == 2 && argv[1] == publishMode) {
        runAsPublisher = true;
    }
    else if (argc != 2 || argv[1] != subscribeMode) {
        std::cout << "Usage:" << std::endl
                  << "LD_LIBRARY_PATH=./deps/vsomeip \\" << std::endl
                  << "VSOMEIP_CONFIGURATION=../conf/pubsub-local.json \\" << std::endl
                  << "./zsomeip_pubsub_demo --<publish|subscribe>" << std::endl;
        exit(1);
    }

    signal (SIGINT,storeSignal);
    std::cout << "Press Ctrl+C to stop SOME/IP app..." << std::endl;

    std::string appName = runAsPublisher ? "zsomeip_publisher" : "zsomeip_subscriber";
    zsomeip::ZsomeIpPubsub zsomeIpPubsub(appName, false);
    weather::WeatherTopics weatherTopics(zsomeIpPubsub);

    zserio::StringView topicName("weather/current");
    zsomeip::AgentDefinition defaultAgent{SAMPLE_SERVICE_ID, SAMPLE_INSTANCE_ID};
    std::set<vsomeip_v3::eventgroup_t> defaultEventGroups = {SAMPLE_EVENTGROUP_ID};
    std::shared_ptr<zsomeip::TopicDefinition> weatherTopicDefinition(new zsomeip::TopicDefinition(
                    {topicName, defaultAgent, SAMPLE_EVENT_ID, defaultEventGroups}));

    if (runAsPublisher) {
        zsomeIpPubsub.addPublisher(weatherTopicDefinition);
    }
    else {
        std::shared_ptr<weather::WeatherTopics::WeatherTopicsCallback<weather::WeatherDescription>>
            callback(new MyWeatherCallback());
        weatherTopics.subscribeCurrentWeather(callback, static_cast<void*>(&weatherTopicDefinition));
    }

    weather::WeatherDescription currentWeather("same as always");
    while(signalStatus != SIGINT) {
        std::this_thread::sleep_for(std::chrono::seconds{1});
        if (runAsPublisher) {
            std::cout << "Publishing current weather: " << currentWeather.getDescription() << std::endl;
            weatherTopics.publishCurrentWeather(currentWeather, static_cast<void *>(&weatherTopicDefinition));
            std::this_thread::sleep_for(std::chrono::seconds{1});
        }
    }

    zsomeIpPubsub.shutdown();
    return 0;
};