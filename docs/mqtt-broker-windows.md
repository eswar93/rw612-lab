# MQTT broker on Windows (Mosquitto)

The board publishes to a broker running on my PC. **Analogy:** MQTT is a post office with labelled mailboxes. The board drops postcards into the mailbox `rw612/telemetry`, and a clerk on my PC (later a Python service) collects them.

## Install and configure

1. Install Mosquitto from mosquitto.org/download. On Windows the setup installs a service called "Mosquitto Broker" that starts automatically.
2. Mosquitto 2 only accepts local connections by default. To let the board connect, add this to the end of `C:\Program Files\mosquitto\mosquitto.conf` (edit as administrator):

```
listener 1883
allow_anonymous true
```

Anonymous access is fine for a home-network test. Add a username and password before this ever leaves my LAN.

3. Restart the service and open the port for my private network only (PowerShell as administrator):

```powershell
Restart-Service mosquitto
New-NetFirewallRule -DisplayName "Mosquitto MQTT 1883" -Direction Inbound -Protocol TCP -LocalPort 1883 -Action Allow -Profile Private -RemoteAddress LocalSubnet
```

If Windows lists the home Wi-Fi as a Public network, the rule does not apply. Switch the network to Private.

## Hotspots: set the network to Private

A phone hotspot (or the laptop's own hotspot) is usually marked **Public** by Windows, and the firewall rule above only applies to Private networks. Symptom: Wi-Fi works, but the board's MQTT connect fails with `-116` (a timeout). Check and fix, as administrator:

```powershell
Get-NetConnectionProfile
Set-NetConnectionProfile -InterfaceAlias "Wi-Fi" -NetworkCategory Private
```

Only do this for a network I control. Also: the PC's address on a hotspot can change when it reconnects, so run `ipconfig` again if the board starts timing out.

## Test it

Two terminals in `C:\Program Files\mosquitto`:

```powershell
.\mosquitto_sub.exe -h localhost -t "test/#" -v
.\mosquitto_pub.exe -h localhost -t "test/hello" -m "it works"
```

Then repeat the subscribe with the PC's LAN address (`ipconfig`) instead of `localhost`. That is what the board will do.

## Watch the board's messages

```powershell
.\mosquitto_sub.exe -h localhost -t "rw612/#" -v
```
