// PanoDialog.cpp : implementation file
//

#include "stdafx.h"
#include "ImageMeasure.h"
#include "PanoDialog.h"
#include "afxdialogex.h"



// CPanoDialog dialog

IMPLEMENT_DYNAMIC ( CPanoDialog, CDialogEx )

CPanoDialog::CPanoDialog ( CWnd* pParent /*=NULL*/ )
    : CDialogEx ( CPanoDialog::IDD, pParent ) {
	m_currentPanoId = 200;
}

CPanoDialog::~CPanoDialog() {
}

void CPanoDialog::DoDataExchange ( CDataExchange* pDX ) {
    CDialogEx::DoDataExchange ( pDX );
}


BEGIN_MESSAGE_MAP ( CPanoDialog, CDialogEx )
    ON_WM_PAINT()
    ON_WM_KEYDOWN()
    ON_BN_CLICKED ( IDC_PANO_BEFORE_BUTTON, &CPanoDialog::OnBnClickedPanoBeforeButton )
    ON_BN_CLICKED ( IDC_PANO_NEXT_BUTTON, &CPanoDialog::OnBnClickedPanoNextButton )
END_MESSAGE_MAP()


BOOL CPanoDialog::PreTranslateMessage ( MSG* pMsg ) {

    if ( pMsg->message == WM_KEYDOWN ) {
        if ( pMsg->wParam == VK_RIGHT ) {
            OnBnClickedPanoNextButton();
        } else if ( pMsg->wParam == VK_LEFT ) {
            OnBnClickedPanoBeforeButton();
        }
    }
    return CDialogEx::PreTranslateMessage ( pMsg );
}

void CPanoDialog::OnKeyDown ( UINT nChar, UINT nRepCnt, UINT nFlags ) {
    switch ( nChar ) {
        case VK_RIGHT:
            OnBnClickedPanoNextButton();
            break;
        case VK_LEFT:
            OnBnClickedPanoBeforeButton();
            break;
    }
}

BOOL CPanoDialog::OnInitDialog() {
    CDialogEx::OnInitDialog();

    std::map<int, ImageMeasure::Product> productDict;
    this->m_dataStore->GetAllProduct ( productDict );

    m_currentProduct = productDict[this->m_currentProductId];
//	m_currentPanoId = 100;
	int id = this->m_currentPanoId % m_currentProduct.panoPath.size();
    LoadMeasureImage(m_currentProduct.panoPath[id].c_str());
    return TRUE;
}

void CPanoDialog::LoadMeasureImage ( const TCHAR* imagePath ) {
    if (_waccess(imagePath, 0) == -1) {
//       AfxMessageBox(L"Í¼Ïñ¶ªÊ§£¡", MB_OK | MB_SYSTEMMODAL);
        return;
    }

    CImage myImage;
    myImage.Load ( imagePath );

    CRect rect;
    CWnd* pWnd = GetDlgItem ( IDC_PANO_PICTURE );
    CDC* pDC = pWnd->GetDC();
    pWnd->GetClientRect ( &rect );
    pDC->SetStretchBltMode ( STRETCH_HALFTONE );
    myImage.Draw ( pDC->m_hDC, rect );
    ReleaseDC ( pDC );
    myImage.Destroy();
}

void CPanoDialog::OnPaint() {
    static bool mark = true;
    if (true) {
        if ( this->m_currentProductId != -1 ) {
            int id = this->m_currentPanoId % m_currentProduct.panoPath.size();
            LoadMeasureImage ( m_currentProduct.panoPath[id].c_str() );
        }
        mark = false;
    }
}

// CPanoDialog message handlers


void CPanoDialog::OnBnClickedPanoBeforeButton() {
    if ( this->m_currentProductId != -1 ) {
        this->m_currentPanoId++;
        int id = this->m_currentPanoId % m_currentProduct.panoPath.size();
        LoadMeasureImage ( m_currentProduct.panoPath[id].c_str() );

        if ( this->m_currentPanoId > 300 ) {
            this->m_currentPanoId = 200;
        }
    }
}


void CPanoDialog::OnBnClickedPanoNextButton() {
    if ( this->m_currentProductId != -1 ) {
        this->m_currentPanoId--;
        int id = this->m_currentPanoId % m_currentProduct.panoPath.size();
        LoadMeasureImage ( m_currentProduct.panoPath[id].c_str() );

        if ( this->m_currentPanoId < 100 ) {
            this->m_currentPanoId = 200;
        }
    }
}
