/*!
 * @file networkstuff.c
 * @brief Networking functions/library.
 * @since 2014-02-16
 *
 * Various networking functions.
 * Credits to teknecal.
 */

#include <errno.h>
#include <malloc.h>
#include <math.h>
#include <network.h>
#include <ogcsys.h>
#include <gccore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/dir.h>
#include <time.h>
#include <wiiuse/wpad.h>
#include <unistd.h>
#include <fat.h>
#include <ogc/lwp_watchdog.h>
#include "networkstuff.h"
#include <debug.h>
#include "ctlaliases.h"


#define NET_BUFFER_SIZE 1024
#define BUFFER_SIZE 1024
#define IP_ADDRESS "85.14.201.130"
#define SOCKET_PORT 80

const char *CRLF="\r\n";
const u32 CRLF_LENGTH=2;
long bytes_count=0;

s32 httprep(s32 server, char *msg) {
	u32 msglen=strlen(msg);
	char msgbuf[msglen+1];
	if (msgbuf==NULL) {printf("ENOMEM");return -ENOMEM;}
	strcpy(msgbuf,msg);
	tcp_write(server,msgbuf,msglen);
	return 1;
}

bool tcp_write(const s32 s, char *buffer, const u32 length) {
	char *p;
	u32 step,left,block,sent;
	s64 t;
	s32 res;
	step=0;
	p=buffer;
	left=length;
	sent=0;
	t=gettime();
	while (left) {
		block=left;
		if (block>2048)
			block=2048;
		res = net_write (s, p, block);
		if ((res==0)||(res==-56)) {
			printf("\nDoing usleep!");
			usleep(20*1000);
			continue;
		}
		if (res<0)
			break;
		sent+=res;
		left-=res;
		p+=res;
		if ((sent/NET_BUFFER_SIZE)>step) {
			t=gettime();
			step++;
		}
	}
	return left==0;
}

bool request_file(s32 server, FILE *f) {
	char message[NET_BUFFER_SIZE];
	s32 bytes_read=net_read(server, message, sizeof(message));
	int length_til_data=0;
	int tok_count=2;
	char *temp_tok;
	if (bytes_read==0) return -1;
	temp_tok=strtok(message,"\n");
	while (temp_tok!=NULL) {
		if (strstr(temp_tok, "HTTP/1.1 4")|| strstr(temp_tok, "HTTP/1.1 5")) {
			printf("Server fault! %s\n", temp_tok);
			return -1;
		}
		if (strlen(temp_tok)==1) break;
		length_til_data+=strlen(temp_tok);
		tok_count++;
		temp_tok=strtok(NULL, "\n");
	}
	char store_data[NET_BUFFER_SIZE];
	int q;
	int i=0;
	for (q=length_til_data+tok_count;q<bytes_read;q++) {
		store_data[i]=message[q]; i++;
	}
	if (store_data!=NULL) {
		s32 bytes_written=fwrite(store_data, 1, i, f);
		if (bytes_written<i) {
			printf("fwrite error: [%i] %s\n", ferror(f), strerror(ferror(f)));
			sleep(1);
			return -1;
		}
	}
	while (bytes_read > 0) {
		bytes_read=net_read(server, message, sizeof(message));
		bytes_count += bytes_read;
		if (bytes_count >=2000) {
			bytes_count=0;
			printf(".");
		}
		s32 bytes_written=fwrite(message, 1, bytes_read, f);
		if (bytes_written<bytes_read) {
			printf("write error: [%i] %s\n", ferror(f), strerror(ferror(f)));
			sleep(1);
			return -1;
		}
	}
	return 1;
}

s32 server_connect(char* hostname) {
	struct sockaddr_in connect_addr;

	s32 server=net_socket(PF_INET, SOCK_STREAM, IPPROTO_IP);
	if (server < 0) printf("Error creating socket, exiting\n");
	//////////////////////////////////////////////////////////////////////////////////////////////////
	// DNS CODE
	//////////////////////////////////////////////////////////////////////////////////////////////////
	// Credits to Drmn4ea @ forum.wiibrew.org
	struct hostent *hp=net_gethostbyname(hostname);
	if (!hp || !(hp->h_addrtype==PF_INET)) {
		printf("net_gethostname failed: %d\n", errno);
		return errno;
	}
	//////////////////////////////////////////////////////////////////////////////////////////////////
	// DNS END
	//////////////////////////////////////////////////////////////////////////////////////////////////
	memset(&connect_addr, 0, sizeof(connect_addr));
	connect_addr.sin_family=PF_INET;
	connect_addr.sin_port=SOCKET_PORT;
	connect_addr.sin_len=sizeof(struct sockaddr_in);
	//connect_addr.sin_addr.s_addr=inet_addr(IP_ADDRESS);
	memcpy((char*) &connect_addr.sin_addr, hp->h_addr_list[0], hp->h_length);
	if (net_connect(server, (struct sockaddr*)&connect_addr, sizeof(connect_addr))==-1) {
		net_close(server);
		printf("Failed to connect to the remote server.\n");
	}
	return server;
}

void initialise_network() {
	printf("\n\nInitialising network...\n");
	printf("\n\nInitialising network...\n");
	s32 result=-1;
	while (result<0) {
		while ((result=net_init())== -EAGAIN) {

		}
		if (result<0) printf("Unable to initialise network, retrying...\n");
	}
	if (result>=0) {
		u32 ip=0;
		do {
			ip=net_gethostip();
			if (!ip) printf("Unable to initialise network, retrying...\n");
		} while(!ip);
		if (ip) printf("Done\n");
	}
}

typedef s32 (*transferrer_type)(s32 s, void *mem, s32 len);
inline static s32 transfer_exact(s32 s, char *buf, s32 length, transferrer_type transferrer) {
	s32 bytes_transferred=0;
	s32 remaining=length;
	while (remaining) {
		if ((bytes_transferred=transferrer(s,buf,remaining>NET_BUFFER_SIZE?NET_BUFFER_SIZE:remaining))>0) {
			remaining-=bytes_transferred;
			buf+=bytes_transferred;
		}
		else if (bytes_transferred<0) return bytes_transferred;
		else return -ENODATA;
	}
	return 0;
}
inline s32 write_exact(s32 s, char *buf, s32 length) {
	return transfer_exact(s,buf,length,(transferrer_type)net_write);
}

int filedl(char host[], char file[], char outfile[]) {
	printf("Attempting to connect to server...\n");
	s32 main_server=server_connect(host);
	printf("Connection successful.\n\n");
	printf("Outfile: %s\n", outfile);
	FILE *f=fopen(outfile, "wb");
	if (f==NULL) {
		fclose(f);
		printf("Could not create download file.\n");
	}
	printf("Downloading...\n");
	char http_request[1000];
	char row1[1000];
	char row2[1000];
	sprintf(row1, "GET /%s", file);
	sprintf(row2, " HTTP/1.0\r\nHost: %s\r\nCache-Control: no-cache\r\n\r\n", host);
	strcpy(http_request, row1);
	strcat(http_request, row2);
	httprep(main_server, http_request);
	int result=request_file(main_server, f);
	fclose(f);
	printf("Getting filesize Res=%d\n",result);
	f=fopen(outfile, "rb");
	//char ret[4096];
	int fsize;
	//char text[1000];
	//while (fgets(text,1000,f)) {
	//	sprintf(ret, "%s\n", text);
	//}
	//sleep(5);
	fseek(f, 0, SEEK_END);
	fsize=ftell(f);
	fclose(f);
	return fsize;
	/*if (dounlink) {
		unlink(outfile);
		printf("%s unlinked!\n", outfile);
	}*/
	//printf("Before strcpy");
	//strcpy(output, ret);
	//printf("After strcpy");
}

void readfile(char filename[], char output[], int dounlink, int filesize) {
	//printf("Readfile()");
	//while(!(GetCtlAlias(0)&WPAD_BUTTON_A)) VIDEO_WaitVSync();
	char ret[filesize+1];
	FILE* f;
	f=fopen(filename, "rb");
//	fgets(ret, filesize, f);
	int i;
	if (ferror(f)) {perror("Error opening file;");return;}
	for (i=0;i<filesize;i++) {
		printf("i=%d\n", i);
		//while(!(GetCtlAlias(0)&WPAD_BUTTON_A)) VIDEO_WaitVSync();
		ret[i]=(char)fgetc(f);
	}
	ret[i]='\0';
	//printf("Out in function: %s, strlen=%d, filesize=%d\n", ret, strlen(ret), filesize);
	strcpy(output, ret);
	//output=ret;
	if (dounlink) {
		unlink(filename);
		printf("%s unlinked!\n", filename);
	}
}
