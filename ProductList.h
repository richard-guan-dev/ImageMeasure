#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#include <memory>
#include <map>
#include "ImageStore.h"


// CProductList dialog

class CProductListDialog : public CDialogEx {
    DECLARE_DYNAMIC(CProductListDialog)

  public:
    CProductListDialog(CWnd* pParent = NULL);   // standard constructor
    virtual ~CProductListDialog();

    void setImageStore(std::shared_ptr<CImageStore> imageStore);
    int ChooseProductId() const {
        return m_chooseProductId;
    }
    void ChooseProductId(int val) {
        m_chooseProductId = val;
    }

// Dialog Data
    enum { IDD = IDD_PRODUCTLIST_DIALOG };

  protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL OnInitDialog();
    std::shared_ptr<CImageStore> m_imageStore;
    std::map<int, int> m_productListIdDict;
    int m_chooseProductId;
	
    std::map<int, ImageMeasure::Product> m_productDict;

    DECLARE_MESSAGE_MAP()
  public:
    CStatic m_demoPicture;
    CListCtrl m_productList;
    afx_msg void OnBnClickedOpenListButton();
    afx_msg void OnBnClickedDeleteListButton();
    afx_msg void OnPaint();
	afx_msg void OnNMClickProductList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclkProductList(NMHDR *pNMHDR, LRESULT *pResult);
	void LoadMeasureImage(const TCHAR *imagePath);
};
