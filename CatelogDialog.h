#pragma once
#include "afxcmn.h"
#include "EditListCtrl.h"
#include <memory>
#include <map>
#include "ImageStore.h"


// CCatelogDialob dialog

class CCatelogDialog : public CDialogEx {
    DECLARE_DYNAMIC(CCatelogDialog)

  public:
    CCatelogDialog(CWnd* pParent = NULL);   // standard constructor
    virtual ~CCatelogDialog();
    void setImageStore(std::shared_ptr<CImageStore> imageStore);

// Dialog Data
    enum { IDD = IDD_CATELOG_DIALOG };

  protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    std::shared_ptr<CImageStore> m_imageStore;
    std::map<int, ImageMeasure::Catelog> m_catelogDict;
    std::map<int, int> m_listToCatelogIdDict;
    DECLARE_MESSAGE_MAP()
  public:
    CEditListCtrl m_catelogList;
    int m_catelogSize;
    afx_msg void OnBnClickedCatelogSaveButton();

    virtual BOOL OnInitDialog() override;

    afx_msg void OnBnClickedCatelogAddButton();
    afx_msg void OnBnClickedCatelogDeleteButton();
};
