#pragma once
#include <memory>

#include "socketinterface.h"
#include "HPSocket.h"

#include "Types.h"
#include <boost/thread.hpp>
#include "Semaphore.hpp"

#define USER_INFO_MSG           (WM_USER + 100)
#define MAX_LOG_RECORD_LENGTH   1000

#define EVT_ON_SEND             "OnSend"
#define EVT_ON_RECEIVE          "OnReceive"
#define EVT_ON_CLOSE            "OnClose"
#define EVT_ON_ERROR            "OnError"
#define EVT_ON_PREPARE_CONNECT  "OnPrepareConnect"
#define EVT_ON_PREPARE_LISTEN   "OnPrepareListen"
#define EVT_ON_ACCEPT           "OnAccept"
#define EVT_ON_CONNECT          "OnConnect"
#define EVT_ON_SHUTDOWN         "OnShutdown"
#define EVT_ON_DONE             "DONE"
#define EVT_ON_END_TEST         "END TEST"

enum ReceiveState {
    WAITING,
    START,
    WAIT_HEADER,
    WAIT_BODY,
    FINISHING,
    PROCESSING,
    HEADER_ERROR
};


extern ImageMeasure::Semaphore QueueEmpty;
extern char* g_buff[2];
extern int g_stopThread;
extern int g_restartFlag;
extern int g_finish;
extern uint64_t buffPosition;
extern uint64_t lastestPosition;
extern const int totalSize;
extern const uint64_t FINISH_TYPE;

extern boost::mutex buffPositionLock;
extern boost::mutex lastPositionLock;
extern boost::mutex buffLock;

extern CWnd* g_pMainWnd;
extern boost::thread g_CheckBuffThread;
extern ReceiveState g_threadState;

struct info_msg {
    CONNID connID;
    char* evt;
    int contentLength;
    char* content;

    static info_msg* Construct ( CONNID dwConnID, char* lpszEvent, int iContentLength, char* lpszContent );
    static void Destruct ( info_msg* pMsg );

  private:
    info_msg ( CONNID dwConnID, char* lpszEvent, int iContentLength, char* lpszContent );
    ~info_msg();
};

class CServerListener :
    public ITcpServerListener {
  public:
    CServerListener ( CWnd* pMainWnd = nullptr );
    ~CServerListener ( void );

    virtual EnHandleResult OnAccept ( CONNID dwConnID, SOCKET soClient );

    virtual EnHandleResult OnPrepareListen ( SOCKET soListen );

    virtual EnHandleResult OnSend ( CONNID dwConnID, const BYTE* pData, int iLength );

    virtual EnHandleResult OnReceive ( CONNID dwConnID, const BYTE* pData, int iLength );

    virtual EnHandleResult OnReceive ( CONNID dwConnID, int iLength );

    virtual EnHandleResult OnClose ( CONNID dwConnID );

    virtual EnHandleResult OnError ( CONNID dwConnID, EnSocketOperation enOperation, int iErrorCode );

    virtual EnHandleResult OnShutdown();


    enum RECEIVE_STATE {
        WAIT_FOR_HRAD,
        RECEIVING,
        FINISHING
    };


    enum TRANSMISSION_STATE {
        WAITING,
        RECEIVING_OVERLOOK_HEADER,
        RECEIVING_OVERLOOK_HEADER_FINISHED,
        RECEIVING_OVERLOOK_IMAGE,
        RECEIVING_OVERLOOK_IMAGE_FINISED,
        RECEIVING_SIDELOOK_HEADER,
        RECEIVING_SIDELOOK_HEADER_FINISHED,
        RECEIVING_SIDELOOK_IMAGE,
        RECEIVING_SIDELOOK_IMAGE_FINISED,
        RECEIVING_PANO_IMAGE,
        RECEIVING_PANO_IMAGE_FINISED,
        TRANSMISSION_FINISHED
    };

  protected:
    void CServerListener::PostInfoMsg ( info_msg* msg );
  public:
    char* m_buff;
  protected:
    CWnd*        m_pMainWnd;
    int m_receivedByte;
    int m_totalByte;
    TRANSMISSION_STATE m_transmissionState;
    RECEIVE_STATE m_receiveState;
    ImageMeasure::TransmissionHeader m_receivedHeader;
    ImageMeasure::ImageHolder m_imageHolder;
    std::wstring m_currentId;

};

