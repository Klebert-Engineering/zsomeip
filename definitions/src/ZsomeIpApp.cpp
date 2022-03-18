#include <iostream>

#include "ZsomeIpApp.h"
#include "vsomeip/vsomeip.hpp"

namespace zsomeip {

ZsomeIpApp::ZsomeIpApp(const std::string& appName) :
    app_(vsomeip::runtime::get()->create_application(appName)) {}

void ZsomeIpApp::init()
{
    app_thread_ = std::make_unique<std::thread>(std::bind(&ZsomeIpApp::run, this));
    std::unique_lock<std::mutex> initializing(initialize_m_);
    initialized_.wait(initializing);
}

void ZsomeIpApp::run()
{
    app_->init();
    if (app_->is_routing()) {
        registered = true;
    }
    app_->register_state_handler(std::bind(&ZsomeIpApp::on_state, this, std::placeholders::_1));
    initialized_.notify_one();
    app_->start();
}

void ZsomeIpApp::shutdown()
{
    app_->clear_all_handler();
    clear();
    app_->stop();
    app_thread_->join();
}

void ZsomeIpApp::on_state(vsomeip::state_type_e state)
{
    registered = (state == vsomeip::state_type_e::ST_REGISTERED);
    onState();
}

} // namespace zsomeip
