
// ImageMeasureDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ImageMeasure.h"
#include "ImageMeasureDlg.h"
#include "afxdialogex.h"
#include "Transmission.h"
#include "ProductList.h"
#include "CatelogDialog.h"
#include "Types.h"
#include <cmath>
#include "strtools.hpp"
//#include "VideoCaptureModuleDll.h"
#include <iostream>

#include <boost/thread.hpp>
#include "opencv2/opencv.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static  BOOL transmissionButtonEnable_pre;
boost::mutex g_transmissionLock;
cv::VideoCapture cap;

void _500msDelay() {
    Sleep(500);
    transmissionButtonEnable_pre = TRUE;
}

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx {
  public:
    CAboutDlg();

// Dialog Data
    enum { IDD = IDD_ABOUTBOX };

  protected:
    virtual void DoDataExchange ( CDataExchange* pDX ); // DDX/DDV support

// Implementation
  protected:
    DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx ( CAboutDlg::IDD ) {
}

void CAboutDlg::DoDataExchange ( CDataExchange* pDX ) {
    CDialogEx::DoDataExchange ( pDX );
}

BEGIN_MESSAGE_MAP ( CAboutDlg, CDialogEx )
END_MESSAGE_MAP()


// CImageMeasureDlg dialog



CImageMeasureDlg::CImageMeasureDlg ( CWnd* pParent /*=NULL*/ )
    : CDialogEx ( CImageMeasureDlg::IDD, pParent ) {
//  _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
//  _CrtSetBreakAlloc(505);
    m_hIcon = AfxGetApp()->LoadIcon ( IDR_MAINFRAME );
    this->m_menu = nullptr;
    this->m_catelogDialog = nullptr;
    this->m_listDialog = nullptr;
    m_currentSide = 0;
    m_hasProduct = false;
    m_statBar = nullptr;

    /*    cap.open(0);*/
}

void CImageMeasureDlg::DoDataExchange ( CDataExchange* pDX ) {
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_MEASURE_LIST, m_measureHistory);
    DDX_Control(pDX, IDC_TRANSMISSION_BUTTON, m_transmissionButton);
    DDX_Control(pDX, IDC_PRODUCT_NAME_EDIT, m_productName);
    DDX_Control(pDX, IDC_WEIGHT_TEXT, m_productWeight);
    DDX_Control(pDX, IDC_SIZE_TEXT, m_productSize);
    DDX_Control(pDX, IDC_CATELOG_COMBO, m_catelogComboBox);
    DDX_Control(pDX, IDC_MEASURE_PICTURE, m_measurePicture);
    DDX_Control(pDX, IDC_BUTTON_SIDE, m_changeSideButton);
}

BEGIN_MESSAGE_MAP ( CImageMeasureDlg, CDialogEx )
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_WM_DESTROY()
    ON_WM_LBUTTONDOWN()
    ON_BN_CLICKED ( IDC_TRANSMISSION_BUTTON, &CImageMeasureDlg::OnBnClickedTransmissionButton )
    ON_MESSAGE ( USER_INFO_MSG, OnUserInfoMsg )
    ON_COMMAND ( ID_FILE_LOAD, &CImageMeasureDlg::OnFileLoad )
    ON_COMMAND ( ID_CATELOG_MENU, &CImageMeasureDlg::OnCatelogMenu )
    ON_BN_CLICKED ( IDC_DELETE_DATA_BUTTON, &CImageMeasureDlg::OnBnClickedDeleteDataButton )
    ON_BN_CLICKED ( IDC_SAVE_DATA_BUTTON, &CImageMeasureDlg::OnBnClickedSaveDataButton )
    ON_BN_CLICKED ( IDC_CLEARLINE_BUTTON, &CImageMeasureDlg::OnBnClickedClearlineButton )
    ON_BN_CLICKED ( IDC_PANO_BUTTON, &CImageMeasureDlg::OnBnClickedPanoButton )
    ON_BN_CLICKED ( IDC_SAVE_MEASURE_INFO_BUTTON, &CImageMeasureDlg::OnBnClickedSaveMeasureInfoButton )
    ON_BN_CLICKED(IDC_BUTTON_SIDE, &CImageMeasureDlg::OnBnClickedButtonSide)
    ON_WM_TIMER()
    ON_BN_CLICKED(IDC_BUTTON_CVTEST, &CImageMeasureDlg::OnBnClickedButtonCvtest)
END_MESSAGE_MAP()

LRESULT CImageMeasureDlg::OnUserInfoMsg ( WPARAM wp, LPARAM lp ) {
    info_msg *msg = ( info_msg * ) wp;

    this->m_receivedCounter++;
    CString count;
    count.Format(L"%d", this->m_receivedCounter);
    /*    m_statBar->SetText(count, 1, 0);*/
    info_msg::Destruct(msg);

    return 0;
}

// CImageMeasureDlg message handlers

BOOL CImageMeasureDlg::OnInitDialog() {
    CDialogEx::OnInitDialog();

    // Add "About..." menu item to system menu.

    // IDM_ABOUTBOX must be in the system command range.
    ASSERT ( ( IDM_ABOUTBOX & 0xFFF0 ) == IDM_ABOUTBOX );
    ASSERT ( IDM_ABOUTBOX < 0xF000 );

    CMenu* pSysMenu = GetSystemMenu ( FALSE );
    if ( pSysMenu != NULL ) {
        BOOL bNameValid;
        CString strAboutMenu;
        bNameValid = strAboutMenu.LoadString ( IDS_ABOUTBOX );
        ASSERT ( bNameValid );
        if ( !strAboutMenu.IsEmpty() ) {
            pSysMenu->AppendMenu ( MF_SEPARATOR );
            pSysMenu->AppendMenu ( MF_STRING, IDM_ABOUTBOX, strAboutMenu );
        }
    }

    // Set the icon for this dialog.  The framework does this automatically
    //  when the application's main window is not a dialog
    SetIcon ( m_hIcon, TRUE );          // Set big icon
    SetIcon ( m_hIcon, FALSE );     // Set small icon

    // TODO: Add extra initialization here
    this->m_menu = new CMenu();
    this->m_menu->LoadMenu ( IDR_MEASURE_MENU );
    SetMenu ( this->m_menu );
    {
        CRect rect;
        this->m_measurePicture.GetWindowRect ( rect );
        ScreenToClient ( rect );
        rect.top = 5;
        rect.left = 20;
        rect.right = rect.left + 472;
        rect.bottom = rect.top + 472;
        this->m_measurePicture.MoveWindow ( rect );
    }

//     {
//         m_statBar = new CStatusBarCtrl;
//         RECT m_Rect;
//         GetClientRect(&m_Rect); //获取对话框的矩形区域
//         m_Rect.top = m_Rect.bottom - 20; //设置状态栏的矩形区域
//         m_statBar->Create(WS_BORDER | WS_VISIBLE | CBRS_BOTTOM, m_Rect, this, 3);
//         int nParts[4] = { 150, 50, 0, -1}; //分割尺寸
//         m_statBar->SetParts(4, nParts); //分割状态栏
//         m_statBar->SetText(L"本次共接收图像:", 0, 0); //第一个分栏加入"这是第一个指示器"
//         m_statBar->SetText(L"0", 1, 0); //以下类似
//         m_statBar->ShowWindow(SW_SHOW);
//     }

    this->m_dataStore = std::make_shared<CImageStore>();
    this->m_transmisson = std::make_shared<CTransmission>();
    CRect rect;
    this->m_measureHistory.GetClientRect ( &rect );

    this->m_measureHistory.SetExtendedStyle ( m_measureHistory.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP | LVS_EX_TWOCLICKACTIVATE );
    this->m_measureHistory.InsertColumn ( 0, _T ( "序号" ), LVCFMT_CENTER, rect.Width() * 1 / 10, 0 );
    this->m_measureHistory.InsertColumn ( 1, _T ( "尺寸（mm）" ), LVCFMT_CENTER, rect.Width() * 3 / 10, 1 );
    this->m_measureHistory.InsertColumn ( 2, _T ( "备注" ), LVCFMT_CENTER, rect.Width() * 6 / 10, 2 );
    this->m_measureHistory.InsertColumn ( 3, _T ( "" ), LVCFMT_CENTER, 0, 3 );
    this->m_measureHistory.InsertColumn ( 4, _T ( "" ), LVCFMT_CENTER, 0, 4 );
    this->m_measureHistory.InsertColumn ( 5, _T ( "" ), LVCFMT_CENTER, 0, 5 );
    this->m_measureHistory.InsertColumn ( 6, _T ( "" ), LVCFMT_CENTER, 0, 6 );
//     this->m_measureHistory.InsertItem(0, _T("1"));
//     this->m_measureHistory.SetItemText(0, 1, _T("1"));
//     this->m_measureHistory.SetItemText(0, 2, _T("Test"));

    this->m_measureHistory.SetNoEditCol ( 0 );
    this->m_measureHistory.SetNoEditCol ( 1 );

    this->m_listDialog = new CProductListDialog();
    this->m_catelogDialog = new CCatelogDialog();
    this->m_panoDialog = new CPanoDialog();

    this->m_listDialog->setImageStore ( this->m_dataStore );
    this->m_catelogDialog->setImageStore ( this->m_dataStore );
    this->m_panoDialog->setImageStore ( this->m_dataStore );

    m_measureInfoSize = 0;
    m_receivedCounter = 0;
    m_measureDrawState = WAIT_FIRST_POINT;

    this->m_currentProduct.id = -1;
    this->m_panoDialog->CurrentProductId ( this->m_currentProduct.id );

    return TRUE;  // return TRUE  unless you set the focus to a control
}

void CImageMeasureDlg::OnSysCommand ( UINT nID, LPARAM lParam ) {
    if ( ( nID & 0xFFF0 ) == IDM_ABOUTBOX ) {
        CAboutDlg dlgAbout;
        dlgAbout.DoModal();
    } else {
        CDialogEx::OnSysCommand ( nID, lParam );
    }
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CImageMeasureDlg::OnPaint() {
    if ( IsIconic() ) {
        CPaintDC dc ( this ); // device context for painting

        SendMessage ( WM_ICONERASEBKGND, reinterpret_cast<WPARAM> ( dc.GetSafeHdc() ), 0 );

        // Center icon in client rectangle
        int cxIcon = GetSystemMetrics ( SM_CXICON );
        int cyIcon = GetSystemMetrics ( SM_CYICON );
        CRect rect;
        GetClientRect ( &rect );
        int x = ( rect.Width() - cxIcon + 1 ) / 2;
        int y = ( rect.Height() - cyIcon + 1 ) / 2;

        // Draw the icon
        dc.DrawIcon ( x, y, m_hIcon );
    } else {
        CDialogEx::OnPaint();
    }
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CImageMeasureDlg::OnQueryDragIcon() {
    return static_cast<HCURSOR> ( m_hIcon );
}

BOOL CImageMeasureDlg::PreTranslateMessage ( MSG *pMsg ) {
    // TODO: Add your specialized code here and/or call the base class

    if ( pMsg->message == WM_LBUTTONDOWN && GetDlgItem ( IDC_MEASURE_PICTURE )->GetSafeHwnd() == pMsg->hwnd ) {

        ScreenToClient ( & ( pMsg->pt ) );

        CPoint pt;

        int xPos = GET_X_LPARAM ( pMsg->lParam );
        int yPos = GET_Y_LPARAM ( pMsg->lParam );

        pt.x = xPos;
        pt.y = yPos;

        OnLButtonDown ( MK_LBUTTON, pMsg->pt );
    }

    return CDialogEx::PreTranslateMessage ( pMsg );
}

void CImageMeasureDlg::OnLButtonDown ( UINT nFlags, CPoint point ) {
    if ( this->m_currentProduct.id == -1 ) {
        return;
    }

    CString strPoint;

    CRect lRect;
    ( GetDlgItem ( IDC_MEASURE_PICTURE ) )->GetWindowRect ( &lRect );
    ScreenToClient ( &lRect );
    if ( ( point.x >= lRect.left && point.x <= lRect.right ) && ( point.y >= lRect.top && point.y <= lRect.bottom ) ) {
        strPoint.AppendFormat ( L"%d %d", point.x - lRect.left, point.y - lRect.top );
//        AfxMessageBox(strPoint);
        m_measurePoints.push_back ( point );
        if ( this->m_measureDrawState == WAIT_FIRST_POINT ) {
            this->m_measureDrawState = WAIT_SECOND_POINT;
            COLORREF color ( RGB ( 255, 0, 0 ) );
            CClientDC aDC ( this );
            CPen pen ( PS_SOLID, 5, color );
            aDC.SelectObject ( &pen );
            aDC.MoveTo ( point );
            aDC.LineTo ( point );
        } else {
            CPoint sP, eP;
            sP = m_measurePoints[0];
            eP = m_measurePoints[1];
            m_measurePoints.clear();
            this->m_measureDrawState = WAIT_FIRST_POINT;
            COLORREF color ( RGB ( 255, 0, 0 ) );
            CClientDC aDC ( this );
            CPen pen ( PS_SOLID, 3, color );
            aDC.SelectObject ( &pen );
            aDC.MoveTo ( sP );
            aDC.LineTo ( eP );

            ImageMeasure::Line line;
            line.start = sP;
            line.end = eP;

            this->m_measureLines.push_back ( line );

            ImageMeasure::Point2I startI, endI;
            ImageMeasure::Point2D start, end;
            double length = 0;
            startI.x = sP.x;
            startI.y = sP.y;
            endI.x = eP.x;
            endI.y = eP.y;

            this->m_calculator.CalculateLocation ( startI, start );
            this->m_calculator.CalculateLocation ( endI, end );
            this->m_calculator.CalculateLength ( this->m_currentProduct.overlookPathMin.c_str(), this->m_currentProduct.sidelookPathMin.c_str(), start, end, length , this->m_currentProduct.isHeight , this->m_currentProduct );

            TCHAR lengthStr[1024];
            swprintf_s ( lengthStr, 1024, L"%2lf", length );

            int count = m_measureHistory.GetItemCount();
            this->m_measureHistory.InsertItem ( count, _T ( "-1" ) );
            this->m_measureHistory.SetItemText ( count, 1, lengthStr );
            this->m_measureHistory.SetItemText ( count, 2, _T ( "" ) );


            TCHAR sx[1024];
            swprintf_s ( sx, 1024, L"%d", sP.x );
            TCHAR sy[1024];
            swprintf_s ( sy, 1024, L"%d", sP.y );
            TCHAR ex[1024];
            swprintf_s ( ex, 1024, L"%d", eP.x );
            TCHAR ey[1024];
            swprintf_s ( ey, 1024, L"%d", eP.y );

            this->m_measureHistory.SetItemText ( count, 3, sx );
            this->m_measureHistory.SetItemText ( count, 4, sy );
            this->m_measureHistory.SetItemText ( count, 5, ex );
            this->m_measureHistory.SetItemText ( count, 6, ey );

            ImageMeasure::MeasureInfo info;
            info.start.x = sP.x;
            info.start.y = sP.y;
            info.end.x = eP.x;
            info.end.x = eP.y;
            info.pid = this->m_currentProduct.id;
            info.length = length;

            this->m_measureInfoVector.push_back ( info );
            m_measureInfoSize++;
        }
    } else {
        this->m_measurePoints.clear();
        this->m_measureDrawState = WAIT_FIRST_POINT;
    }
}

void CImageMeasureDlg::OnBnClickedTransmissionButton() {
    this->m_transmissionButton.EnableWindow(FALSE);
    transmissionButtonEnable_pre = FALSE;
    if (g_threadState == PROCESSING) {
        AfxMessageBox(_T("数据正在处理中，为保证数据安全，请稍后重试。"));
        this->m_transmissionButton.EnableWindow(TRUE);
        transmissionButtonEnable_pre = TRUE;
        return;
    }

    g_transmissionLock.lock();
    try {
        if ( m_transmisson->IsConnected() ) {
            m_transmisson->CloseConnection();

            if ( m_receivedCounter > 0 ) {
                this->LoadLastestImage();
            }

            this->m_transmissionButton.SetWindowText(_T("开始传输"));
        } else {
            m_transmisson->Listener ( new CServerListener ( this ) );
            BOOL state = m_transmisson->OpenConnection();
            if (!state) {
                AfxMessageBox(L"开启失败");
                return;
            }

            m_receivedCounter = 0;
            this->m_transmissionButton.SetWindowText ( _T ( "结束传输" ) );
        }
    } catch (std::exception &e) {

    }
    g_transmissionLock.unlock();

    SetTimer(1, 100, NULL);

    boost::thread thd(_500msDelay);

//     //DEBUG
//     LoadMeasureImage(L"test");
}

void CImageMeasureDlg::OnDestroy() {
//     while (g_threadState == PROCESSING) {
//      Sleep(100);
//     }
    cap.release();
    if (m_transmisson->IsConnected()) {
        m_transmisson->CloseConnection();
    }

    g_stopThread = 1;
    g_finish = 1;
    QueueEmpty.set(0);
    QueueEmpty.signal();
    g_CheckBuffThread.join();

    if ( g_buff[0] != nullptr ) {
        delete[] g_buff[0];
        g_buff[0] = nullptr;
    }

    if ( g_buff[1] != nullptr ) {
        delete[] g_buff[1];
        g_buff[1] = nullptr;
    }

    g_pMainWnd = nullptr;


    if ( this->m_menu != nullptr ) {
        delete this->m_menu;
        this->m_menu = nullptr;
    }

    if (this->m_statBar != nullptr) {
        delete this->m_statBar;
        this->m_statBar = nullptr;
    }

    if ( this->m_listDialog != nullptr ) {
        delete this->m_listDialog;
        this->m_listDialog = nullptr;
    }

    if ( this->m_catelogDialog != nullptr ) {
        delete this->m_catelogDialog;
        this->m_catelogDialog = nullptr;
    }

    if ( this->m_panoDialog != nullptr ) {
        delete this->m_panoDialog;
        this->m_panoDialog = nullptr;
    }

    if ( this->m_dataStore != nullptr ) {
        if ( this->m_dataStore->Connected() ) {
            this->m_dataStore->CloseDatabase();
        }

        this->m_dataStore.reset();
    }

    CDialogEx::OnDestroy();
}

void CImageMeasureDlg::LoadMeasureImage ( const TCHAR *imagePath ) {
    if (_waccess(imagePath, 0) == -1) {
        // Bad File
        AfxMessageBox(L"图像丢失！", MB_OK | MB_SYSTEMMODAL);
        return;
    }
    CImage myImage;
    myImage.Load ( imagePath );

    CRect rect;
    CWnd *pWnd = GetDlgItem ( IDC_MEASURE_PICTURE );
    CDC *pDC = pWnd->GetDC();
    pWnd->GetClientRect ( &rect );
    pDC->SetStretchBltMode ( STRETCH_HALFTONE );
    myImage.Draw ( pDC->m_hDC, rect );
    ReleaseDC ( pDC );
    myImage.Destroy();
}

void CImageMeasureDlg::UpdateCatelogComboBox ( std::map<int, ImageMeasure::Catelog> &catelogDict ) {
    this->m_catelogComboBox.ResetContent();
    this->m_catelogComboDict.clear();
    int count = 0;
    int markId = -1;
    for ( auto i = catelogDict.begin(); i != catelogDict.end(); i++ ) {
        auto& catelog = i->second;
        m_catelogComboDict[count] = catelog.id;
        this->m_catelogComboBox.AddString ( catelog.name.c_str() );
        if ( catelog.id == this->m_currentProduct.type.id ) {
            markId = count;
        }
        count++;
    }

    this->m_catelogComboBox.SetCurSel ( markId );
}

void CImageMeasureDlg::LoadProductToWnd ( ImageMeasure::Product &product ) {
    m_measureListInfoDict.clear();
    this->m_measureHistory.DeleteAllItems();
    if (this->m_currentSide == 0) {
        LoadMeasureImage(product.overlookPath1.c_str());
    } else {
        LoadMeasureImage(product.overlookPath2.c_str());
    }

    this->m_productName.SetWindowText ( product.name.c_str() );

    TCHAR weight[64];
    swprintf_s ( weight, 64, L"%.3lfg", product.weight );
    this->m_productWeight.SetWindowText ( weight );

//     {
//         if ( std::fabs ( product.length + 1 ) < FLT_EPSILON ) {
//             this->m_calculator.CalculateSize ( product.overlookPath.c_str(), product.sidelookPath.c_str(), product.width, product.height, product.length );
//             this->m_dataStore->UpdateProduct ( product );
//         }
//     }

    TCHAR productSize[128];
    swprintf_s ( productSize, 128, L"%.2lfmm * %.2lfmm * %.2lfmm", product.length, product.width, product.height );
    this->m_productSize.SetWindowText ( productSize );

    for ( int i = 0; i < product.measureInfo.size(); i++ ) {
        auto &measureInfo = product.measureInfo[i];

        if (measureInfo.side != this->m_currentSide) {
            continue;
        }

        ImageMeasure::Point2I startI, endI;
        ImageMeasure::Point2D start, end;
        double length = 0;

        startI = measureInfo.start;
        endI = measureInfo.end;

        try {
            this->m_calculator.CalculateLocation ( startI, start );
            this->m_calculator.CalculateLocation ( endI, end );
            this->m_calculator.CalculateLength ( this->m_currentProduct.overlookPathMin.c_str(), this->m_currentProduct.sidelookPathMin.c_str(), start, end, length, this->m_currentProduct.isHeight, this->m_currentProduct );
        } catch (std::exception &e) {

        }

        m_measureListInfoDict[i] = measureInfo.id;
        TCHAR idStr[1024];
        swprintf_s ( idStr, 1024, L"%d", i + 1 );

        TCHAR lengthStr[1024];
        swprintf_s ( lengthStr, 1024, L"%2lf", length );

        int nIndex = m_measureHistory.InsertItem ( m_measureHistory.GetItemCount(), _T ( "" ) );
//      nIndex = 0;
        m_measureHistory.SetItemText ( nIndex, 0, idStr );
        m_measureHistory.SetItemText ( nIndex, 1, lengthStr );
        m_measureHistory.SetItemText ( nIndex, 2, measureInfo.desc.c_str() );
        this->m_measureHistory.SetItemText ( nIndex, 3, _T ( "" ) );
        this->m_measureHistory.SetItemText ( nIndex, 4, _T ( "" ) );
        this->m_measureHistory.SetItemText ( nIndex, 5, _T ( "" ) );
        this->m_measureHistory.SetItemText ( nIndex, 6, _T ( "" ) );
    }
}

void CImageMeasureDlg::OnFileLoad() {
    this->m_listDialog->ChooseProductId ( -1 );
    m_listDialog->DoModal();

    int chooseId = this->m_listDialog->ChooseProductId();

    if (chooseId == -1) {
        m_hasProduct = false;
    } else {
        m_hasProduct = true;
    }

    if (m_hasProduct) {
        this->m_currentSide = 0;

//         std::map<int, ImageMeasure::Product> productDict;
//         m_dataStore->GetAllProduct ( productDict );
//
//         if ( productDict.count ( chooseId ) < 0 ) {
//             return;
//         }

        ImageMeasure::Product product;

        bool result = m_dataStore->GetProductById(chooseId, product);

        if (!result) {
            return;
        }

        m_currentSide = 0;
        this->m_changeSideButton.SetWindowText(L"看反面");

        m_measureInfoSize = 0;
        m_measureInfoSize += product.measureInfo.size();

        m_currentProduct = product;
        LoadProductToWnd(m_currentProduct);

        for (int i = 0; i < m_currentProduct.measureInfo.size(); i++) {
            auto &info = m_currentProduct.measureInfo[i];

            if (info.side != this->m_currentSide) {
                continue;
            }

            CPoint sP, eP;
            sP.x = info.start.x;
            sP.y = info.start.y;

            eP.x = info.end.x;
            eP.y = info.end.y;

            COLORREF color(RGB(0, 255, 0));
            CClientDC aDC(this);
            CPen pen(PS_SOLID, 3, color);
            aDC.SelectObject(&pen);
            aDC.MoveTo(sP);
            aDC.LineTo(eP);

        }

        std::map<int, ImageMeasure::Catelog> catelogDict;
        this->m_dataStore->GetAllCateLog ( catelogDict );
        UpdateCatelogComboBox ( catelogDict );
    }
}


void CImageMeasureDlg::OnCatelogMenu() {
    m_catelogDialog->DoModal();

    std::map<int, ImageMeasure::Catelog> catelogDict;
    this->m_dataStore->GetAllCateLog ( catelogDict );

    if ( m_currentProduct.id == -1 ) {
        return;
    }

    UpdateCatelogComboBox(catelogDict);

    std::map<int, ImageMeasure::Product> productDict;
    this->m_dataStore->GetAllProduct ( productDict );

    this->m_currentProduct = productDict[this->m_currentProduct.id];

    this->LoadProductToWnd ( m_currentProduct );
}


void CImageMeasureDlg::OnBnClickedDeleteDataButton() {
    if (!m_hasProduct) {
        return;
    }

    if ( ::MessageBox ( this->m_hWnd, L"确认删除此产品？\n", L"删除", MB_OKCANCEL ) == IDOK ) {
        this->m_dataStore->DeleteProduct ( this->m_currentProduct.id );
        this->m_currentProduct.id = -1;
        this->m_productWeight.SetWindowText ( _T ( "" ) );
        this->m_productSize.SetWindowText ( _T ( "" ) );
        this->m_measureHistory.DeleteAllItems();
        this->m_hasProduct = false;
        Invalidate();
    }

    m_hasProduct = false;
}

void CImageMeasureDlg::OnBnClickedSaveDataButton() {
    if (!m_hasProduct) {
        return;
    }

    int nIndex = this->m_catelogComboBox.GetCurSel();
    if ( nIndex == -1 ) {
        AfxMessageBox ( _T ( "请选择类型" ) );
        return;
    }

    int count = this->m_measureHistory.GetItemCount();
    for ( int i = 0; i < count; i++ ) {
        ImageMeasure::MeasureInfo info;

        CString idString = this->m_measureHistory.GetItemText ( i, 0 );

        int id = _ttoi ( idString.GetBuffer() );

        if ( id == -1 ) {
            CString name = this->m_measureHistory.GetItemText ( i, 2 );
            CString sx = this->m_measureHistory.GetItemText ( i, 3 );
            CString sy = this->m_measureHistory.GetItemText ( i, 4 );
            CString ex = this->m_measureHistory.GetItemText ( i, 5 );
            CString ey = this->m_measureHistory.GetItemText ( i, 6 );

            info.desc = name.GetBuffer();
            info.name = L"";
            info.start.x = _ttoi ( sx.GetBuffer() );
            info.start.y = _ttoi ( sy.GetBuffer() );
            info.end.x = _ttoi ( ex.GetBuffer() );
            info.end.y = _ttoi ( ey.GetBuffer() );

            this->m_dataStore->InsertMeasureInfo ( info );
        }
    }

    this->m_productName.UpdateData ( false );
    CString str;
    this->m_productName.GetWindowText ( str );

    this->m_currentProduct.name = str.GetBuffer();

    int catelogId = m_catelogComboDict[m_catelogComboBox.GetCurSel()];
    this->m_currentProduct.type.id = catelogId;

    this->m_dataStore->UpdateProduct ( this->m_currentProduct );
}

void CImageMeasureDlg::LoadLastestImage() {

}


void CImageMeasureDlg::OnBnClickedClearlineButton() {
//  this->Invalidate();
    m_measureInfoVector.clear();
    this->m_measureLines.clear();
    this->m_measurePoints.clear();
    this->m_measureDrawState = WAIT_FIRST_POINT;
    if ( this->m_currentProduct.id != -1 ) {
        this->LoadProductToWnd ( this->m_currentProduct );
    }
}

void CImageMeasureDlg::OnBnClickedPanoButton() {
    if (!m_hasProduct) {
        return;
    }

    this->m_panoDialog->CurrentProductId ( this->m_currentProduct.id );
    this->m_panoDialog->DoModal();
}

void CImageMeasureDlg::OnBnClickedSaveMeasureInfoButton() {
    if (!m_hasProduct) {
        return;
    }

    int count = this->m_measureHistory.GetItemCount();
    for ( int i = 0; i < count; i++ ) {
        ImageMeasure::MeasureInfo info;

        CString idString = this->m_measureHistory.GetItemText ( i, 0 );

        const wchar_t *wccId = idString.GetBuffer();
        char *idChar = ( char * ) malloc ( sizeof ( char ) * ( 2 * wcslen ( wccId ) + 1 ) );
        memset ( idChar, 0, 2 * wcslen ( wccId ) + 1 );
        strtool::w2c ( idChar, wccId, 2 * wcslen ( wccId ) + 1 );

        int measureInfoId = atol ( idChar );

        if ( measureInfoId != -1 ) {
            continue;
        }

        info.id = measureInfoId;
        info.name = this->m_measureHistory.GetItemText ( i, 2 ).GetBuffer();
        info.desc = this->m_measureHistory.GetItemText ( i, 2 ).GetBuffer();
        CString sx = this->m_measureHistory.GetItemText ( i, 3 );
        CString sy = this->m_measureHistory.GetItemText ( i, 4 );
        CString ex = this->m_measureHistory.GetItemText ( i, 5 );
        CString ey = this->m_measureHistory.GetItemText ( i, 6 );

        info.start.x = _ttoi ( sx );
        info.start.y = _ttoi ( sy );
        info.end.x = _ttoi ( ex );
        info.end.y = _ttoi ( ey );
        info.pid = this->m_currentProduct.id;
        info.side = this->m_currentSide;

        if ( info.id == -1 ) {
            this->m_dataStore->InsertMeasureInfo ( info );
        }

        free ( idChar );
        idChar = nullptr;
    }

    std::map<int, ImageMeasure::Product> productDict;
    m_dataStore->GetAllProduct(productDict);
    this->m_currentProduct = productDict[this->m_currentProduct.id];

}

// CImageMeasureDlg::~CImageMeasureDlg()
// {
//  _CrtDumpMemoryLeaks();
// }

void CImageMeasureDlg::ChangeSide() {

    if (m_hasProduct) {
        LoadProductToWnd(m_currentProduct);

        for (int i = 0; i < m_currentProduct.measureInfo.size(); i++) {
            auto &info = m_currentProduct.measureInfo[i];

            if (info.side != this->m_currentSide) {
                continue;
            }

            CPoint sP, eP;
            sP.x = info.start.x;
            sP.y = info.start.y;

            eP.x = info.end.x;
            eP.y = info.end.y;

            COLORREF color(RGB(0, 255, 0));
            CClientDC aDC(this);
            CPen pen(PS_SOLID, 3, color);
            aDC.SelectObject(&pen);
            aDC.MoveTo(sP);
            aDC.LineTo(eP);

        }
    }
}

void CImageMeasureDlg::OnBnClickedButtonSide() {
    if (!m_hasProduct) {
        return;
    }

    if (m_currentSide == 0) {
        m_currentSide = 1;
        this->m_changeSideButton.SetWindowText(L"看正面");

    } else {
        m_currentSide = 0;
        this->m_changeSideButton.SetWindowText(L"看反面");
    }

    ChangeSide();
}


void CImageMeasureDlg::OnTimer(UINT_PTR nIDEvent) {
    // TODO: Add your message handler code here and/or call default
    if (transmissionButtonEnable_pre) {

        GetDlgItem(IDC_TRANSMISSION_BUTTON)->EnableWindow(TRUE);
        KillTimer(1);
    } else {

        GetDlgItem(IDC_TRANSMISSION_BUTTON)->EnableWindow(FALSE);
    }

    CDialogEx::OnTimer(nIDEvent);
}

void captureImage() {
    try {
        cv::VideoCapture cap(0);
//      cap.set(CV_CAP_PROP_SETTINGS, 1);
        cap.set(CV_CAP_PROP_FPS, 15);
        cap.set(CV_CAP_PROP_FRAME_WIDTH, 2048);
        cap.set(CV_CAP_PROP_FRAME_HEIGHT, 1536);

        if (!cap.isOpened()) {
            return;
        }
        cv::Mat frame;
        while (!cap.read(frame)) {
            int count = 0;
            count++;
        }

        //cap >> frame;
//      Sleep(1000);
        cv::imwrite("d:/test.jpg", frame);

    } catch (std::exception e) {
        std::cout << e.what();
    }
}

void cutTest(std::wstring woverstr) {
    {
        const wchar_t* oPathW = woverstr.c_str();
        char* oPath = (char*)malloc(sizeof (char) * (2 * wcslen(oPathW) + 1));
        memset(oPath, 0, 2 * wcslen(oPathW) + 1);
        strtool::w2c(oPath, oPathW, 2 * wcslen(oPathW) + 1);

        cv::Mat pano = cv::imread(oPath);

        const int colOffset = 412;
        const int rowOffset = 250;

        cv::Mat temp = cv::Mat(710, 760, CV_8UC3);
        for (int j = 0; j < temp.rows; j++) {
            for (int i = 0; i < temp.cols; i++) {
                cv::Point3_<uchar>* p = pano.ptr<cv::Point3_<uchar> >(j + rowOffset, i + colOffset);
                temp.ptr<cv::Point3_<uchar> >(j, i)->x = p->x;
                temp.ptr<cv::Point3_<uchar> >(j, i)->y = p->y;
                temp.ptr<cv::Point3_<uchar> >(j, i)->z = p->z;
            }
        }

        cv::imwrite(oPath, temp);

        free(oPath);
        oPath = nullptr;
    }
}


void CImageMeasureDlg::OnBnClickedButtonCvtest() {

    cv::Mat src, dst;
    double alpha = 1.2;
    double beta = 60;

    src = cv::imread("d:\\b.jpg");
    if (!src.data) {
        cout << "Failed to load image!" << endl;
        return ;
    }

    dst = cv::Mat::zeros(src.size(), src.type());
    for (int i = 0; i < src.rows; ++i)
        for (int j = 0; j < src.cols; ++j)
            for (int k = 0; k < 3; ++k)
            { dst.at<cv::Vec3b>(i, j)[k] = cv::saturate_cast<uchar>(src.at<cv::Vec3b>(i, j)[k] * alpha + beta); }

    cv::imwrite("d:\\b.out.jpg", dst);
    return;
//     CCameraCalculator calculatetor;
//     double width = 0;
//     double height = 0;
//     double length = 0;
//     calculatetor.CalculateSize(L"D:\\b.jpg", L"D:\\s.jpg", width, height, length);
//     cutTest(L"d:\\p.jpg");
//
//     std::ofstream fout("D:\\info.txt");
//     fout << "w " << width << " h " << height << " l " << length;
//     fout.close();
//     int cameraCount = 0;
//     int select = 0;
//     VideoCaptureModule_Initialize();
//     VideoCaptureModule_GetCameraCount(cameraCount);
//     VideoCaptureModule_SetSelectedCamera(select);
//     VideoCaptureModule_SetFrameSize(1600, 1200);
//     VideoCaptureModule_Open();
//     VideoCaptureModule_Play();
//     int width, height;
//     BYTE *image = new BYTE[1600 * 1200 * 3];
//     for (size_t i = 0; i < 1000; i++) {
//      VideoCaptureModule_GetFrameSize(width, height);
//         VideoCaptureModule_GetFrame(image, width, height);
//     }


//     IplImage* img = cvCreateImageHeader(cvSize(width, height), 255, 3);
//     cvSetData(img, image, width * 3);
//    int lineByte = (width + 3) / 4 * 4; //格式宽度

//
//     cv::Mat iMat(height, width, CV_8UC3, image, width);
//
//     cv::imwrite("d:\\t.jpg", iMat);

//    BITMAPINFOHEADER m_bih;
//     memset(&m_bih, 0, sizeof(m_bih));
//     m_bih.biSize = sizeof(m_bih);
//     m_bih.biWidth = width;
//     m_bih.biHeight = height;
//     m_bih.biPlanes = 1;
//     m_bih.biBitCount = 24;
//
//     CFile file;
//     file.Open(L"D:\\test.bmp", CFile::modeReadWrite | CFile::modeCreate);
//     file.Write(&m_bih, sizeof(m_bih));
//     file.Write(image,  sizeof(BYTE) * width * height * 3);
//     file.Close();
//     VideoCaptureModule_Close();
//     delete image;
//     //设置显示图像格式BMP头
//     //BITMAPINFOHEADER m_bih;
//     memset(&m_bih, 0, sizeof(m_bih));
//     m_bih.biSize = sizeof(m_bih);
//     m_bih.biWidth = m_nWidth;
//     m_bih.biHeight = m_nHeight;
//     m_bih.biPlanes = 1;
//     m_bih.biBitCount = 24;
//     m_GetFrameTimer = SetTimer(GETFRAME_TIMER, 67, NULL);

//     try {
//         //      cap.set(CV_CAP_PROP_SETTINGS, 1);
//         cap.set(CV_CAP_PROP_FPS, 15);
//         cap.set(CV_CAP_PROP_FRAME_WIDTH, 2048);
//         cap.set(CV_CAP_PROP_FRAME_HEIGHT, 1536);
//
//         if (!cap.isOpened()) {
//             return;
//         }
//         cv::Mat frame;
// //         while (!cap.read(frame)) {
// //             int count = 0;
// //             count++;
// //             Sleep(10);
// //         }
//         int i = 5;
//         while (i--) {
//             cap >> frame;
//         }
//         //      Sleep(1000);
//         cv::imwrite("d:/0.jpg", frame);
//
//     } catch (std::exception e) {
//         std::cout << e.what();
//     }

//  try {
//         cv::VideoCapture cap(1);
//         //      cap.set(CV_CAP_PROP_SETTINGS, 1);
//         cap.set(CV_CAP_PROP_FPS, 15);
//         cap.set(CV_CAP_PROP_FRAME_WIDTH, 2048);
//         cap.set(CV_CAP_PROP_FRAME_HEIGHT, 1536);
//
//         if (!cap.isOpened()) {
//             return;
//         }
//         cv::Mat frame;
//         while (!cap.read(frame)) {
//             int count = 0;
//             count++;
//             Sleep(10);
//
//         }
//
//         //cap >> frame;
//         //      Sleep(1000);
//         cv::imwrite("d:/1.jpg", frame);
//
//     } catch (std::exception e) {
//         std::cout << e.what();
//     }

//    AfxMessageBox(L"OK");
}
