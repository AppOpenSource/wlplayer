
#ifndef _P2P_COMMINC_TRANSPORT_D20190719_H_
#define _P2P_COMMINC_TRANSPORT_D20190719_H_

#ifdef __cplusplus
extern "C" {
#endif

/////////////////////////////////////////////////////////////////////////////////////

#define FUT_P2P_DEVICE_TERMINAL 	0
#define FUT_P2P_CLIENT_TERMINAL 	1
#define FUT_INET6_ADDR_STRLEN		46

#define FUT_P2P_DECL(type) type
#define FUT_P2P_SUCCESS 		0
/////////////////////////////////////////////////////////////////////////////////////

typedef struct p2p_transport p2p_transport;

typedef struct _Fut_p2p_transport_cb_
{
	void (*on_create_complete)(p2p_transport *transport,
		int status,
		void *user_data);

	void (*on_disconnect_server)(p2p_transport *transport,
		int status,
		void *user_data);

	void (*on_connect_complete)(p2p_transport *transport,
		int connection_id,
		int status,
		void *transport_user_data,
		void *connect_user_data);

	void (*on_connection_disconnect)(p2p_transport *transport,
		int connection_id,
		void *transport_user_data,
		void *connect_user_data);

	void (*on_accept_remote_connection)(p2p_transport *transport,
		int connection_id,
		void *transport_user_data);

	void (*on_connection_recv)(p2p_transport *transport,
		int connection_id,
		void *transport_user_data,
		void *connect_user_data,
		char* data,
		int len);

	void (*on_tcp_proxy_connected)(p2p_transport *transport,
		unsigned short port, 
		char* addr);
}Fut_p2p_transport_cb;

typedef enum _Fut_p2p_trans_type_
{
    /* relay*/
    P2P_TRANS_TYPE_RELAY,
    /* p2p */
    P2P_TRANS_TYPE_P2P,
} Fut_p2p_trans_type;

typedef struct _Fut_p2p_transport_cfg_
{
	char* server;
	unsigned short port;
	unsigned char terminal_type;
	char* user; 
	char* password;
	void *user_data;
	int use_tcp_connect_srv;
	char* proxy_addr;
	Fut_p2p_transport_cb *cb;
	unsigned short relay_server_port;
}Fut_p2p_transport_cfg;


typedef enum _Fut_EUM_p2p_addr_type_
{
    /*host address*/
    P2P_ADDR_TYPE_HOST,

    /*local reflexive address  */
    P2P_ADDR_TYPE_SRFLX,

    /*peer reflexive address*/
    P2P_ADDR_TYPE_PRFLX,

    /*relayed address */
    P2P_ADDR_TYPE_RELAYED,

}Fut_EUM_p2p_addr_type;

typedef enum _Fut_EUM_p2p_opt_
{
	P2P_SNDBUF,
	P2P_RCVBUF,
}Fut_EUM_p2p_opt;

typedef struct _Fut_p2p_conn_info_
{
	int result;
	Fut_p2p_trans_type trans_type;
	long start_time;
	long end_time;
	char local_addr[FUT_INET6_ADDR_STRLEN+10];
	char remote_addr[FUT_INET6_ADDR_STRLEN+10];

	Fut_EUM_p2p_addr_type local_addr_type;
	Fut_EUM_p2p_addr_type remote_addr_type;
	char local_internet_addr[FUT_INET6_ADDR_STRLEN+10];
	char remote_internet_addr[FUT_INET6_ADDR_STRLEN+10];
}Fut_p2p_conn_info;

/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////

typedef void (*LOG_FUNC)(const char *data, int len);
FUT_P2P_DECL(int) Fut_p2p_init(LOG_FUNC log_func);

FUT_P2P_DECL(void) Fut_p2p_uninit();
FUT_P2P_DECL(void) Fut_set_log_level(int level);

FUT_P2P_DECL(int) Fut_create_p2p_transport(Fut_p2p_transport_cfg* cfg,p2p_transport **transport);
FUT_P2P_DECL(void) Fut_destroy_p2p_transport(p2p_transport *transport);

FUT_P2P_DECL(int) Fut_connect_p2p_transport(p2p_transport *transport,char* remote_user,void *user_data,int* connection_id);
FUT_P2P_DECL(int) Fut_connect_tcp_transport(p2p_transport *transport,char* remote_user,void* user_data,int* connection_id);

FUT_P2P_DECL(int) Fut_get_p2p_conn_remote_addr(p2p_transport *transport, int connection_id, char* addr, int* addr_len, Fut_EUM_p2p_addr_type* addr_type);
FUT_P2P_DECL(int) Fut_get_p2p_conn_local_addr(p2p_transport *transport, int connection_id, char* addr, int* addr_len, Fut_EUM_p2p_addr_type* addr_type);
FUT_P2P_DECL(int) Fut_set_p2p_conn_opt(p2p_transport *transport, int connection_id, Fut_EUM_p2p_opt opt, const void* optval, int optlen);

FUT_P2P_DECL(void) Fut_disconnect_p2p_transport(p2p_transport *transport,int connection_id);
FUT_P2P_DECL(int) Fut_send_p2p_transport(p2p_transport *transport,int connection_id,char* buffer,int len,int* error_code);
FUT_P2P_DECL(int) Fut_create_p2p_tcp_proxy(p2p_transport *transport,int connection_id,unsigned short remote_listen_port,unsigned short* local_proxy_port);
FUT_P2P_DECL(void) Fut_destroy_p2p_tcp_proxy(p2p_transport *transport,int connection_id,unsigned short local_proxy_port);
FUT_P2P_DECL(void) Fut_p2p_strerror(int error_code,char *buf,int bufsize);

typedef void (*ON_DETECT_NET_TYPE)(int status, int nat_type, void* user_data);
FUT_P2P_DECL(int) Fut_detect_p2p_nat_type(char* turn_server, unsigned short turn_port, ON_DETECT_NET_TYPE callback, void* user_data);
FUT_P2P_DECL(char*) Fut_get_p2p_ver();
FUT_P2P_DECL(int) Fut_get_p2p_proxy_remote_addr(p2p_transport *transport, unsigned short port, char* addr, int* add_len);

FUT_P2P_DECL(int) Fut_search_p2p_transport_server_net_state(p2p_transport *transport);
FUT_P2P_DECL(int) Fut_get_p2p_conn_info(p2p_transport *transport, int connection_id, Fut_p2p_conn_info *conn_info);
/////////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus /*extern "C"*/
}
#endif

#endif /* __P2P_TRANSPORT_H__ */
