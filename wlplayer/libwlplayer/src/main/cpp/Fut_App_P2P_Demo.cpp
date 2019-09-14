
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <stdarg.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <string>

#include "Fut_App_P2P_Demo.h"
#include "AndroidLog.h"

///////////////////////////////////////////////////////////////////////////////////////////

#define MAX_ERROR_STRING_LEN 256
#define FUT_SERVER_DEFAULT_STRING "192.168.55.103:8989;192.168.55.104:8989"

///////////////////////////////////////////////////////////////////////////////////////////

static void Fut_OnP2PServer_Disconnect_Cb(p2p_transport *transport,
		int connection_id,
		void *transport_user_data,
		void *connect_user_data)
{
	//CP2PConnectProtocol *temp = (CP2PConnectProtocol*)transport_user_data;
	//temp->m_connectState = _FUT_DISCONECT_SERVER;
	
	//do nothing.
	LOGD("That Device-part connection is disconnected .\n");
	//LOGD("App-Client have conencted with p2p-server that is disconnected cn_id:%d \n", connection_id);
}

static void Fut_OnCreateConnectP2P_Complete_Cb(p2p_transport *transport,
		int status,
		void *user_data)
{
	CP2PConnectProtocol *temp = (CP2PConnectProtocol*)user_data;
	if (status == FUT_P2P_SUCCESS) 
	{
		temp->m_connectState = _FUT_CONNECT_SERVER_OK;
		LOGD("connect p2p server successful. \n");
	} 
	else 
	{
		temp->m_connectState = _FUT_CONNECT_SERVER_ERROR;

		char errmsg[MAX_ERROR_STRING_LEN] = {0};
		Fut_p2p_strerror(status, errmsg, sizeof(errmsg));
		LOGD("connect p2p server failed: %s. \n", errmsg);
	}
}

static void Fut_OnDisconnect_Server_Cb(p2p_transport *transport,
		int status,
		void *user_data)
{
	CP2PConnectProtocol *temp = (CP2PConnectProtocol*)user_data;
	temp->m_connectState = _FUT_DISCONECT_SERVER;

	char errmsg[MAX_ERROR_STRING_LEN] = {0};
	Fut_p2p_strerror(status, errmsg, sizeof(errmsg));
	LOGD("disconnect p2p server or can't connect p2p-server, %s \n", errmsg);
}

static void Fut_OnConnect_Complete_Cb(p2p_transport *transport,
		int connection_id,
		int status,
		void *transport_user_data,
		void *connect_user_data)
{
	CP2PConnectProtocol *temp = (CP2PConnectProtocol*)transport_user_data;

	if (status == FUT_P2P_SUCCESS) 
	{

		char addr[256] = {0};
		int len = sizeof(addr);
		Fut_EUM_p2p_addr_type conType;

		temp->m_connectState = _FUT_CONNECT_REMOTE_OK;
		Fut_get_p2p_conn_remote_addr(transport, connection_id, addr, &len,&conType);
		temp->m_p2pMode = (int)conType;

		LOGD("p2p connect remote user successful, connection id %d address:%s, contype = %d \n", connection_id, addr, conType);
	} 
	else 
	{
		temp->m_connectState = _FUT_CONNECT_REMOTE_ERROR;

		char errmsg[MAX_ERROR_STRING_LEN] = {0};
		Fut_p2p_strerror(status, errmsg, sizeof(errmsg));
		LOGD("p2p connect remote user failed: %s, connection id %d \n", errmsg, connection_id);
	}
}

static void Fut_OnDispatchSvr_Cb(void* dispatcher, int status, void* user_data, char* server, unsigned short port, unsigned int server_id)
{
	CP2PConnectProtocol *temp = (CP2PConnectProtocol*)user_data;
	if(status == FUT_P2P_SUCCESS)
	{
		//double check the server-str-msg
		if (strlen(server) < 3)
		{
			temp->m_connectState = _FUT_GET_DSSERVER_FAILED;
			LOGD("Fut_OnDispatchSvr_Cb failed, %d that maybe ts-server is offline.\n", status);
		}
		else
		{
			memset(temp->m_ts_server_str,0,sizeof(temp->m_app_cfg.server));
			strcpy(temp->m_ts_server_str,server);

			temp->m_app_cfg.server = temp->m_ts_server_str;
			temp->m_app_cfg.port = port;
			temp->m_connectState = _FUT_GET_DSSERVER_OK;

			LOGD("Fut_OnDispatchSvr_Cb Successed,Ts-Server infor: %s, %d, %d new-str:%s \n", server, port, server_id,temp->m_ts_server_str);
		}
	}
	else
	{
		temp->m_connectState = _FUT_GET_DSSERVER_FAILED;
		LOGD("Fut_OnDispatchSvr_Cb failed, %d \n", status);
	}

	//realse the dispatchserver.
	if (dispatcher != NULL)
	{
		Fut_destroy_p2p_dispatch_requester(dispatcher);
	}

}

void CP2PConnectProtocol::Fut_QuitP2PModule()
{
	m_quit_module = 1;	
}

CP2PConnectProtocol::CP2PConnectProtocol(char *p_in_user_str,char *p_in_dispsvr_str)
{
	m_proxy_svr_port = 0;
	m_quit_module = 0;

	memset(&m_app_cfg, 0, sizeof(m_app_cfg));
	memset(&m_app_cb, 0, sizeof(m_app_cb));

	memset(m_dispatch_svr_str,0,sizeof(m_dispatch_svr_str));
	if (strlen(p_in_dispsvr_str) > 4)
	{
		strcpy(m_dispatch_svr_str,p_in_dispsvr_str);
	}
	else
	{
		strcpy(m_dispatch_svr_str,"192.168.55.103:8989");
	}

	m_app_transport = 0;
	m_conn_id = -1;

	memset(m_ts_server_str,0,sizeof(m_ts_server_str));
	m_connectState = _FUT_CONNECT_INIT;

	memset(m_szFullUid, 0, MAX_STRING_LEN);
	strcpy(m_szFullUid, p_in_user_str);

	//init the p2p module.
	Fut_p2p_init(0);

	//set the log-level.
	//Fut_set_log_level(2);
}

CP2PConnectProtocol::~CP2PConnectProtocol()
{
	Fut_QuitP2PModule();
	usleep(500000);	//500 ms.

	if (m_app_transport)
	{		
		if (m_proxy_svr_port > 0)
		{
			LOGD("Release the P2P Connect Protocol that disconnect in destructor, id = %d", m_conn_id);
			Fut_destroy_p2p_tcp_proxy(m_app_transport, m_conn_id, m_proxy_svr_port);
			m_proxy_svr_port = 0;
		}

		if (m_conn_id != -1)
		{
			Fut_disconnect_p2p_transport(m_app_transport, m_conn_id);
			m_conn_id = -1;
		}

		Fut_destroy_p2p_transport(m_app_transport);
		m_app_transport = 0;
	}

	m_connectState = _FUT_CONNECT_INIT;
}

int CP2PConnectProtocol::Fut_ConnectDvsByP2PProtocol(int iType)
{
	if (1) //that must check the device to be found by LAN.
	{
		memset(&m_app_cfg, 0, sizeof(m_app_cfg));
		memset(&m_app_cb, 0, sizeof(m_app_cb));
		m_quit_module = 0;

		if(m_app_transport)
		{	
			LOGD("p2p transport already created that destroy it first!\n");
			if (m_proxy_svr_port > 0)
			{
				Fut_destroy_p2p_tcp_proxy(m_app_transport, m_conn_id, m_proxy_svr_port);
				m_proxy_svr_port = 0;
			}

			if (m_conn_id != -1)
			{
				Fut_disconnect_p2p_transport(m_app_transport, m_conn_id);
				m_conn_id = -1;
			}

			Fut_destroy_p2p_transport(m_app_transport);
			m_app_transport = 0;
		}

		//this is the dispatchserver/assignserver address.
		char dsserver[2*1024] = {0};

		if (strlen(m_dispatch_svr_str) > 4)
		{
			strcpy(dsserver,m_dispatch_svr_str);
		}
		else
		{
			strcpy(dsserver, FUT_SERVER_DEFAULT_STRING);
		}

		LOGD("query to assign-server address:%s \n",dsserver);

		void* dispatcher = NULL;
		int ret = Fut_query_p2p_dispatch_server( m_szFullUid, dsserver, this, Fut_OnDispatchSvr_Cb, &dispatcher);
		if (ret != 0)
		{
			LOGD("Fut_query_p2p_dispatch_server ret = %d,will return .\n",ret);
			return -1;
		}

		int time_count  = 1500;	//15 seconds.
		int m_try_count = 0;

		while(time_count && !m_quit_module)
		{
			usleep(10000);
			time_count--;

			if(m_connectState == _FUT_GET_DSSERVER_OK)
			{
				break;
			}
			else if(m_connectState == _FUT_GET_DSSERVER_FAILED)
			{
				LOGD("query dispatch server failed,will return .\n");

				if (dispatcher != NULL)
				{
					Fut_destroy_p2p_dispatch_requester(dispatcher);
				}

				m_try_count++;
				time_count  = 15000;
				if (m_try_count >= 3) 
					return -1;

				//try to reconnect the dispatchserver.
				ret = Fut_query_p2p_dispatch_server(m_szFullUid, dsserver, this, Fut_OnDispatchSvr_Cb, &dispatcher);
				if (ret != 0)
				{
					LOGD("Re-Try Fut_query_p2p_dispatch_server ret = %d,will return .\n",ret);
					return -1;
				}

			}

		}

		if (dispatcher != NULL)
		{
			Fut_destroy_p2p_dispatch_requester(dispatcher);
		}

		if(m_connectState == _FUT_CONNECT_INIT)
		{
			return -1;
		}

		if (m_quit_module == 1)
		{
			LOGD("p2p-module quit by %d first-time.\n",m_quit_module);
			return -1;
		}

		m_connectState = _FUT_CONNECT_INIT;
		m_conn_id = -1;
		m_proxy_svr_port = 0;
		m_quit_module = 0;

		m_app_cb.on_connect_complete = Fut_OnConnect_Complete_Cb;
		m_app_cb.on_create_complete = Fut_OnCreateConnectP2P_Complete_Cb;
		m_app_cb.on_connection_disconnect = Fut_OnP2PServer_Disconnect_Cb;
		m_app_cb.on_disconnect_server = Fut_OnDisconnect_Server_Cb;

		m_app_cfg.user_data = this;	//different app-client partment.
		m_app_cfg.cb = &m_app_cb;
		m_app_cfg.terminal_type = FUT_P2P_CLIENT_TERMINAL;
		m_app_cfg.relay_server_port = 5101;

		int status = Fut_create_p2p_transport(&m_app_cfg, &m_app_transport);
		if (status != FUT_P2P_SUCCESS) 
		{
			char errmsg[MAX_ERROR_STRING_LEN] = {0};
			Fut_p2p_strerror(status, errmsg, sizeof(errmsg));
			LOGD("Create p2p transport failed: %s \n", errmsg);

			return -1;
		}

		LOGD("Fut_create_p2p_transport successful.\n");

		// 10ms * 1500 = 15s
		int count2 = 1500;
		while(count2 && !m_quit_module)
		{
			usleep(10000);
			count2--;

			if(m_connectState == _FUT_CONNECT_SERVER_OK)
				break;
			else if(m_connectState == _FUT_CONNECT_SERVER_ERROR)
				break;
		}

		if (m_quit_module == 1)
		{
			LOGD("p2p-module quit by %d second-time. \n",m_quit_module);
			return -1;
		}

		if(m_app_transport && m_connectState == _FUT_CONNECT_SERVER_OK )
		{
			char username[32] = {"s1"};
			status = Fut_connect_p2p_transport(m_app_transport, m_szFullUid, 0, &m_conn_id);

			//if (iType == 0)
			//status = Fut_connect_p2p_transport(m_app_transport, m_szFullUid, 0, &m_conn_id);
			//else
			//	status = Fut_connect_tcp_transport(m_app_transport, m_szFullUid, 0, &m_conn_id);

			if (status != FUT_P2P_SUCCESS) 
			{
				char errmsg[MAX_ERROR_STRING_LEN] = {0};
				Fut_p2p_strerror(status, errmsg, sizeof(errmsg));
				LOGD("p2p connect remote user failed: %s ...\n", errmsg);

				Fut_destroy_p2p_transport(m_app_transport);
				m_app_transport = 0;
				return -1;
			}

			LOGD("Fut_connect_p2p_transport ok \n");
		}
		else
		{
			LOGD("Fut_connect_p2p_transport connect ts-server failed:%d .\n",m_connectState);
			return -1;
		}

		//waite to connect the remote device.
		// 10ms * 3000 = 30s
		count2 = 3000;
		while(count2 && !m_quit_module)
		{
			usleep(10000);
			count2--;

			if(m_connectState == _FUT_CONNECT_REMOTE_OK)
				break;
			else if(m_connectState == _FUT_CONNECT_REMOTE_ERROR)
				break;
		}
				
		if (m_quit_module)
		{
			LOGD("p2p-module quit by %d thrid-time. \n",m_quit_module);
			return -1;
		}

		//create the proxy server.
		if (m_connectState == _FUT_CONNECT_REMOTE_OK)
		{
			srand(time(0));
			int iFirstPort = rand() % 15000 + 20000;

			int status = Fut_create_p2p_tcp_proxy(m_app_transport, m_conn_id, iFirstPort, &m_proxy_svr_port);
			if(status == FUT_P2P_SUCCESS)
			{
				LOGD("p2p listen successful, port is %d, id = %d \n", m_proxy_svr_port, m_conn_id);
				return 1;
			}
			else
			{
				char errmsg[MAX_ERROR_STRING_LEN] = {0};
				Fut_p2p_strerror(status, errmsg, sizeof(errmsg));
				LOGD("p2p listen failed: %s \n", errmsg);

				LOGD("Create tcp proxy failed, P2P transport disconnect id = %d \n", m_conn_id);
				Fut_disconnect_p2p_transport(m_app_transport, m_conn_id);
				Fut_destroy_p2p_transport(m_app_transport);

				m_conn_id = -1;
				m_app_transport = 0;

				return -1;
			}
		}
		else
		{
			if (m_conn_id != -1)
			{
				Fut_disconnect_p2p_transport(m_app_transport, m_conn_id);
				m_conn_id = -1;
			}

			Fut_destroy_p2p_transport(m_app_transport);
			m_app_transport = 0;

			LOGD("p2p connect remote user failed ... \n");
			return -1;
		}

		return 1;
	}

	return 1;
}

