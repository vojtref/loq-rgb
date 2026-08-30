#include <libusb-1.0/libusb.h>
#include <stdint.h>
#include <string.h>

#include "util/logger.h"
#include "util/options.h"

#define LENOVO_VENDOR_ID           0x048d
#define LENOVO_PRODUCT_ID_LOQ_2024 0xc993

#define RGB_REPORT_ID 0xcc
#define RGB_COMMAND   0x16

#define HID_SET_REPORT         0x09
#define USB_HID_FEATURE_REPORT 0x03

#define TRANSER_TIMEOUT_MS 1000

int main(int argc, char **argv)
{
	parse_opts(argc, argv);

	log_info("Options:");
	log_info("  Mode: 0x%02x", OPTIONS.mode);
	log_info("  Speed: 0x%02x", OPTIONS.speed);
	log_info("  Brightness: 0x%02x", OPTIONS.brightness);
	log_info("  Zone colors:");
	for (size_t z = 0; z < 4; ++z) {
		log_info("    %d: #%02x%02x%02x",
		         z,
		         OPTIONS.zone_rgb[z][0],
		         OPTIONS.zone_rgb[z][1],
		         OPTIONS.zone_rgb[z][2]);
	}

	log_debug("Assembling packet");
	uint8_t p[33] = {0};
	p[0]          = RGB_REPORT_ID;
	p[1]          = RGB_COMMAND;
	p[2]          = OPTIONS.mode;
	p[3]          = OPTIONS.speed;
	p[4]          = OPTIONS.brightness;
	for (size_t z = 0; z < 4; z++) {
		memcpy(&p[5 + 3 * z], OPTIONS.zone_rgb[z], 3);
	}

	int r = -1;

	log_debug("Initializing libusb");
	if (libusb_init(nullptr)) {
		log_error("Failed to initialize libusb");
		goto E0;
	}

	log_debug("Opening RGB controller handle");
	libusb_device_handle *dev =
	    libusb_open_device_with_vid_pid(nullptr,
	                                    LENOVO_VENDOR_ID,
	                                    LENOVO_PRODUCT_ID_LOQ_2024);
	if (!dev) {
		log_error("Failed to open RGB controller device handle");
		goto E1;
	}

	libusb_set_auto_detach_kernel_driver(dev, 1);

	log_debug("Claiming RGB controller interface");
	if (libusb_claim_interface(dev, 0)) {
		log_error("Failed to claim RGB controller interface");
		goto E2;
	}

	log_debug("Transferring packet");
	r = libusb_control_transfer(dev,
	                            LIBUSB_ENDPOINT_OUT |
	                                LIBUSB_REQUEST_TYPE_CLASS |
	                                LIBUSB_RECIPIENT_INTERFACE,
	                            HID_SET_REPORT,
	                            (USB_HID_FEATURE_REPORT << 8) | RGB_REPORT_ID,
	                            0, // interface
	                            p,
	                            sizeof(p),
	                            TRANSER_TIMEOUT_MS);
	if (r < 0) {
		log_error("libusb: %s", libusb_error_name(r));
	}

	log_debug("Releasing RGB controller interface");
	libusb_release_interface(dev, 0);
E2:
	log_debug("Closing RGB controller handle");
	libusb_close(dev);
E1:
	log_debug("Terminating libusb");
	libusb_exit(nullptr);
E0:
	return r == sizeof p ? 0 : 1;
}
