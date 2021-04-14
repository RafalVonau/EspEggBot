
//#define WIFI_SSID                "EggBot"
//#define WIFI_PASS                "EggBotPass"

#if defined(WIFI_SSID) && defined(WIFI_PASS)
	#define select_network() WiFi.begin ( WIFI_SSID, WIFI_PASS );
#else
	#define select_network() WiFi.begin();
#endif


#define detect_network() { \
	int n, i,j,got_one = 1, WifiCounter = 0; \
	pdebug("Connecting to WiFi network\n"); \
	WiFi.hostname(HOSTNAME); \
	WiFi.mode(WIFI_STA); \
	WiFi.disconnect(); \
	delay(100); \
	select_network(); \
	if (got_one) { \
		pdebug("Got one, connecting\n"); \
		while ( WiFi.status() != WL_CONNECTED ) { \
			pdebug("."); \
			delay (500); \
			WifiCounter++; \
			if (WifiCounter >=20) { \
				got_one = 0; \
				break; \
			} \
		} \
	} \
	if (got_one == 0) { \
		pdebug("Start WiFi manager\n"); \
		WiFiManager wifiManager; \
		wifiManager.autoConnect(HOSTNAME); \
	} \
	}

