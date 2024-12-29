#include "pico/stdlib.h"
#include "pico/multicore.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "fonts.h"
#include "deviceLegacy.h"
#include "deviceRTC.h"
#include "deviceEEPROM.h"
#include "deviceTouch.h"
#include "pixelDisplayDriver.h"
#include "pixelDisplayGC9A01A.h"
#include "pixelDisplayILI9341.h"
#include "pixelDisplaySH1106.h"
#include "pixelDisplaySH1122.h"
#include "pixelDisplayST7789.h"
#include "pixelDisplaySSD1306.h"
#include "pixelDisplaySSD1309.h"
#include "textDisplayDriver.h"
#include "textDisplayUS2066.h"
#include "color.h"

auto_init_mutex(mutex);

static deviceLegacy* device = NULL;
static bool refresh = false;

void render_loop() 
{
	const uint16_t width = 128;
	const uint16_t height = 64;
	const uint16_t xShift = 0;
	const uint16_t yShift = 0;
	const uint16_t bitsPerPixel = 1;

	spi_inst_t* spi = spi0;
	const uint32_t baudRate = 100 * 1000;
	const uint8_t rxPin = 13;
	const uint8_t sckPin = 10;
	const uint8_t csnPin = 9;
	const uint8_t rstPin = 12;
	const uint8_t dcPin = 8;
	const uint8_t backlightPin = 20;

	pixelDisplaySSD1309* display = new pixelDisplaySSD1309(width, height, xShift, yShift, bitsPerPixel);
	display->initSpi(spi, baudRate, rxPin, sckPin, csnPin, rstPin, dcPin, backlightPin);
	display->fill(0x000000);
	display->drawDisplay();

    while (true)
	{
		mutex_enter_blocking(&mutex);
		bool needRefresh = refresh;
		refresh = false;
		mutex_exit(&mutex);

		if (needRefresh == true)
		{
			display->fill(0x000000);
			for (int y = 0; y < device->getRows(); y++)
			{
				for (int x = 0; x < device->getCols(); x++)
				{
					display->drawChar(0xffffff, fonts::Font_6x8(), (x * 6) + 2, y << 3, device->getDisplayChar(y, x));
				}
			}
			display->drawDisplay();
		}
		sleep_ms(1000);
	}
}

int main() 
{
    stdio_init_all();

	spi_inst_t* spi = spi0;
	const uint32_t baudRate = 100 * 1000;
	const uint8_t rxPin = 0;
	const uint8_t csnPin = 1;
	const uint8_t sckPin = 2;

	device = new deviceLegacy(4, 20);
	device->initSpi(spi, baudRate, rxPin, sckPin, csnPin);
	multicore_launch_core1(render_loop);

	while (true)
	{
		bool needRefresh = device->poll();
		if (needRefresh == true)
		{
			mutex_enter_blocking(&mutex);
			refresh = true;
			mutex_exit(&mutex);
		}
		sleep_ms(1);
	}
}
