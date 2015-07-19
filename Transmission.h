#pragma once

#include "socketinterface.h"
#include "HPSocket.h"
#include "ServerListener.h"

class CTransmission {
  public:
    CTransmission();
    ~CTransmission();

    CTcpServerPtr * Server() const {
        return m_pServer;
    }

    void Server(CTcpServerPtr * val) {
        m_pServer = val;
    }

    CServerListener * Listener() const {
        return m_pListener;
    }

    void Listener(CServerListener * val) {
        m_pListener = val;
    }

    bool IsConnected() const {
        return isConnected;
    }

    BOOL OpenConnection();
    void CloseConnection();

  private:

    CTcpServerPtr *m_pServer;
    CServerListener *m_pListener;
    bool isConnected;

};

