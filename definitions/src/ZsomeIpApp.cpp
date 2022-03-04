#include "ZsomeIpApp.h"
#include "vsomeip/vsomeip.hpp"

namespace zsomeip {

 ZsomeIpApp::ZsomeIpApp(const std::string& appName)
    : app_(vsomeip::runtime::get()->create_application(appName)),
      app_thread_(std::bind(&ZsomeIpApp::run, this)) {};

void ZsomeIpApp::run() {
    app_->init();
    app_->start();
}

void ZsomeIpApp::shutdown() {
    app_->clear_all_handler();
    clear();
    app_->stop();
    app_thread_.join();
}

} // namespace zsomeip