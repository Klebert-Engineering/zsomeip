#ifndef ZSOMEIP_ZSOMEIPAPP_H
#define ZSOMEIP_ZSOMEIPAPP_H

#include <thread>
#include <condition_variable>

#include "vsomeip/vsomeip.hpp"

namespace zsomeip {

class ZsomeIpApp {
public:
    explicit ZsomeIpApp(const std::string& appName);
    void shutdown();

protected:
    /* ZsomeIpApp implementations must call init() as soon as SOME/IP app
     * can be started and before calling any functions of the app_ object.
     * Internally, this will start run() in a private app thread and
     * wait until vsomeip app initialization is complete before returning.*/
    void init();

    /* ZsomeIpApp implementations must stop all event or service
     * offers when clear() is called. Otherwise, the app will not shut down. */
    virtual void clear() = 0;

    /* ZsomeIpApp implementations should always (re-)offer all available services
     * when receiving the onState call with internal variable registered == true.
     * Otherwise, the offer will not reach their clients. */
    virtual void onState() = 0;

    /* Set to true if the last onState call was with ST_REGISTERED,
     * and to false if it was ST_DEREGISTERED. */
    bool registered = false;

    std::shared_ptr <vsomeip::application> app_;

private:
    void run();
    virtual void on_state(vsomeip::state_type_e _state);
    std::unique_ptr<std::thread> app_thread_;
    std::mutex initialize_m_{};
    std::condition_variable initialized_{};
};

} // namespace zsomeip

#endif //ZSOMEIP_ZSOMEIPAPP_H
