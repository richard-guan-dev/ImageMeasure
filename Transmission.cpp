#include "stdafx.h"
#include "Transmission.h"

CTransmission::CTransmission() : m_pListener(nullptr), m_pServer(nullptr), isConnected(false) {
}

CTransmission::~CTransmission() {
    CloseConnection();
}


BOOL CTransmission::OpenConnection() {
    if (this->isConnected == true && m_pServer != nullptr) {
        return TRUE;
    }

    if (this->m_pListener == nullptr) {
        return FALSE;
    }

    if (m_pServer != nullptr) {
        CloseConnection();
    }

    this->m_pServer = new CTcpServerPtr(this->m_pListener);

    if ((*m_pServer) == nullptr) {
        return FALSE;
    }

    ((this->m_pServer)->Get())->SetWorkerThreadCount(1);
    BOOL state = (*m_pServer)->Start(L"0.0.0.0", 55999);

    if (state == FALSE) {
        EnSocketError error = (*m_pServer)->GetLastError();
        CloseConnection();

    } else {
        this->isConnected = true;
    }

    return  state;
}

void CTransmission::CloseConnection() {
    if (this->m_pServer != nullptr) {
        if (((this->m_pServer)->Get()) != nullptr) {
            ((this->m_pServer)->Get())->Stop();
        }

        delete this->m_pServer;
        m_pServer = nullptr;
        this->isConnected = false;
    }

    if (this->m_pListener != nullptr) {
        delete this->m_pListener;
        this->m_pListener = nullptr;
    }
}
