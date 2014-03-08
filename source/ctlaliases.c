#include <stdio.h>
#include <wiiuse/wpad.h>
#include <gccore.h>
#include <string.h>
#include "ctlaliases.h"

u32 GetCtlType(int ctl) {
	u32 devtype;
	s32 ret=WPAD_Probe(ctl, &devtype);
	if (ret!=WPAD_ERR_NONE) return -1;
	if (devtype == WPAD_EXP_NONE) return 0;
	if (devtype == WPAD_EXP_NUNCHUK) return 1;
	if (devtype == WPAD_EXP_CLASSIC) return 2;
	return devtype;
}

u32 GetCtlAlias(int ctl) {
	u32 ret=0;
	u32 pad=PAD_ButtonsDown(ctl);
	u32 classicbtns=WPAD_ButtonsDown(ctl);
	int devtype=GetCtlType(ctl);
	if (devtype==0)
		ret=WPAD_ButtonsDown(ctl);
	else if (devtype==1) {
		ret=WPAD_ButtonsDown(ctl);
		if (ret&WPAD_NUNCHUK_BUTTON_C)
			ret |= WPAD_BUTTON_A;
		if (ret&WPAD_NUNCHUK_BUTTON_Z)
			ret |= WPAD_BUTTON_B;
	}
	else if (devtype==2) {
		if (classicbtns&WPAD_CLASSIC_BUTTON_UP)
			ret |= WPAD_BUTTON_UP;
		if (classicbtns&WPAD_CLASSIC_BUTTON_LEFT)
			ret |= WPAD_BUTTON_LEFT;
		if (classicbtns&WPAD_CLASSIC_BUTTON_ZR)
			ret |= WPAD_BUTTON_PLUS;
		if (classicbtns&WPAD_CLASSIC_BUTTON_X)
			ret |= WPAD_BUTTON_PLUS;
		if (classicbtns&WPAD_CLASSIC_BUTTON_A)
			ret |= WPAD_BUTTON_A;
		if (classicbtns&WPAD_CLASSIC_BUTTON_Y)
			ret |= WPAD_BUTTON_MINUS;
		if (classicbtns&WPAD_CLASSIC_BUTTON_B)
			ret |= WPAD_BUTTON_B;
		if (classicbtns&WPAD_CLASSIC_BUTTON_ZL)
			ret |= WPAD_BUTTON_MINUS;
		if (classicbtns&WPAD_CLASSIC_BUTTON_FULL_R)
			ret |= WPAD_BUTTON_2;
		if (classicbtns&WPAD_CLASSIC_BUTTON_PLUS)
			ret |= WPAD_BUTTON_PLUS;
		if (classicbtns&WPAD_CLASSIC_BUTTON_HOME)
			ret |= WPAD_BUTTON_HOME;
		if (classicbtns&WPAD_CLASSIC_BUTTON_MINUS)
			ret |= WPAD_BUTTON_MINUS;
		if (classicbtns&WPAD_CLASSIC_BUTTON_FULL_L)
			ret |= WPAD_BUTTON_1;
		if (classicbtns&WPAD_CLASSIC_BUTTON_DOWN)
			ret |= WPAD_BUTTON_DOWN;
		if (classicbtns&WPAD_CLASSIC_BUTTON_RIGHT)
			ret |= WPAD_BUTTON_RIGHT;
	}
	if (ret==0&&pad!=0) {
		if (pad&PAD_BUTTON_LEFT)
			ret |= WPAD_BUTTON_LEFT;
		if (pad&PAD_BUTTON_RIGHT)
			ret |= WPAD_BUTTON_RIGHT;
		if (pad&PAD_BUTTON_DOWN)
			ret |= WPAD_BUTTON_DOWN;
		if (pad&PAD_BUTTON_UP)
			ret |= WPAD_BUTTON_UP;
		if (pad&PAD_TRIGGER_Z)
			ret |= WPAD_BUTTON_MINUS;
		if (pad&PAD_TRIGGER_R)
			ret |= WPAD_BUTTON_2;
		if (pad&PAD_TRIGGER_L)
			ret |= WPAD_BUTTON_1;
		if (pad&PAD_BUTTON_A)
			ret |= WPAD_BUTTON_A;
		if (pad&PAD_BUTTON_B)
			ret |= WPAD_BUTTON_B;
		if (pad&PAD_BUTTON_X)
			ret |= WPAD_BUTTON_PLUS;
		if (pad&PAD_BUTTON_Y)
			ret |= WPAD_BUTTON_MINUS;
		if (pad&PAD_BUTTON_START)
			ret |= WPAD_BUTTON_HOME;
	}
	return ret;
}
