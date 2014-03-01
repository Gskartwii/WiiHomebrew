/*
 * networkstuff.h
 *
 *  Created on: 15.2.2014
 *      Author: gskw
 */

#ifndef NETWORKSTUFF_H_
#define NETWORKSTUFF_H_

#include <ogcsys.h>

const char *CRLF;
const u32 CRLF_LENGTH;

void initialise_network();
s32 server_connect(char* hostname);
bool request_file(s32 server, FILE *f);
extern u32 net_gethostip();
s32 send_message(s32 server, char *msg);
s32 httprep(s32 server, char *msg);
bool tcp_write(const s32 s, char *buffer, const u32 length);
u32 split(char *s, char sep, u32 maxsplit, char *result[]);
void filedl();

#endif /* NETWORKSTUFF_H_ */
