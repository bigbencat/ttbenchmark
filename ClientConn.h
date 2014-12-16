/*
 * ClientConn.h
 *
 *  Created on: 2014-12-14
 *      Author: liguangyu
 */

#ifndef CLIENTCONN_H_
#define CLIENTCONN_H_

#include "imconn.h"
#include "MsgServInfo.h"


class CClientConn : public CImConn
{
public:
	static void CreateClient(msg_serv_info_t* msg_serv,uint32_t nNumOfUser);
	
	msg_serv_info_t m_serv_info;
	
	CClientConn();
	virtual ~CClientConn();

	bool IsOpen() { return m_bOpen; }
	uint64_t GetConnectTime() { return m_connect_time; }

	void Connect(const char* server_ip, uint16_t server_port, uint32_t serv_idx);
	virtual void Close();

	virtual void OnConfirm();
	virtual void OnClose();
	virtual void OnTimer(uint64_t curr_tick);
	virtual void SendMessage(const char* szDestID,const char* szMsg);
	virtual void SendResultMsg();
	virtual void HandlePdu(CImPdu* pPdu);
	
	//是否由此client发送最终信息结果
	bool m_bSendResultClient;
private:
	void _HandleKickUser(CImPduKickUser* pPdu);
	void _HandleMessage(CImPduClientMsgData* pPdu);
	uint32_t _getSeqNo();
private:
	bool 		m_bOpen;
	uint32_t	m_serv_idx;
	uint64_t	m_connect_time;
	//最后发送消息
	uint64_t  m_last_send_msg_time;
	
	//最后发送消息
	uint64_t  m_last_send_result_time;
	//msg seq
	uint32_t  m_msg_seq;
	char m_szWhoAmI[64];
};

void init_msg_serv_conn(msg_serv_info_t* server_list, uint32_t server_count);




#endif /* CLIENTCONN_H_ */

