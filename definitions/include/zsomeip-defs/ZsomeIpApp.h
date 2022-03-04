#ifndef ZSOMEIP_ZSOMEIPAPP_H
#define ZSOMEIP_ZSOMEIPAPP_H

#include <thread>

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

    std::shared_ptr <vsomeip::application> app_;

private:
    void run();
    std::thread app_thread_{};
};

} // namespace zsomeip

#endif //ZSOMEIP_ZSOMEIPAPP_H
