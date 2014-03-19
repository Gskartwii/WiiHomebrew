#include <stdio.h>
#include <stdlib.h>
#include <gccore.h>
#include <wiiuse/wpad.h>
#include "libbrsar.h"
#include "networkstuff.h"
#include <fat.h>
#include <debug.h>
#include <network.h>
#include <sys/dir.h>
#include <dirent.h>
#include <unistd.h>
#include "ctlaliases.h"

#define CURR_VERSION 1

int currow=3;
static void *xfb = NULL;
static GXRModeObj *rmode = NULL;


int main(int argc, char **argv) {
	char printthis[4096];
	s32 ret;
	char localip[16] = {0};
	char gateway[16] = {0};
	char netmask[16] = {0};
	ret=if_config(localip, netmask, gateway, TRUE);
	if (ret>=0)
		sprintf(printthis, "network configured, ip: %s, gw: %s, mask %s\n", localip, gateway, netmask);

	// Initialise the video system
	VIDEO_Init();

	// This function initialises the attached controllers
	WPAD_Init();
	PAD_Init();

	// Obtain the preferred video mode from the system
	// This will correspond to the settings in the Wii menu
	rmode = VIDEO_GetPreferredMode(NULL);

	// Allocate memory for the display in the uncached region
	xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));

	// Initialise the console, required for printf
	console_init(xfb,20,20,rmode->fbWidth,rmode->xfbHeight,rmode->fbWidth*VI_DISPLAY_PIX_SZ);

	// Set up the video registers with the chosen mode
	VIDEO_Configure(rmode);

	// Tell the video hardware where our display memory is
	VIDEO_SetNextFramebuffer(xfb);

	// Make the display visible
	VIDEO_SetBlack(FALSE);

	// Flush the video register changes to the hardware
	VIDEO_Flush();

	// Wait for Video setup to complete
	VIDEO_WaitVSync();
	if(rmode->viTVMode&VI_NON_INTERLACE) VIDEO_WaitVSync();
	initialise_network();
	if (!fatInitDefault()) printf("Unable to initialise FAT subsystem, exiting.\nYour SD card may not be placed in the Wii correctly.");


	// The console understands VT terminal escape codes
	// This positions the cursor on row 2, column 0
	// we can use variables for this with format codes too
	// e.g. printf ("\x1b[%d;%dH", row, column );
	//printf("\x1b[2;0H");
	printf("\e[2;0H%s\n", printthis);
	printf("\e[HOME to exit, A to update.\n");
	DEBUG_Init(100, 5656);
	while(1) {
		// Call WPAD_ScanPads each loop, this reads the latest controller states
		WPAD_ScanPads();
		PAD_ScanPads();

		// WPAD_ButtonsDown tells us which buttons were pressed in this loop
		// this is a "one shot" state which will not fire again until the button has been released
		u32 pressed=GetCtlAlias(WPAD_CHAN_0);
		if (pressed & WPAD_BUTTON_HOME) {
			printf("Quitting!\n");
			return 0;
		}
		else if (pressed & WPAD_BUTTON_A) {
			DIR *brsardir=opendir("sd:/BRSAR");
			if (!brsardir) {
				mkdir("sd:/BRSAR", 0777);
				printf("Made dir BRSAR\n");
			}
			int fsize;
			fsize=filedl("gskartwii.arkku.net", "latest.txt", "sd:/BRSAR/latest.txt");
			//char file[fsize+1];
			char* file;
			file=(char*)malloc((fsize+1)*sizeof(char));
			readfile("sd:/BRSAR/latest.txt", file, 1, fsize);
			printf("\n\nFilesize: %d\nFile: %s\n", fsize, file);
//			free(file);
		}
		else if (pressed & WPAD_BUTTON_B) {
			printf("Expansion type: %X\n", (int)GetCtlType(WPAD_CHAN_0));
		}
		else if (pressed & WPAD_BUTTON_PLUS) {
			printf("Rumbling...\n");
			WPAD_Rumble(WPAD_CHAN_0, 1);
			sleep(1);
			printf("Stopping rumble!\n");
			WPAD_Rumble(WPAD_CHAN_0, 0);
		}

		// Wait for the next frame
		VIDEO_WaitVSync();
	}

	return 0;
}
