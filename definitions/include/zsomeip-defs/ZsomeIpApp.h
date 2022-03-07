#ifndef ZSOMEIP_ZSOMEIPAPP_H
#define ZSOMEIP_ZSOMEIPAPP_H

#include <thread>
#include <condition_variable>

#include "vsomeip/vsomeip.hpp"

namespace zsomeip {

class ZsomeIpApp {
public:
    /* ZsomeIpApp construction will start run() in a private app thread. */
    explicit ZsomeIpApp(const std::string& appName);
    void shutdown();

protected:
    /* ZsomeIpApp implementations must stop all event or service
     * offers when clear() is called. Otherwise, app will not shut down. */
    virtual void clear() = 0;

    virtual void on_state(vsomeip::state_type_e _state);
    void on_availability(vsomeip::service_t service, vsomeip::instance_t instance, bool available);

    std::shared_ptr <vsomeip::application> app_;
    bool registered_ = false;

private:
    void run();
    std::thread app_thread_{};
    std::mutex initialize_m_{};
    std::condition_variable initialized_{};
};

} // namespace zsomeip

#endif //ZSOMEIP_ZSOMEIPAPP_H
