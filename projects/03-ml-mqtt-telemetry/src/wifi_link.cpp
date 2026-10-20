#include "wifi_link.hpp"

#include <string.h>

#include <zephyr/net/dhcpv4.h>
#include <zephyr/net/wifi_mgmt.h>
#include <zephyr/sys/printk.h>

namespace telemetry {

WifiLink *WifiLink::instance_ = nullptr;

bool WifiLink::start()
{
    iface_ = net_if_get_first_wifi();
    if (iface_ == nullptr) {
        printk("No Wi-Fi interface found\n");
        return false;
    }

    instance_ = this;
    k_sem_init(&connect_sem_, 0, 1);
    k_sem_init(&ip_sem_, 0, 1);

    net_mgmt_init_event_callback(&wifi_callback_, &WifiLink::on_event,
                                 NET_EVENT_WIFI_CONNECT_RESULT | NET_EVENT_WIFI_DISCONNECT_RESULT);
    net_mgmt_add_event_callback(&wifi_callback_);

    net_mgmt_init_event_callback(&ipv4_callback_, &WifiLink::on_event, NET_EVENT_IPV4_ADDR_ADD);
    net_mgmt_add_event_callback(&ipv4_callback_);
    return true;
}

bool WifiLink::connect(const WifiCredentials &credentials, int timeout_ms)
{
    if (iface_ == nullptr) {
        return false;
    }

    link_up_ = false;
    has_ip_  = false;
    k_sem_reset(&connect_sem_);
    k_sem_reset(&ip_sem_);

    wifi_connect_req_params params{};
    params.ssid        = reinterpret_cast<const uint8_t *>(credentials.ssid);
    params.ssid_length = static_cast<uint8_t>(strlen(credentials.ssid));
    params.security    = static_cast<wifi_security_type>(credentials.security);
    if (credentials.security != WIFI_SECURITY_TYPE_NONE) {
        /* PSK types read the password from psk, SAE (WPA3) types from sae_password */
        params.psk                 = reinterpret_cast<const uint8_t *>(credentials.psk);
        params.psk_length          = static_cast<uint8_t>(strlen(credentials.psk));
        params.sae_password        = reinterpret_cast<const uint8_t *>(credentials.psk);
        params.sae_password_length = static_cast<uint8_t>(strlen(credentials.psk));
    }
    params.band    = WIFI_FREQ_BAND_UNKNOWN;
    params.channel = WIFI_CHANNEL_ANY;
    params.mfp     = WIFI_MFP_OPTIONAL;
    params.timeout = SYS_FOREVER_MS;

    const int rc = net_mgmt(NET_REQUEST_WIFI_CONNECT, iface_, &params, sizeof(params));
    if (rc != 0) {
        printk("Wi-Fi connect request failed: %d\n", rc);
        return false;
    }

    if (k_sem_take(&connect_sem_, K_MSEC(timeout_ms)) != 0) {
        printk("Wi-Fi connect timed out\n");
        disconnect();
        return false;
    }
    if (connect_status_ != 0) {
        printk("Wi-Fi connect failed, status %d\n", connect_status_);
        return false;
    }

    /* Zephyr normally starts DHCP by itself; if no address shows up, start it by hand. */
    if (!has_ip_ && k_sem_take(&ip_sem_, K_SECONDS(10)) != 0) {
        net_dhcpv4_start(iface_);
        if (!has_ip_ && k_sem_take(&ip_sem_, K_SECONDS(20)) != 0) {
            printk("No IPv4 address from DHCP\n");
            return false;
        }
    }
    return true;
}

void WifiLink::disconnect()
{
    if (iface_ != nullptr) {
        (void)net_mgmt(NET_REQUEST_WIFI_DISCONNECT, iface_, nullptr, 0);
    }
    link_up_ = false;
    has_ip_  = false;
}

void WifiLink::on_event(net_mgmt_event_callback *cb, uint64_t event, net_if *iface)
{
    ARG_UNUSED(iface);

    WifiLink *self = instance_;
    if (self == nullptr) {
        return;
    }

    if (event == NET_EVENT_WIFI_CONNECT_RESULT) {
        const auto *status  = reinterpret_cast<const wifi_status *>(cb->info);
        self->connect_status_ = (status != nullptr) ? status->status : -1;
        self->link_up_        = (self->connect_status_ == 0);
        printk("Wi-Fi connect result: status %d\n", self->connect_status_);
        k_sem_give(&self->connect_sem_);
    } else if (event == NET_EVENT_WIFI_DISCONNECT_RESULT) {
        printk("Wi-Fi disconnected\n");
        self->link_up_ = false;
        self->has_ip_  = false;
    } else if (event == NET_EVENT_IPV4_ADDR_ADD) {
        printk("IPv4 address acquired\n");
        self->has_ip_ = true;
        k_sem_give(&self->ip_sem_);
    }
}

} // namespace telemetry
