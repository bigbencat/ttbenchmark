/*
 * ClientConn.cpp
 *
 *  Created on: 2014-12-14
 *      Author: liguangyu
 */

#include "ClientConn.h"


static ConnMap_t g_msg_server_conn_map;
//static ConnMap_t g_msg_conn_map;

static msg_serv_info_t* g_msg_server_list;

static uint32_t g_msg_server_count;

void msg_server_conn_timer_callback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam)
{
	
	//printf("timer......................\n");
	ConnMap_t::iterator it_old;
	CClientConn* pConn = NULL;
	uint64_t cur_time = get_tick_count();

	for (ConnMap_t::iterator it = g_msg_server_conn_map.begin(); it != g_msg_server_conn_map.end(); ) {
        it_old = it;
        it++;
        pConn = (CClientConn*)it_old->second;
        pConn->OnTimer(cur_time);
	}

	// reconnect RouteServer
	msg_serv_check_reconnect<CClientConn>(g_msg_server_list, g_msg_server_count);
}

void init_msg_serv_conn(msg_serv_info_t* server_list, uint32_t server_count)
{
	g_msg_server_list = server_list;
	g_msg_server_count = server_count;

	msg_serv_init<CClientConn>(g_msg_server_list, g_msg_server_count);

	netlib_register_timer(msg_server_conn_timer_callback, NULL, 1000);

}

static uint32_t g_msg_sended_count=0;
static uint32_t g_msg_received_count=0;
static uint64_t g_msg_server_start_time=0;


CClientConn::CClientConn()
{
	m_bOpen = false;
	m_bSendResultClient = false;
	m_last_send_result_time =0;
	m_serv_idx = 0;
	m_msg_seq = rand() % 10000;
	m_szWhoAmI[0]=0x0;
	if(g_msg_server_start_time ==0)
		g_msg_server_start_time =get_tick_count();
}

CClientConn::~CClientConn()
{

}

//建立一个连接，把初始化参数输入！！！
void CClientConn::CreateClient(msg_serv_info_t* msg_serv,uint32_t nNumOfUser)
{
		CClientConn* pConn = new CClientConn();
		if(nNumOfUser == 0)
		{//第一个用户给手机用户发送测试结果
			pConn->m_bSendResultClient=true;
		}
		memcpy(&pConn->m_serv_info,msg_serv,sizeof(msg_serv_info_t));
		pConn->m_serv_info.nCurrentUserID=nNumOfUser;
		pConn->Connect(msg_serv->server_ip, msg_serv->server_port, nNumOfUser);
	
}


void CClientConn::Connect(const char* server_ip, uint16_t server_port, uint32_t idx)
{
	log("Connecting to MsgServer %s:%d\n", server_ip, server_port);
	
	//printf("Connecting to MsgServer %s:%d\n", server_ip, server_port);

	m_serv_idx = idx;
	m_handle = netlib_connect(server_ip, server_port, imconn_callback, (void*)&g_msg_server_conn_map);

	if (m_handle != NETLIB_INVALID_HANDLE) {
		g_msg_server_conn_map.insert(make_pair(m_handle, this));
	}
}

void CClientConn::Close()
{
	//这个没啥用。
	//msg_serv_reset<CClientConn>(g_msg_server_list, g_msg_server_count, m_serv_idx);


	//断线了就重新连接
	CreateClient(&m_serv_info,m_serv_info.nCurrentUserID);
	
	m_bOpen = false;
	if (m_handle != NETLIB_INVALID_HANDLE) {
		netlib_close(m_handle);
		g_msg_server_conn_map.erase(m_handle);
	}
	
	//printf("%d release Ref!\n",this);
	ReleaseRef();

}

void CClientConn::OnConfirm()
{
	log("connect to route server success\n");
	m_bOpen = true;
	m_connect_time = get_tick_count();
	m_last_send_msg_time = m_connect_time;
	m_last_send_result_time = m_connect_time;
	m_serv_info.reconnect_cnt = MIN_RECONNECT_CNT / 2;


//	list<user_conn_stat_t> online_user_list;
//    CImUserManager::GetInstance()->GetOnlineUserInfo(&online_user_list);
//	CImPduOnlineUserInfo pdu(&online_user_list);
//	SendPdu(&pdu);

	
	//string name,pass;
	//name="jiaolei";
	//pass="e10adc3949ba59abbe56e057f20f883e";
	//CImPduLoginRequest pdu(name.c_str(), pass.c_str(), USER_STATUS_ONLINE, CLIENT_TYPE_ANDROID);
	//char szUser[64];
	sprintf(m_szWhoAmI,"%s%d",m_serv_info.szUserName,m_serv_info.nCurrentUserID);
	
	//printf("connect msg server ok!,handle=%d .send login %s   %s  request\n",m_handle,m_szWhoAmI,m_serv_info.szPassword);
	
	
	CImPduLoginRequest pdu(m_szWhoAmI,m_serv_info.szPassword, USER_STATUS_ONLINE, CLIENT_TYPE_ANDROID);
	SendPdu(&pdu);
}

void CClientConn::OnClose()
{
	printf("onclose from route server handle=%d\n", m_handle);
	log("onclose from route server handle=%d\n", m_handle);
	Close();
}

uint32_t CClientConn::_getSeqNo()
{//uint32_t  m_msg_seq;
	return ++m_msg_seq;
}

void CClientConn::SendMessage(const char* szDestID,const char* szMsg)
{
	/*
	_SendSessionMsg(mixMsg);
		//本地消息展现
		msg.msgType = MSG_TYPE_TEXT_P2P;
		msg.talkerSid = module::getSysConfigModule()->userID();
		msg.sessionId = m_sId;
		msg.msgRenderType = MESSAGE_RENDERTYPE_TEXT;
		msg.msgStatusType = MESSAGE_TYPE_RUNTIME;
		msg.content = util::cStringToString(mixMsg.MakeMixedLocalMSG());
		msg.msgTime = module::getSessionModule()->getTime();
		_DisplayMsgToIE(msg, _T("sendMessage"));
		
		
		
		sendingMsg.sendtime = static_cast<long>(clock());
		sendingMsg.msg = msg;
		sendingMsg.seqNo = _getSeqNo();
		sendingMsg.status = MSGSTATUS_SENDING;
		m_ListSendingMsg.push_back(sendingMsg);
		sendMessage(sendingMsg);
		*/
		
		
		char szSrcID[64];
		sprintf(szSrcID,"%d",m_serv_info.nUserIDStart+m_serv_info.nCurrentUserID);
		
		//printf("send msg:%s(%s) --> %s  :%s\n",m_szWhoAmI,szSrcID,szDestID,szMsg);
		CImPduClientMsgData pduMsgData(_getSeqNo()
			, szSrcID
			, szDestID													//要用原始初始ID
			, get_tick_count()      //创建时间 目前由服务器提供
			, MSG_TYPE_P2P_TEXT           		//消息来源类型
			, strlen(szMsg), (uchar_t*)szMsg
			, 0,"");
		SendPdu(&pduMsgData);
		g_msg_sended_count++;

}
//发送benchmark 结果
void CClientConn::SendResultMsg()
{
	/*
	g_msg_server_conn_map
	static uint32_t g_msg_sended_count=0;
	static uint32_t g_msg_received_count=0;
		g_msg_server_start_time =get_tick_count();
	*/
	
	uint32_t u_msg_open_count=0;
	ConnMap_t::iterator it_old;
	CClientConn* pConn = NULL;
	uint64_t cur_time = get_tick_count();



	for (ConnMap_t::iterator it = g_msg_server_conn_map.begin(); it != g_msg_server_conn_map.end(); ) {
        it_old = it;
        it++;
        pConn = (CClientConn*)it_old->second;
        if(pConn->m_bOpen)
        	u_msg_open_count++;
        
	}
	
	
	double dStartTime=(cur_time-g_msg_server_start_time)/1000.0f;
	char szMsg[1024];
	sprintf(szMsg,"start time\t:%.2f min \nuser num\t:%d\nlogin num\t:%d\nsend messages\t:%d\nreceive messages\t:%d\nmessage success\t:%.2f%%\nprocess msg\t:%.2f message/s",
		dStartTime/60,
		g_msg_server_conn_map.size(),
		u_msg_open_count,
		g_msg_sended_count,
		g_msg_received_count,
		g_msg_received_count*100.f/g_msg_sended_count,
		g_msg_sended_count/dStartTime);
		
	char szDest[64];
	sprintf(szDest,"%d",m_serv_info.nReceiveResultID);
	SendMessage(szDest,szMsg);
	g_msg_sended_count--;//结果信息msg就不计入统计啦！！！
	printf("%s\n------------------------------------------------------------\n",szMsg);
		
}

void CClientConn::OnTimer(uint64_t curr_tick)
{
	if (curr_tick > m_last_send_tick + SERVER_HEARTBEAT_INTERVAL) {
		CImPduHeartbeat pdu;
		SendPdu(&pdu);
		m_last_send_tick = curr_tick;//原来程序没有？？？ by liguangyu 
	}

	if (curr_tick > m_last_recv_tick + SERVER_TIMEOUT) {
		log("conn to route server timeout\n");
		Close();
	}
	
	if((m_bSendResultClient) && (curr_tick > m_last_send_result_time + 30000)) {
		m_last_send_result_time = curr_tick;
		SendResultMsg();//发送benchmark 结果
	}
	
	
	
	//#define SEND_MSG_TIME 10000  //10s   nSendMsgInterval
	if (curr_tick > m_last_send_msg_time + m_serv_info.nSendMsgInterval) {
		if(( rand() % 100)>90){//到时间后 1/10的概率要发送消息。每秒测试一次
			uint32_t uDestUser = rand() % m_serv_info.num_connection;
			if(uDestUser != m_serv_info.nCurrentUserID)//不给我自己发啦
			{
				m_last_send_msg_time = curr_tick;
				char szDestID[64];
				sprintf(szDestID,"%d",m_serv_info.nUserIDStart+uDestUser);
				SendMessage(szDestID,"haha 1234567890");
				
			}
		}
	}
	
}


void CClientConn::_HandleKickUser(CImPduKickUser* pPdu)
{
	uint32_t to_user_id = pPdu->GetUserId();
  uint32_t reason_type = pPdu->GetReason();
	log("HandleKickUser, user_id=%u\n", to_user_id);
	//printf("%s\t:  handle=%d  be kicked ,close()\n",m_szWhoAmI,m_handle);
	Close();

}

void CClientConn::_HandleMessage(CImPduClientMsgData* pPdu)
{
	char szSrc[100];memset(szSrc,0,100);
	char szDst[100];memset(szDst,0,100);
	memcpy(szSrc,pPdu->GetFromIdUrl(),pPdu->GetFromIdLen());
	memcpy(szDst,pPdu->GetToIdUrl(),pPdu->GetToIdLen());
	
	if (CHECK_MSG_TYPE_GROUP(pPdu->GetMsgType()))
	{
		//printf("%s\treceive group msg %s --> %s : %s \n",m_szWhoAmI,szSrc,szDst,pPdu->GetMsgData());	
	}
	else
	{//只有个人消息要发已读确认,群消息是不发已读确认的 ??
			//printf("%s\treceive client msg %s --> %s : %s \n",m_szWhoAmI,szSrc,szDst,pPdu->GetMsgData());	
	
			//通知服务器收到该消息
			CImPduClientMsgDataAck pduMsgData(pPdu->GetSeqNo(),pPdu->GetFromIdLen(),szSrc/*pPdu->GetFromIdUrl()*/);
			SendPdu(&pduMsgData);
			//已读消息确认
			CImPduClientMsgReadAck pudRedAck(szSrc);
			SendPdu(&pudRedAck);
		}
		g_msg_received_count++;
}

void CClientConn::HandlePdu(CImPdu* pPdu)
{
	static unsigned int nNum=0;
	
	nNum++;
	//if(pPdu->GetPduType() !=IM_PDU_TYPE_HEARTBEAT) printf("%s\t%u\t:  handle=%d  :receive pdu type %d\n",m_szWhoAmI,nNum,m_handle,pPdu->GetPduType());
	switch (pPdu->GetPduType()) {
	case IM_PDU_TYPE_HEARTBEAT:
		break;
	case IM_PDU_TYPE_CLIENT_KICK_USER://IM_PDU_TYPE_SERVER_KICK_USER:
		_HandleKickUser( (CImPduKickUser*)pPdu );
		break;
	case IM_PDU_TYPE_CLIENT_MSG_DATA://receive message!!!!!
		_HandleMessage( (CImPduClientMsgData*)pPdu );
		
		break;
	case IM_PDU_TYPE_FRIEND_STATUS_LIST:
		//_HandleFriendStatusList( (CImPduFriendStatusList*)pPdu );
		break;
	case IM_PDU_TYPE_FRIEND_STATUS_NOTIFY:
		//_HandleFriendStatusNotify( (CImPduFriendStatusNotify*)pPdu );
		break;
	case IM_PDU_TYPE_MSG_DATA:
	{
		//CImPduMsgData* pPduMsg = (CImPduMsgData*)pPdu;
		//if (CHECK_MSG_TYPE_GROUP(pPduMsg->GetMsgType()))
    //        s_group_chat->HandleGroupMessage(pPduMsg);
		//else
    //        _HandleMsgData(pPduMsg);
		break;
	}
	case IM_PDU_TYPE_P2P_MSG:
		//_HandleP2PMsg( (CImPduP2PMsg*)pPdu );
		break;
    case IM_PDU_TYPE_GROUP_P2P_MSG_RESPONSE:
        //_HandleGroupP2PMsg((CImPduGroupP2PMessageResponse*)pPdu);
        break;
	case IM_PDU_TYPE_USER_STATUS_RESPONSE:
		//_HandleUserStatusResponse( (CImPduUserStatusResponse*)pPdu );
		break;
	case IM_PDU_TYPE_USERS_STATUS_RESPONSE:
		//_HandleUsersStatusResponse( (CImPduUsersStatusResponse*)pPdu );
		break;
    case IM_PDU_TYPE_USER_CLIENT_TYPE_RESPONSE:
       // _HandleUserClientTypeResponse((CImPduUserClientTypeResponse*) pPdu);
        break;
    case IM_PDU_TYPE_FILE_NOTIFY:
        //s_file_handler->HandleFileNotify((CImPduFileNotify *)pPdu);
        break;
    /*
	case IM_PDU_TYPE_FILE_REQUEST:
		s_file_handler->HandleFileRequest( (CImPduFileRequest*)pPdu );
		break;
	case IM_PDU_TYPE_FILE_RESPONSE:
		s_file_handler->HandleFileResponse( (CImPduFileResponse*)pPdu );
		break;
	case IM_PDU_TYPE_FILE_RECV_READY:
		s_file_handler->HandleFileRecvReady( (CImPduFileRecvReady*)pPdu );
		break;
	case IM_PDU_TYPE_FILE_ABORT:
		s_file_handler->HandleFileAbort( (CImPduFileAbort*)pPdu );
		break;
	case IM_PDU_TYPE_FILE_UPLOAD_OFFLINE_NOTIFY:
		s_file_handler->HandleFileUploadOfflineNotify( (CImPduFileUploadOfflineNotify*)pPdu );
		break;
	case IM_PDU_TYPE_FILE_DOWNLOAD_OFFLINE_NOTIFY:
		s_file_handler->HandleFileDownloadOfflineNotify( (CImPduFileDownloadOfflineNotify*)pPdu );
		break;
    */
	case IM_PDU_TYPE_GROUP_CREATE_TMP_GROUP_RESPONSE:
		//s_group_chat->HandleGroupCreateTmpGroupBroadcast((CImPduGroupCreateTmpGroupResponse*)pPdu);
		break;
	case IM_PDU_TYPE_GROUP_CHANGE_MEMBER_RESPONSE:
		//s_group_chat->HandleGroupChangeMemberBroadcast((CImPduGroupChangeMemberResponse*)pPdu);
		break;
    case IM_PDU_TYPE_GROUP_CREATE_NORMAL_GROUP_NOTIFY:
       // s_group_chat->HandleGroupCreateNormalGroupNotify((CImPduGroupCreateNormalGroupNotify*)pPdu);
        break;
    case IM_PDU_TYPE_GROUP_CHANGE_MEMBER_NOTIFY:
        //s_group_chat->HandleGroupChangeMemberNotify((CImPduGroupChangeMemberNotify*)pPdu);
        break;
	default:
		log("unknown pdu_type=%d\n", pPdu->GetPduType());
		break;
	}
}