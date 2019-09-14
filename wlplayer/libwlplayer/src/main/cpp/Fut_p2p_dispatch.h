
#ifndef _P2P_COMMINC_DISPATCH_D20190719_H_
#define _P2P_COMMINC_DISPATCH_D20190719_H_

#include "Fut_p2p_transport.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*DISPATCH_CALLBACK)(void* dispatcher, int status, void* user_data, char* server, unsigned short port, unsigned int server_id); 

FUT_P2P_DECL(int) Fut_request_p2p_dispatch_server(char* user,char* password,char* ds_addr,void* user_data, DISPATCH_CALLBACK cb,void** dispatcher);

FUT_P2P_DECL(int) Fut_query_p2p_dispatch_server(char* dest_user,char* ds_addr,void* user_data, DISPATCH_CALLBACK cb,void** dispatcher);

FUT_P2P_DECL(void) Fut_destroy_p2p_dispatch_requester(void* dispatcher);

#ifdef __cplusplus
}
#endif

#endif

