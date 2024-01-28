/**
 * @file main.c
 * @author Jeronimo Agullo (jeronimoagullo97@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2023-12-23
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(weather_assistant, LOG_LEVEL_DBG);

#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/display/cfb.h>

const struct device *display_device;

/* Defining display stack area */
#define DISPLAY_STACK_SIZE 512
#define THREAD_DISPLAY_PRIORITY 5
K_THREAD_STACK_DEFINE(display_stack_area, DISPLAY_STACK_SIZE);
struct k_thread thread_display_data;

extern void thread_display(void* v1, void *v2, void *v3){

	uint16_t x_res, y_res;
	uint16_t rows, cols;
	uint8_t width, height;
	uint8_t ppt;

	if (display_set_pixel_format(display_device, PIXEL_FORMAT_MONO10) != 0) {
		if (display_set_pixel_format(display_device, PIXEL_FORMAT_MONO01) != 0) {
			LOG_ERR("Failed to set required pixel format");
		}
	}

	if (cfb_framebuffer_init(display_device)) {
		LOG_ERR("Framebuffer initialization failed!\n");
	}

	cfb_framebuffer_clear(display_device, true);
	display_blanking_off(display_device);

	x_res = cfb_get_display_parameter(display_device, CFB_DISPLAY_WIDTH);
	y_res = cfb_get_display_parameter(display_device, CFB_DISPLAY_HEIGH);
	rows = cfb_get_display_parameter(display_device, CFB_DISPLAY_ROWS);
	cols = cfb_get_display_parameter(display_device, CFB_DISPLAY_COLS);
	ppt = cfb_get_display_parameter(display_device, CFB_DISPLAY_PPT);

	// log display parameters
	LOG_INF("Display parameters: x_res %d, y_res %d, ppt %d, rows %d, cols %d",
	       x_res, y_res, ppt, rows, cols);

	// log supported display fonts
	LOG_INF("number of fonts: %d, indexes and sizes:", cfb_get_numof_fonts(display_device));
	for (int idx = 0; idx < cfb_get_numof_fonts(display_device); idx++) {
		cfb_get_font_size(display_device, idx, &width, &height);
		LOG_INF("font %d -> width %d, height %d", idx, width, height);
	}

	// get display width and height of font with index 0
	cfb_get_font_size(display_device, 0, &width, &height);

	cfb_framebuffer_invert(display_device);

	// Set font kerning (spacing between individual letters).
	cfb_set_kerning(display_device, 0);
	cfb_framebuffer_set_font(display_device, 0);

	while (1) {
		cfb_framebuffer_clear(display_device, false);

		// Display text in each of the 4 different lines
		cfb_print(display_device, "Zephyr RTOS!!",0,0);
		cfb_print(display_device, "ST & ssd1306",0,height);
		cfb_print(display_device, ">>Project",0,2 * height);
		cfb_print(display_device, "By Jeronimo :D",0,3 * height);

		// Finalize frame to load it into RAM to be displayed
		cfb_framebuffer_finalize(display_device);
		
		k_msleep(1000);
	}
}

int main(void)
{
	/* Init display */
	display_device = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
	if (!device_is_ready(display_device)) {
		LOG_ERR("Display %s not found. Aborting sample.",
			display_device->name);
	} else {
		LOG_INF("Display %s is ready", display_device->name);
	}

	k_tid_t idThreadDisplay = k_thread_create(&thread_display_data, display_stack_area,
                                K_THREAD_STACK_SIZEOF(display_stack_area),
                                thread_display,
                                NULL, NULL, NULL,
                                THREAD_DISPLAY_PRIORITY, 0, K_NO_WAIT);

    k_thread_start(idThreadDisplay);

	LOG_INF("end of main");

	return 0;
}
