
#if !defined(__TTRL_MAIN_CMD_CODE_HEADER_20091105__)
#define __TTRL_MAIN_CMD_CODE_HEADER_20091105__

///////////////////////////////////////////////
//TTR serise main  Q command code header file
//////////////////////////////////////////////	

#define	TTR_MAIN_CC_REGISTER			100		//start service request
#define	TTR_MAIN_CC_UNREGISTER			101		//stop service request

#define	TTR_MAIN_CC_GET_CONTEXT			200		//Get context( may be open a device. )
#define	TTR_MAIN_CC_RELEASE_CONTEXT		201		//Release context( may be close device. )

#define	TTR_THREAD_CC_SYNC_TX_RX		500		//sync method transmit( tx & rx )
#define	TTR_THREAD_CC_SYNC_TX			501		//sync method send ( tx )
#define	TTR_THREAD_CC_SYNC_RX			502		//sync method receive( rx )

#define	TTR_THREAD_CC_ASYNC_TX_RX		600		//async method transmit( tx & rx )
#define	TTR_THREAD_CC_ASYNC_TX			601		//async method send( tx )
#define	TTR_THREAD_CC_ASYNC_RX			602		//async method receive( rx )


/////////////////////////////////////////

#endif	//__TTRL_MAIN_CMD_CODE_HEADER_20091105__