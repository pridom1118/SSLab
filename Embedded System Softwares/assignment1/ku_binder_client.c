#include <stdio.h>
#include <string.h>
#include "ku_binder_lib.c"

/* Client application for the binder */

struct binder_msg my_msg[KBINDER_SNUM_MAX];
char request_sname[KBINDER_SNUM_MAX][KBINDER_NAME_MAX];

void print_menu() {
	printf("********** ku_binder_client **********\n");
	printf("-1: exit client\n");
	printf(" 0: get service number\n");
	printf(" 1: send the rpc request\n");
	printf("**************************************\n");
}

int main() {
	kbinder_init();
	
	int cmd, snum = -1; 
	int input_snum, input_fcode;
	double input_param;
	char input_sname[KBINDER_NAME_MAX];
	static union param my_param;

	print_menu();
	
	while(1) {
		scanf("%d", &cmd);

		switch(cmd) {
			case -1:
				printf("Exit Client\n");
				return 0;
			case 0:
				printf("service name: ");
				scanf("%s", input_sname);
				snum = kbinder_query(input_sname);

				if(snum == -1) {
					printf("%s is not registered yet.\n", input_sname);
					break;	
				} else printf("%s registered with snum: %d\n", input_sname, snum);
				
				strncpy(request_sname[snum], input_sname, KBINDER_NAME_MAX);
				my_msg[snum].snum = snum;
				memset(input_sname, 0, sizeof(input_sname));
				break;
			case 1:
				printf("Send the rpc request\n");
				printf("Input snum, fcode, param\n");
				scanf("%d %d %lf", &input_snum, &input_fcode, &input_param);

				if(strncmp(request_sname[input_snum], "vol", KBINDER_NAME_MAX) == 0) {
					my_param.user_vol.vol = input_param;
				} else if(strncmp(request_sname[input_snum], "led", KBINDER_NAME_MAX) == 0) {
					my_param.user_led.onoff = input_param;
				} else {
					printf("wrong snum. request cancelled.\n");
					break;
				}

				kbinder_rpc(input_snum, input_fcode, &my_param);
				printf("request sent\n");
				printf("========== request info ==========\n");
				printf("service: %s [%d]\n", request_sname[input_snum], input_snum);
				printf("fcode: %d\n", input_fcode);
				printf("param: %lf\n", input_param);
				printf("==================================\n");
				break;
		}
	}
	return 0;
}
