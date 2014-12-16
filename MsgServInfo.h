/*
 * MsgServInfo.h
 *
 *  Created on: 2014-12-14
 *      Author: liguangyu
 */

#ifndef MSG_SERVINFO_H_
#define MSG_SERVINFO_H_

#include "util.h"
#include "imconn.h"
#include "ConfigFileReader.h"


#define MAX_RECONNECT_CNT	64
#define MIN_RECONNECT_CNT	4

typedef struct msg_serv_info{
	msg_serv_info(){memset(this,0,sizeof(msg_serv_info));}
	char		server_ip[64];
	uint16_t	server_port;
	uint32_t	idle_cnt;
	uint32_t	reconnect_cnt;
	uint32_t  num_connection;//到底多少个线程压？
	char szUserName[64];
	char szPassword[64];
	uint32_t  nCurrentUserID;
	uint32_t  nUserIDStart;
	uint32_t  nReceiveResultID;
	uint32_t  nSendMsgInterval;
	//CImConn*	serv_conn;
	//
} msg_serv_info_t;

//extern static ConnMap_t g_msg_server_conn_map;

/*
g_msg_conn_map.insert(make_pair(handle, this));
*/
//template <class T>



template <class T>
void msg_serv_init(msg_serv_info_t* server_list, uint32_t server_count)
{
	for (uint32_t i = 0; i < server_count; i++) {
		server_list[i].idle_cnt = 0;
		server_list[i].reconnect_cnt = MIN_RECONNECT_CNT / 2;
			
		for(uint32_t nUser=0;nUser<server_list[i].num_connection;nUser++)
		{
			T::CreateClient(&server_list[i],nUser);
			/*T* pConn = new T();
			
			
			
			
			memcpy(&pConn->m_serv_info,&server_list[i],sizeof(msg_serv_info_t));
			sprintf(pConn->m_serv_info.szUserName,"%s%d",server_list[i].szUserName,nUser);
			strcpy(pConn->m_serv_info.szPassword,server_list[i].szPassword);
			
		
			pConn->Connect(server_list[i].server_ip.c_str(), server_list[i].server_port, i);
			*/
			

		}
	}
}

template <class T>
void msg_serv_check_reconnect(msg_serv_info_t* server_list, uint32_t server_count)
{
	T* pConn;
	/*
	for (uint32_t i = 0; i < server_count; i++) {
		pConn = (T*)server_list[i].serv_conn;
		if (!pConn) {
			server_list[i].idle_cnt++;
			if (server_list[i].idle_cnt >= server_list[i].reconnect_cnt) {
				pConn = new T();
				pConn->Connect(server_list[i].server_ip.c_str(), server_list[i].server_port, i);
				server_list[i].serv_conn = pConn;
			}
		}
	}
	*/
}

template <class T>
void msg_serv_reset(msg_serv_info_t* server_list, uint32_t server_count, uint32_t serv_idx)
{
	if (serv_idx >= server_count) {
		return;
	}
/*
	server_list[serv_idx].serv_conn = NULL;
	server_list[serv_idx].idle_cnt = 0;
	server_list[serv_idx].reconnect_cnt *= 2;
	if (server_list[serv_idx].reconnect_cnt > MAX_RECONNECT_CNT) {
		server_list[serv_idx].reconnect_cnt = MIN_RECONNECT_CNT;
	}
	*/
}

//读取配置文件，到底多少个msgserver 需要压力测试，而且每个msgserver要多少个线程去压

msg_serv_info_t* read_msg_server_config(CConfigFileReader* config_file, const char* server_ip_format,
		const char* server_port_format,const char* server_num_format
		,const char* server_username_format,const char* server_password_format,  uint32_t& server_count);


#endif /* MSG_SERVINFO_H_ */
