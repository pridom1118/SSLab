#include <stdio.h>
#include <string.h>
#include "ku_binder_lib.c"

static int led;

struct binder_msg req;
int snum = -1;

/* LED */

void led_on() {
	led = 1;
}

void led_off() {
	led = 0;
}

void print_menu() {
	printf("********** ku_binder_server_led **********\n");
	printf("-1: exit server\n");
	printf(" 0: register service\n");
	printf(" 1: check service umber\n");
	printf(" 2: read the rpc request\n");
	printf(" 3: check the lastest request\n");
	printf(" 4; check current led\n");
	printf("**************************************\n\n");
}

void handle_request_led() {
	int fcode = req.fcode;

	switch (fcode) {
		case 0:
			printf("led off\n");
			led_off();
			break;
		case 1:
			printf("led on\n");
			led_on();
			break;
		default:
			printf("wrong fcode\n");
			break;
	}
	printf("current led: %s\n", led == 1 ? "ON" : "OFF");
}

int main() {	
	kbinder_init();

	print_menu();

	while(1) {
		int cmd;
		scanf("%d", &cmd);
		switch(cmd) {
			case -1:
				printf("Exit Server\n");
				return 0;
			case 0:
				snum = kbinder_reg("led");
				
				if(snum == -1) printf("SERVICE NOT REGISTERED\n");
				else printf("\"led\" registered with snum %d\n", snum);
				break;
			case 1:
				printf("current snum: %d\n", snum);
				break;
			case 2:
				if(snum == -1) {
					printf("SERVICE NOT REGISTERED, THEREFORE READ CANCELLED\n");
					break;
				}
				req.snum = snum;
				kbinder_read(snum, &req);
				handle_request_led();
				break;
			case 3:
				printf("check current rpc\n");
				printf("snum: %d\n", req.snum);
				printf("fcode: %d\n", req.fcode);
				printf("led: %d\n", req.param.user_led.onoff);
				break;
			case 4:
				printf("current led: %s\n", led == 1 ? "ON" : "OFF");
				break;
			default:
				break;
		}
	}
	return 0;
}
