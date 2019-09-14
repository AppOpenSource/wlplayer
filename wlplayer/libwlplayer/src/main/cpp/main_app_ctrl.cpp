
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdarg.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <string>

#include "Fut_p2p_transport.h"
#include "Fut_App_P2P_Demo.h"
#include "AndroidLog.h"

///////////////////////////////////////////////////////////

int Fut_App_ConnectDvs(int m_p2p_proxy_port,int m_p2p_session_id)
{
	//This way to connect the p2p-module proxy-server.
	struct sockaddr_in servaddr;
	bzero(&servaddr, sizeof(servaddr));  
	servaddr.sin_family = AF_INET;  
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");  
	servaddr.sin_port = htons(m_p2p_proxy_port); 

	int app_client_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(connect(app_client_fd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
	{
		LOGD("connect the tcp proxy server error: %s(errno: %d)\n", strerror(errno), errno);

		close(app_client_fd);
		return -1;
	}

	LOGD("connect proxy-server: %d success, app client_fd= %d ...\n",m_p2p_proxy_port, app_client_fd);

#ifdef WIN32
	unsigned long l = 1;
	int n = ioctlsocket(app_client_fd, FIONBIO, &l);
#else
	int flags = fcntl(app_client_fd, F_GETFL, 0);
	fcntl(app_client_fd, F_SETFL, flags | O_NONBLOCK);
#endif

	fd_set rset, wset, allset;
	FD_ZERO(&allset);
	FD_SET(app_client_fd, &allset);

	char m_send_buf[128] = {0};
	int maxfd = app_client_fd;
	int m_send_count = 0;
	int nready = 0;
	int nPlaycount = 0;

	nready = system("rm ./tmpapp.h264");

	//Test the h264 file transmit.
	FILE* fp_app = fopen("tmpapp.h264", "ab+");
	if(NULL == fp_app)
	{
		LOGD("fopen tmpapp.h264 file failed \n");
		close(app_client_fd);
		return -1;
	}

	while(1)
	{
		rset = allset;
		wset = allset;

		nready = select(maxfd+1, &rset, &wset, NULL, NULL);
		if (FD_ISSET(app_client_fd, &wset))
		{
			m_send_count++;
			if (m_send_count >= 300)
			{
				m_send_count = 0;
				sprintf(m_send_buf,"App-Heart-Pkg");	//keep the link alived.
				int nwritten = write(app_client_fd, m_send_buf, strlen(m_send_buf));
				if (nwritten > 0)
				{
					//LOGD("app write buffer:%s nlen:%d nready:%d \n", m_send_buf,nwritten,nready);
				}
				else if ((nwritten == 0)
						|| ((nwritten < 0) && (errno != EINTR) && (errno != EAGAIN)&&(errno != EWOULDBLOCK))) 
				{
					LOGD("app write error:nwritten=%d %s(errno: %d)\n", nwritten, strerror(errno), errno);
					close(app_client_fd);
					return -1;
				}
			}
		}

		if (FD_ISSET(app_client_fd, &rset))
		{
			char recv_buf[10*1024] = {0};
			int nread = read(app_client_fd, recv_buf, sizeof(recv_buf));
			if (nread > 0)
			{
				//LOGD("app read buffer-len:%d \n", nread);
				//wite the local file.
				int n_have_write = 0;
				int n_writelen = 0;

				while (1)
				{
					n_have_write = fwrite(recv_buf+n_writelen, sizeof(char), nread-n_writelen, fp_app);
					if (n_writelen < 0)
					{
						LOGD("fwrite file error: 0009 \n");
						break;
					}

					n_writelen = n_writelen + n_have_write;
					if (n_writelen >= nread)
						break;

					usleep(1000);
				}

				//---- wirte file end --
				/*nPlaycount++;
				if (nPlaycount == 3)
				{
					nready = system("vlc ./tmpapp.h264 &");
					LOGD("vlc ./tmpapp.h264 &");
					nPlaycount = 4;
				}
				else if (nPlaycount > 3)
					nPlaycount = 4;*/

			}
			else if ((nread == 0)
					|| ((nread < 0) && (errno != EINTR) && (errno != EAGAIN)&&(errno != EWOULDBLOCK)))
			{
				LOGD("app read error: nread=%d %s(errno: %d) \n", nread, strerror(errno), errno);
				close(app_client_fd);
				fclose(fp_app);
				return -1;
			}
		}

		usleep(10000);
	}
}

//////////////////////////////////////////////////////////
int maina(int argc, char *argv[])
{
	LOGD("\t================================\n \
			JOOAN P2P APP-Part Demo Version=%s \n     \
			Completion time:%s %s \n     \
			================================", Fut_get_p2p_ver(),__DATE__,__TIME__);

	int c = 0;
	int n_ret = 0;
	char m_connect_dvs_uid[32] = {0};
	char m_dispatch_server_strs[128] = {0};

	pthread_t   mthreadId;

	//get the operation type.
	while((c = getopt(argc,argv,"d:c:")) != -1)
	{
		switch (c)
		{
			case 'c':
				{
					//This is app client port.
					memset(m_connect_dvs_uid, 0, sizeof(m_connect_dvs_uid));
					strncpy(m_connect_dvs_uid, optarg, sizeof(m_connect_dvs_uid)-1);

					break;
				}


			case 'd':
				{
					//This is connected server address.
					memset(m_dispatch_server_strs, 0 ,sizeof(m_dispatch_server_strs));
					strncpy(m_dispatch_server_strs, optarg, sizeof(m_dispatch_server_strs)-1);
					break;
				}

			default:
				LOGD("The argument is not valid ...\n");
				return 0;
		}
	}

	if (strlen(m_dispatch_server_strs) < 2)
		strcpy(m_dispatch_server_strs,"192.168.55.103:8989");

	LOGD("request the assign-server address:%s \n",m_dispatch_server_strs);

	//create the app-part-class.
	CP2PConnectProtocol app_operation_h(m_connect_dvs_uid,m_dispatch_server_strs);

	//that connect the remote-device by p2p-module.
	n_ret = app_operation_h.Fut_ConnectDvsByP2PProtocol(0);
	if (n_ret == 1)
	{
		//that connect the remote-device successful. start to transmit the av.file.
		n_ret = Fut_App_ConnectDvs(app_operation_h.m_proxy_svr_port,app_operation_h.m_conn_id);
	}
	else
	{
		LOGD("connect the remote-device failed !\n");
	}

	//waite for release all.
	sleep(12);

	return 0;
}


