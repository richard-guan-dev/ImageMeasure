#pragma once
#include "ImageStore.h"
#include "Types.h"
#include <map>
#include <memory>

// CPanoDialog dialog

class CPanoDialog : public CDialogEx {
    DECLARE_DYNAMIC ( CPanoDialog )

  public:
    CPanoDialog ( CWnd* pParent = NULL ); // standard constructor
    virtual ~CPanoDialog();
    int CurrentProductId() const {
        return m_currentProductId;
    }
    void CurrentProductId ( int val ) {
        m_currentProductId = val;
    }

    void setImageStore ( std::shared_ptr<CImageStore>  val ) {
        m_dataStore = val;
    }
// Dialog Data
    enum { IDD = IDD_PANO_DIALOG };

  protected:
    virtual void DoDataExchange ( CDataExchange* pDX ); // DDX/DDV support
    int m_currentProductId;
    ImageMeasure::Product m_currentProduct;
    std::shared_ptr<CImageStore> m_dataStore;
    int m_currentPanoId;

    DECLARE_MESSAGE_MAP()

    virtual BOOL OnInitDialog() override;
    afx_msg void OnPaint();
    afx_msg void CPanoDialog::OnKeyDown ( UINT nChar, UINT nRepCnt, UINT nFlags );
    BOOL CPanoDialog::PreTranslateMessage ( MSG* pMsg );

    void LoadMeasureImage ( const TCHAR* imagePath );
    LRESULT OnUserInfoMsg ( WPARAM wp, LPARAM lp );
  public:
    afx_msg void OnBnClickedPanoBeforeButton();
    afx_msg void OnBnClickedPanoNextButton();
};
