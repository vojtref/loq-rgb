#include <libusb-1.0/libusb.h>
#include <stdint.h>
#include <string.h>

#include "util/logger.h"
#include "util/options.h"

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
	uint8_t p[33] = {
	    0xcc,
	    0x16,
	    OPTIONS.mode,
	    OPTIONS.speed,
	    OPTIONS.brightness,
	};
	for (int z = 0; z < 4; z++) {
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
	    libusb_open_device_with_vid_pid(nullptr, 0x048d, 0xc993);
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
	                            0x21,   /* OUT | CLASS | INTERFACE */
	                            0x09,   /* HID SET_REPORT */
	                            0x03cc, /* FEATURE report, ID 0xcc */
	                            0x0000, /* interface 0 */
	                            p,
	                            sizeof(p),
	                            100);
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
