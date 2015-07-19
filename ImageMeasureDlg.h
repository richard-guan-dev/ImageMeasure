
// ImageMeasureDlg.h : header file
//

#pragma once
#include <memory>
#include "afxpropertygridctrl.h"
#include "afxcmn.h"
#include "afxvslistbox.h"
#include "EditListCtrl.h"
#include "ImageStore.h"
#include "afxwin.h"
#include "ProductList.h"
#include "CatelogDialog.h"
#include "Transmission.h"
#include "PanoDialog.h"
#include "CameraCalculator.h"
#include "vld.h"

// CImageMeasureDlg dialog
class CImageMeasureDlg : public CDialogEx {
// Construction
  public:
    CImageMeasureDlg(CWnd* pParent = NULL); // standard constructor
//  ~CImageMeasureDlg();
    std::shared_ptr<CImageStore> DataStore() const {
        return m_dataStore;
    }
    void DataStore(std::shared_ptr<CImageStore> val) {
        m_dataStore = val;
    }
// Dialog Data
    enum { IDD = IDD_IMAGEMEASURE_DIALOG };

    /*
        waiting two over-side-look images. 10 pano images. finish.

    */

  protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    void LoadMeasureImage(const TCHAR *imagePath);
    void LoadLastestImage();
    void ChangeSide();

// Implementation
  protected:
    HICON m_hIcon;
    CMenu *m_menu;
    CStatusBarCtrl *m_statBar;

    CProductListDialog *m_listDialog;
    CCatelogDialog *m_catelogDialog;
    CPanoDialog *m_panoDialog;
    CCameraCalculator m_calculator;
    int m_receivedCounter;
    std::shared_ptr<CImageStore> m_dataStore;
    std::shared_ptr<CTransmission> m_transmisson;
    std::vector<ImageMeasure::Line> m_clickHistory;
    std::map<int, int> m_catelogComboDict;
    std::map<int, int> m_measureListInfoDict;
    ImageMeasure::Product m_currentProduct;
    std::vector<ImageMeasure::Line> m_measureLines;
    std::vector<ImageMeasure::MeasureInfo> m_measureInfoVector;

    int m_measureInfoSize;
    int m_currentSide;
    bool m_hasProduct;

    enum MeasureDrawState {
        WAIT_FIRST_POINT = 0,
        WAIT_SECOND_POINT
    };

    MeasureDrawState m_measureDrawState;
    std::vector<CPoint> m_measurePoints;
    // Generated message map functions
    virtual BOOL OnInitDialog();
    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    afx_msg void OnDestroy();
    afx_msg LRESULT OnUserInfoMsg(WPARAM wp, LPARAM lp);

    DECLARE_MESSAGE_MAP()
  public:

    CEditListCtrl m_measureHistory;
    afx_msg void OnBnClickedTransmissionButton();

    CButton m_transmissionButton;
    afx_msg void OnFileLoad();

    void UpdateCatelogComboBox(std::map<int, ImageMeasure::Catelog> &catelogDict);

    afx_msg void OnCatelogMenu();
    afx_msg void OnBnClickedDeleteDataButton();
    afx_msg void OnBnClickedMeasureBeforeButton();
    afx_msg void OnBnClickedMeasureNextButton();
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    BOOL CImageMeasureDlg::PreTranslateMessage(MSG *pMsg);

    CEdit m_productName;
    CStatic m_productWeight;
    CStatic m_productSize;
    afx_msg void OnBnClickedSaveDataButton();
    void LoadProductToWnd(ImageMeasure::Product &product);
    CComboBox m_catelogComboBox;
    afx_msg void OnBnClickedClearlineButton();
    afx_msg void OnBnClickedPanoButton();
    CStatic m_measurePicture;
    afx_msg void OnBnClickedSaveMeasureInfoButton();
    afx_msg void OnBnClickedButtonSide();
    CButton m_changeSideButton;
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedButtonCvtest();
};
