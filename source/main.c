/** @file main.c
 *  @brief The main code file of the project.
 *  @author		Gskartwii <gskartwii@gskartwii.arkku.net>
 *  @version		1.2
 *  @since		2014-02-01
 *
 *  This file contains the screen printing function, updater and other basic code.
 */


#include <stdio.h>
#include <stdlib.h>
#include <gccore.h>
#include <wiiuse/wpad.h>
#include "libbrsar.h"
#include "networkstuff.h"
#include <fat.h>

/*! @var int currow
 * @brief The current chosen row.
 * @since 1.2
 *
 * This variable holds the current row the user has chosen.
 */
int currow=3;

/*! @var static void* xfb
 *	@brief Current External Frame Buffer.
 *	@warning Notice it is a static variable!
 *	@since 1.2
 *
 *	This variable contains the current External Farm Buffer.
 */
static void *xfb = NULL;

/*! @ṿar static GXRModeObj* rmode
 *  @brief Preferred video mode.
 *  @warning Notice it is a static variable!
 *  @since 1.2
 *
 *	This contains the preferred video mode set in the Wii settings.
 */
static GXRModeObj *rmode = NULL;

/*! @var char* screen[]
 *	@brief Strings to be printed.
 *	@since 1.2
 *
 *	This string array contains all the strings to be printed on the screen every frame.
 */
char *screen[]={
		"",
		"",
		"N_BLOCK_F",
		"N_BLOCK_N",
		"N_BOSSMI_32",
		"N_CASINO_F",
		"N_CASINO_N",
		"N_CIRCUIT32_F",
		"N_CIRCUIT32_N",
		"N_DAISY32_F",
		"N_DAISY32_N",
		"N_FARM_F",
		"N_FARM_N",
		"N_KINOKO_F",
		"N_KINOKO_N",
		"N_MAPLE_F",
		"N_MAPLE_N"
};

/*! @fn void updatescr()
 *  @brief Updates the screen.
 *  @since 1.2
 *  @see currow
 *  @see screen
 *
 *  Updates the screen every frame to have all the file names printed.
 */
void updatescr() {
	printf("\e[2;0HMenu of BRSAR's:\n");
	static int i;
	for (i=2;i<sizeof(screen)/sizeof(screen[0]);i++) {
		if (currow-1==i) printf("\e[%d;0H\e[32m%s\e[37m Offset: %2X", i+1, screen[i], offsetFromString(screen[currow-1]));
		else printf("\e[%d;0H\e[37m%s                 ", i+1, screen[i]);
	}
	printf("\n\e[37mCurrent row: %d, size=%d ",currow,sizeof(screen)/sizeof(screen[0]));
}

/*! @fn int main(int argv, char** argv)
 *  @brief The main program.
 *  @param argc Number of arguments.
 *  @param argv Array of argument strings.
 *  @return The exit code of the program.
 *  @since 1.2
 *
 *  This is the main program of the file. Everything is called from here.
 *
 *  @see rmode
 *  @see xfb
 *  @see initialise_network()
 *  @see currow
 *  @see filedl()
 *  @see updatescr()
 */
int main(int argc, char **argv) {

	// Initialise the video system
	VIDEO_Init();

	// This function initialises the attached controllers
	WPAD_Init();

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
	/*printf("\x1b[2;0H");

	printf("Menu of BRSTM's:\n");
	printf("\e[32mN_BLOCK_F\n"); // Row 3;0H
	printf("\e[37mN_BLOCK_N\n"); // Row 4;0H*/


	while(1) {

		// Call WPAD_ScanPads each loop, this reads the latest controller states
		WPAD_ScanPads();

		// WPAD_ButtonsDown tells us which buttons were pressed in this loop
		// this is a "one shot" state which will not fire again until the button has been released
		u32 pressed = WPAD_ButtonsDown(0);

		if ( pressed & WPAD_BUTTON_DOWN ) {
			if (currow==sizeof(screen)/sizeof(screen[0])) currow=3;
			else currow++;
		}
		else if (pressed & WPAD_BUTTON_UP) {
			if (currow==3) currow=sizeof(screen)/sizeof(screen[0]);
			else currow--;
		}
		else if (pressed & WPAD_BUTTON_A) {
			char file[4096];
			filedl("gskartwii.arkku.net", "latest.txt", &file);
			printf(file);
		}
		//updatescr();

		// Wait for the next frame
		VIDEO_WaitVSync();
	}

	return 0;
}
