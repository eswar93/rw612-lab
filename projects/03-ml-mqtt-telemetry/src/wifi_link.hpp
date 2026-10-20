#pragma once

#include <zephyr/kernel.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/net_mgmt.h>

namespace telemetry {

struct WifiCredentials {
    const char *ssid;
    const char *psk;
    int security; /* same numbers as 'wifi connect -k' in Zephyr's wifi shell */
};

/* Connects to a Wi-Fi network and waits for an IPv4 address. No heap, no exceptions.
 * Only one WifiLink may exist: Zephyr's event callback has no user-data pointer. */
class WifiLink {
public:
    bool start();
    bool connect(const WifiCredentials &credentials, int timeout_ms);
    void disconnect();
    bool is_up() const { return link_up_ && has_ip_; }

private:
    static void on_event(net_mgmt_event_callback *cb, uint64_t event, net_if *iface);

    static WifiLink *instance_;

    net_if *iface_ = nullptr;
    net_mgmt_event_callback callback_{};
    k_sem connect_sem_{};
    k_sem ip_sem_{};
    volatile bool link_up_      = false;
    volatile bool has_ip_       = false;
    volatile int connect_status_ = -1;
};

} // namespace telemetry
