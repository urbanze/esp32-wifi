#ifndef wifi_H
#define wifi_H

#include <esp_err.h>

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sys.h"

/**
  	@file WiFi.h
	@author Jose Morais

	WiFi Class (STA, AP, AP_STA) to ESP32.


	MIT License

	Copyright (c) 2020 José Morais

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
 */

class WF
{
	private:
		const char tag[5] = "WiFi"; ///< TAG to ESP_LOGx().

		int8_t started = 0; ///< Start WiFi driver once time.
		static int8_t wf_sts; ///< Last WiFi status.

		static int8_t sta_sts; ///< Last STA status.
		static int16_t sta_dscrsn; ///< Last STA disconnect reason.
		static char sta_ip[16]; ///< STA IP Address.

		static int8_t ap_sts; ///< Last AP status.
		

		static esp_err_t event_handler(void *ctx, system_event_t *event); ///< Event handler.

		int8_t init(); ///< Init WiFi driver.
		

	public:
		//Generic functions
		int8_t status();
		int8_t mode();

		//STA functions
		void sta_static_ip(const char *ip, const char *gateway, const char *mask);
		void sta_connect(const char *ssid, const char *pass, int8_t wait);
		void sta_disconnect();
		void sta_reconnect();
		char *sta_get_ip();
		int8_t sta_status();
		int16_t sta_disconnect_reason();

		//AP functions
		void ap_start(const char *ssid, const char *pass, int8_t channel, int8_t max, int8_t hidden);
		void ap_stop();
		int8_t ap_status();
	
			

};


#endif
