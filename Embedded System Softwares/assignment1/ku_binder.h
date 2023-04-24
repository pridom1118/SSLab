#define IOCTL_START_NUM 0x80
#define IOCTL_NUM1 IOCTL_START_NUM+1
#define IOCTL_NUM2 IOCTL_START_NUM+2
#define IOCTL_NUM3 IOCTL_START_NUM+3
#define IOCTL_NUM4 IOCTL_START_NUM+4

#define BINDER_IOCTL_NUM 'z'
#define BINDER_RPC _IOWR(BINDER_IOCTL_NUM, IOCTL_NUM1, unsigned long *)
#define BINDER_READ _IOWR(BINDER_IOCTL_NUM, IOCTL_NUM2, unsigned long *)
#define BINDER_REG _IOWR(BINDER_IOCTL_NUM, IOCTL_NUM3, char *)
#define BINDER_QUERY _IOWR(BINDER_IOCTL_NUM, IOCTL_NUM4, char *)

#define KBINDER_NAME_MAX 11
#define KBINDER_SNUM_MAX 5
#define KBINDER_PARAM_MAX 20

/* Structs for the services */

struct volume {
	double vol;
};

struct led {
	int onoff;
};

struct lcd {
	char str[16];
	int onoff;
};

struct rgb_led {
	int red, green, blue;
	int onoff;
};

struct distance_sensor {
	float distance;
};

struct special_sensor {
	float val, threshold;
	char desc[11];
};

union param {
	struct volume user_vol;
	struct led user_led;
	struct lcd user_lcd;
	struct rgb_led user_rgb;
	struct distance_sensor user_dist;
	struct special_sensor user_special;
};

struct binder_msg {
	int fcode;
	union param param;
	int snum;
};

struct msg {
	int fcode;
	union param param;
};
