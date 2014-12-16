/*
 * MsgServInfo.cpp
 *
 *  Created on: 2014-12-14
 *      Author: liguangyu
 */

#include "MsgServInfo.h"


//读取配置文件，到底多少个msgserver 需要压力测试，而且每个msgserver要多少个线程去压
msg_serv_info_t* read_msg_server_config(CConfigFileReader* config_file, const char* server_ip_format,
		const char* server_port_format,const char* server_num_format
		,const char* server_username_format,const char* server_password_format,  uint32_t& server_count)
{
	char server_ip_key[64];
	char server_port_key[64];
	char server_num_key[64];

	server_count = 0;

	// get server_count first;
	while (true) {
		sprintf(server_ip_key, "%s%d", server_ip_format, server_count + 1);
		sprintf(server_port_key, "%s%d", server_port_format, server_count + 1);
		sprintf(server_num_key,"%s%d",server_num_format, server_count + 1);
		
		char* server_ip_value = config_file->GetConfigName(server_ip_key);
		char* server_port_value = config_file->GetConfigName(server_port_key);
		char* server_num_value = config_file->GetConfigName(server_num_key);
		
		if (!server_ip_value || !server_port_value|| !server_num_value) {
			break;
		}

		server_count++;
	}

	if (server_count == 0) {
		return NULL;
	}

	msg_serv_info_t* server_list = new msg_serv_info_t [server_count];

	for (uint32_t i = 0; i < server_count; i++) {
		sprintf(server_ip_key, "%s%d", server_ip_format, i + 1);
		sprintf(server_port_key, "%s%d", server_port_format, i + 1);
		sprintf(server_num_key,"%s%d",server_num_format, i + 1);	
		char* server_ip_value = config_file->GetConfigName(server_ip_key);
		char* server_port_value = config_file->GetConfigName(server_port_key);
		char* server_num_value = config_file->GetConfigName(server_num_key);
		
		strcpy(server_list[i].server_ip,server_ip_value);
		server_list[i].server_port = atoi(server_port_value);
		server_list[i].num_connection = atoi(server_num_value);
		
		
		sprintf(server_num_key,"%s%d",server_username_format, i + 1);	//复用临时变量
		char* temp = config_file->GetConfigName(server_num_key);
		strcpy(server_list[i].szUserName,temp);
		
		sprintf(server_num_key,"%s%d",server_password_format, i + 1);//复用临时变量	
		temp = config_file->GetConfigName(server_num_key);
		strcpy(server_list[i].szPassword,temp);
		
		
		sprintf(server_num_key,"%s%d","MsgServerUserIDStart", i + 1);//复用临时变量	
		temp = config_file->GetConfigName(server_num_key);
		server_list[i].nUserIDStart = atoi(temp);
		
		
		sprintf(server_num_key,"%s%d","MsgServerReceiveResultID", i + 1);//复用临时变量	
		temp = config_file->GetConfigName(server_num_key);
		server_list[i].nReceiveResultID = atoi(temp);
		
		
		sprintf(server_num_key,"%s%d","MsgServerSendMsgInterval", i + 1);//复用临时变量	
		temp = config_file->GetConfigName(server_num_key);
		server_list[i].nSendMsgInterval = 1000*atoi(temp);
				
		//printf("server %s:%s  num connections:%s \n",server_ip_value,server_port_value,server_num_value);
	}

	return server_list;
}

