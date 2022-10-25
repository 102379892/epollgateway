#ifndef __H__ERRORCODE__H__
#define __H__ERRORCODE__H__

//成功（0）
const int Comm_Success = 0;

//DAP业务码（100 ~ 110）
const int Dap_ComputeID_Code = 100;		//computeid

//公共错误码（-100 ~ -199）
const int Comm_Exception_Err = -198;	//其它异常
const int Comm_Other_Err = -199;		//其它错误

const int Comm_New_Err = -100;			//分配内存失败
const int Comm_ParseParam_Err = -101;	//解析配置失败
const int Comm_NoParam_Err = -102;		//无此配置信息
const int Comm_InvalidValue_Err = -103;	//值不合法
const int Comm_InvalidParam_Err = -104;	//参数不合法
const int Comm_OpenFile_Err = -105;		//文件打开错误
const int Comm_ReadFile_Err = -106;		//文件读取错误
const int Comm_Connect_Err = -107;		//连接错误

const int Db_BeginTran_Err = -151;		//数据库开始事务失败
const int Db_ComitTran_Err = -152;		//数据库提交事务失败		
const int Db_RollBTran_Err = -153;		//数据库回滚事务失败
const int Db_Login_Err = -154;			//数据库登录失败
const int Db_ExecSql_Err = -155;		//数据库执行SQl失败
const int Db_NoData = -156;				//数据库无此数据
const int Db_Conn_Err = -157;			//连接数据库失败

const int Ice_Exception_Err = -160;		//Ice异常

const int Json_Prase_Err = -170;		//解析json失败
const int Json_ReDeal_Err = -171;		//待重试
const int Comm_NoData_Err = -172;		//无数据

//DAP、CAP错误码（-201 ~ -300）
const int Dap_SqlBufIsTooBig_Err = -201;	//SQL太长
const int Dap_Init_Err = -202;				//DB初始化失败
const int Dap_NoMoreResult_Err = -203;		//DB执行无结果
const int Dap_GetResult_Err = -204;			//DB获取结果失败
const int Dap_NoNumCol_Err = -205;			//表中无数据列
const int Dap_BindCol_Err = -206;			//绑定数据列失败
const int Dap_GetNextRow_Err = -207;		//获取下一行数据失败
const int Dap_ConnNumOver_Err = -208;		//连接池里的连接用完

const int Cap_ConnNumOver_Err = -220;		//连接池里的连接用完
const int Cap_ExecCmd_Err = -221;			//执行redis命令失败
const int Cap_ResultTypeInvaild = -222;		//返回结果类型不合法
const int Cap_NoData = -223;				//redis不存在该数据
const int Cap_InvaildCursor = -224;			//不合法的索引值
const int Cap_InvaildAccuracyType = -225;	//不合法的定位类型
const int Cap_ParseDataFailed = -226;		//解析数据失败
const int Cap_InvaildParam = -227;			//参数不合法
const int Over_FlowControl_Err = -228;      //超过流量控制

//MQ、HCC错误码（-301 ~ -350）
const int Mq_Exception_Err = -301;			//MQ异常
const int Mq_NoExchange_Err = -302;			//MQ发布者未创建Exchange
const int Mq_NoQueue_Err = -303;			//MQ接收者未创建Queue
const int Mq_NoData = -304;					//MQ无数据

const int Hcc_NoLoadBaseData_Err = -304;	//基础数据未加载
const int Hcc_NoData = -305;				//无此记录

const int KafkaMq_Exception_Err = -306;			//异常
const int KafkaMq_NoData = -307;				//无数据
const int KafkaMq_Invaild_Partition = -308;		//不合法的分区
const int KafkaMq_InvaildParam = -309;			//参数不合法
const int KafkaMq_ParseDataFailed = -310;		//解析数据失败
const int KafkaMq_SeriDataFailed = -311;		//打包数据失败
const int KafkaMq_SendFailed = -312;			//发送数据失败
const int KafkaMq_RecvFailed = -313;			//接收数据失败
const int KafkaMq_CONNECT = -314;				//启动连接kafka失败
const int KafkaMq_InvaildMode = -315;			//使用kafka模式不正确
const int KafkaMq_HISDATA = -316;				//重复数据
const int KafkaMq_OFFSET_Err = -317;			//偏移量错误
const int KafkaMq_Invaild_Data_Err = -318;		//不合法数据

const int Zoo_ExecSet = -401;				//执行set失败
const int Zoo_NoNode = -402;				//无可用的节点获取（或者所有节点都被使用）
const int Zoo_NoExistNode = -403;			//该节点不存在
const int Zoo_CreatePath = 404;				//创建路径失败

const int IAS_CREATEEPOLL_ERR = -500;
const int IAS_CREATESOCKET_ERR = -501;
const int IAS_SOCKETBIND_ERR = -502;
const int IAS_SOCKETLISTEN_ERR = -503;
const int IAS_SOCKETACCEPT_ERR = -504;
const int IAS_EPOLLADD_ERR = -505;
const int IAS_EPOLLDEL_ERR = -506;
const int IAS_SENDDATA_ERR = -507;
const int IAS_RECVDATA_ERR = -508;
const int IAS_SAMEIMEI_ERR = -509;
const int IAS_NOEXIST_GASE = -510;

#endif
