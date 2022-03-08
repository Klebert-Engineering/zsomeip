#ifndef ZSOMEIP_ZSOMEIPAPP_H
#define ZSOMEIP_ZSOMEIPAPP_H

#include <thread>
#include <condition_variable>

#include "vsomeip/vsomeip.hpp"

namespace zsomeip {

class ZsomeIpApp {
public:
    /* ZsomeIpApp constructor will start run() in a private app thread and
     * wait until vsomeip app initialization is complete before returning. */
    explicit ZsomeIpApp(const std::string& appName);
    void shutdown();

protected:
    /* ZsomeIpApp implementations must stop all event or service
     * offers when clear() is called. Otherwise, the app will not shut down. */
    virtual void clear() = 0;

    /* ZsomeIpApp implementations should only offer services when receiving the
     * onState(ST_REGISTERED) call. Otherwise, offer will not reach clients. */
    virtual void onState(vsomeip::state_type_e _state) = 0;

    /* Set to true if the last onState call was with ST_REGISTERED,
     * and to false if it was ST_DEREGISTERED. */
    bool registered_ = false;

    std::shared_ptr <vsomeip::application> app_;

private:
    void run();
    virtual void on_state(vsomeip::state_type_e _state);
    std::thread app_thread_{};
    std::mutex initialize_m_{};
    std::condition_variable initialized_{};
};

} // namespace zsomeip

#endif //ZSOMEIP_ZSOMEIPAPP_H
