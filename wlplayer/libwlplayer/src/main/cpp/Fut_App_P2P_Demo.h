
#ifndef _FUT_USE_P2P_SDK_DEMO_HEAD_D20190815_H_
#define _FUT_USE_P2P_SDK_DEMO_HEAD_D20190815_H_

#include "Fut_p2p_dispatch.h"
#include "Fut_p2p_transport.h" 

#define MAX_STRING_LEN 32


enum
{
	_FUT_CONNECT_INIT = 0x101,
	_FUT_CONNECT_SERVER_OK,
	_FUT_CONNECT_REMOTE_OK,
	_FUT_GET_DSSERVER_OK,
	_FUT_CONNECT_SERVER_ERROR,
	_FUT_CONNECT_REMOTE_ERROR,
	_FUT_DISCONECT_SERVER,
	_FUT_GET_DSSERVER_FAILED,
};

static void Fut_OnP2PServer_Disconnect_Cb(p2p_transport *transport,int connection_id,void *transport_user_data,void *connect_user_data);
static void Fut_OnCreateConnectP2P_Complete_Cb(p2p_transport *transport,int status,void *user_data);
static void Fut_OnDisconnect_Server_Cb(p2p_transport *transport,int status,void *user_data);
static void Fut_OnConnect_Complete_Cb(p2p_transport *transport,int connection_id,int status,void *transport_user_data,void *connect_user_data);
static void Fut_OnDispatchSvr_Cb(void* dispatcher, int status, void* user_data, char* server, unsigned short port, unsigned int server_id);

class CP2PConnectProtocol
{
	public:
		CP2PConnectProtocol(char *p_usr_info,char *p_in_dispsvr_str);
		~CP2PConnectProtocol();
		void Fut_QuitP2PModule();	//release or distroy the p2p module.
		int Fut_ConnectDvsByP2PProtocol(int iType);
	public:
		Fut_p2p_transport_cb  m_app_cb;
		Fut_p2p_transport_cfg m_app_cfg;
		p2p_transport *m_app_transport;

		int   m_conn_id;
		char m_ts_server_str[128];
		char m_dispatch_svr_str[128];
		unsigned short m_proxy_svr_port;

		int m_p2pMode;
		int m_connectState;

		int m_quit_module;
		char m_szFullUid[MAX_STRING_LEN];
};

#endif	//end of _FUT_USE_P2P_SDK_DEMO_HEAD_D20190815_H_ 

