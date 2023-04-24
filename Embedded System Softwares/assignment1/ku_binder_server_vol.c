#include <stdio.h>
#include <string.h>
#include "ku_binder_lib.c"

static double vol;

struct binder_msg req;
int snum = -1;

/* Volume */

void set_vol(double v) {
	vol = v;
}

void add_vol(double v) {
	vol += v;
}

void sub_vol(double v) {
	vol -= v;
}

void print_menu() {
	printf("********** ku_binder_server_vol **********\n");
	printf("-1: exit server\n");
	printf(" 0: register service\n");
	printf(" 1: check service umber\n");
	printf(" 2: read the rpc request\n");
	printf(" 3: check the lastest request\n");
	printf(" 4: check current volume\n");
	printf("**************************************\n\n");
}

void handle_request_vol() {
	int fcode = req.fcode;

	switch(fcode) {
		case 0:
			printf("set volume\n");
			set_vol(req.param.user_vol.vol);
			break;
		case 1:
			printf("add volume\n");
			add_vol(req.param.user_vol.vol);
			break;
		case 2:
			printf("subtract volume\n");
			sub_vol(req.param.user_vol.vol);
			break;
		default:
			printf("wrong fcode\n");
			break;
	}
	printf("current volume: %lf\n", vol);
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
				snum = kbinder_reg("vol");

				if(snum == -1) printf("SERVICE NOT REGISTERED\n");
				else printf("\"vol\" registered with snum %d\n", snum);
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
				handle_request_vol();
				break;
			case 3:
				printf("check current rpc\n");
				printf("snum: %d\n", req.snum);
				printf("fcode: %d\n", req.fcode);
				printf("vol: %lf\n", req.param.user_vol.vol);
				break;
			case 4:
				printf("current volume: %lf\n", vol);
				break;
			default:
				break;
		}
	}
	return 0;
}
