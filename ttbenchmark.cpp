/*
 * ttbenchmark.cpp
 *
 *  Created on: 2014-12-14
 *      Author: liguangyu
 */

#include "netlib.h"
#include "ConfigFileReader.h"
#include "version.h"
#include "MsgServInfo.h"
#include "ClientConn.h"




int main(int argc, char* argv[])
{
	if ((argc == 2) && (strcmp(argv[1], "-v") == 0)) {
		printf("client Version: ttbenchmark/%s\n", VERSION);
		printf("client Build: %s %s\n", __DATE__, __TIME__);
		printf("usage \"ttbenchmark -sql >user.sql\" to create user.sql\nmysql -u$USER -p$PASSWORD < user.sql\n");
		
		return 0;
	}

	CConfigFileReader config_file("ttbenchmark.conf");
	uint32_t 			msg_server_count = 0;
	msg_serv_info_t* 	msg_server_list = read_msg_server_config(&config_file, "MsgServerIP", "MsgServerPort",
				"MsgServerConnNum","MsgServerUserName","MsgServerPwd",msg_server_count);

	log("ttbenchmark.conf has : %d  msg servers\n", msg_server_count);
	
	if(msg_server_count<=0)
		return 0;
	
	if ((argc == 2) && (strcmp(argv[1], "-sql") == 0)) {
		//INSERT INTO `IMUsers` VALUES ('10004', 'mykeyonline', 'wenyuangui', 'e10adc3949ba59abbe56e057f20f883e', '0', 'g0/000/000/1417614097916119_140360801912.jpg', 'ÎÄÔ¶¹ï', '1010', '1', 'ÉîÛÚ', '13911111111', 'test@mail.com', '1', '1', '1409816664');	
		printf("use macim;\n");
		for(int i=0;i<msg_server_list[0].num_connection;i++)
		{
				printf("INSERT INTO `IMUsers` VALUES ('%d', 'mykeyonline', '%s%d', '%s', '0', 'g0/000/000/1417614097916119_140360801912.jpg',\
				 'name%s%d', '1010', '1', 'ÉîÛÚ', '13911111111', 'test@mail.com', '1', '1', '1409816664');	\n",
								i+msg_server_list[0].nUserIDStart,msg_server_list[0].szUserName,i,msg_server_list[0].szPassword,msg_server_list[0].szUserName,i);
		
		}
		return 0;
	}
	signal(SIGPIPE, SIG_IGN);
	srand(time(NULL));

	log("ttbenchmark max files can open: %d\n", getdtablesize());
	
	
	


	int ret = netlib_init();

	if (ret == NETLIB_ERROR)
	{
		printf("netlib_init error!");
		return ret;
	}
	
	init_msg_serv_conn(msg_server_list, msg_server_count);

	printf("now enter the event loop...\n");

	netlib_eventloop();

	return 0;
}
