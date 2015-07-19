// ProductList.cpp : implementation file
//

#include "stdafx.h"
#include "ImageMeasure.h"
#include "ProductList.h"
#include "afxdialogex.h"
#include <vector>
#include "Types.h"


// CProductList dialog

IMPLEMENT_DYNAMIC(CProductListDialog, CDialogEx)

CProductListDialog::CProductListDialog(CWnd* pParent /*=NULL*/)
    : CDialogEx(CProductListDialog::IDD, pParent) {

}

CProductListDialog::~CProductListDialog() {
}

void CProductListDialog::DoDataExchange(CDataExchange* pDX) {
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_DEMO_PICTURE, m_demoPicture);
    DDX_Control(pDX, IDC_PRODUCT_LIST, m_productList);
}


BEGIN_MESSAGE_MAP(CProductListDialog, CDialogEx)
    ON_WM_PAINT()
    ON_BN_CLICKED(IDC_OPEN_LIST_BUTTON, &CProductListDialog::OnBnClickedOpenListButton)
    ON_BN_CLICKED(IDC_DELETE_LIST_BUTTON, &CProductListDialog::OnBnClickedDeleteListButton)
    ON_NOTIFY(NM_CLICK, IDC_PRODUCT_LIST, &CProductListDialog::OnNMClickProductList)
    ON_NOTIFY(NM_DBLCLK, IDC_PRODUCT_LIST, &CProductListDialog::OnNMDblclkProductList)
END_MESSAGE_MAP()

void CProductListDialog::OnPaint() {

}

// CProductList message handlers

BOOL CProductListDialog::OnInitDialog() {
    CDialogEx::OnInitDialog();
    m_productListIdDict.clear();
    CRect rect;
    this->m_productList.GetClientRect(&rect);

    this->m_productList.SetExtendedStyle(m_productList.GetExtendedStyle() | LVS_EX_ONECLICKACTIVATE | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP);
    this->m_productList.InsertColumn(0, _T("序号"), LVCFMT_CENTER, rect.Width() * 2 / 10, 0);
    this->m_productList.InsertColumn(1, _T("名称"), LVCFMT_CENTER, rect.Width() * 4 / 10, 1);
    this->m_productList.InsertColumn(2, _T("类别"), LVCFMT_CENTER, rect.Width() * 4 / 10, 2);

    m_productDict.clear();
    this->m_imageStore->GetAllProduct(m_productDict);

    int counter = 0;
    for (auto i = m_productDict.begin(); i != m_productDict.end(); i++) {
        auto &product = i->second;
        int id = i->first;

        auto &catelog = product.type;
        TCHAR productID[2048];
        swprintf_s(productID, 2048, L"%d", counter + 1);
        m_productListIdDict[counter] = product.id;

        this->m_productList.InsertItem(counter, _T(""));
        this->m_productList.SetItemText(counter, 0, productID);
        this->m_productList.SetItemText(counter, 1, product.name.c_str());
        if (catelog.id == -1) {
            catelog.name = L"未分类";
        }
        this->m_productList.SetItemText(counter, 2, catelog.name.c_str());
        counter++;
    }

    return TRUE;
}

void CProductListDialog::OnBnClickedOpenListButton() {
    if (m_productDict.size() == 0) {
        this->m_chooseProductId = -1;

        this->EndDialog(0);
        return;
    }

    int row = this->m_productList.GetSelectionMark();
    if (row == -1) {
        AfxMessageBox(_T("请选择一个产品"));
    } else {
        this->m_chooseProductId = m_productListIdDict[row];
        this->EndDialog(0);
        return;
    }
}


void CProductListDialog::OnBnClickedDeleteListButton() {
    int row = this->m_productList.GetSelectionMark();
    if (row == -1) {
        return;
    }

    if (::MessageBox(this->m_hWnd, L"确认删除此产品？\n", L"删除", MB_OKCANCEL) == IDOK) {
        this->m_productList.DeleteItem(row);
        int pid = m_productListIdDict[row];
        this->m_imageStore->DeleteProduct(pid);
    }
}

void CProductListDialog::setImageStore(std::shared_ptr<CImageStore> imageStore) {
    this->m_imageStore = imageStore;
}
void CProductListDialog::LoadMeasureImage(const TCHAR *imagePath) {
    if (_waccess(imagePath, 0) == -1) {
//        AfxMessageBox(L"图像丢失！");
        return;
    }

    CImage myImage;
    myImage.Load(imagePath);

    CRect rect;
    CWnd *pWnd = GetDlgItem(IDC_DEMO_PICTURE);
    CDC *pDC = pWnd->GetDC();
    pWnd->GetClientRect(&rect);
    pDC->SetStretchBltMode(STRETCH_HALFTONE);
    myImage.Draw(pDC->m_hDC, rect);
    ReleaseDC(pDC);
    myImage.Destroy();
}


void CProductListDialog::OnNMClickProductList(NMHDR *pNMHDR, LRESULT *pResult) {
    LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
    CString itemContent = this->m_productList.GetItemText(pNMItemActivate->iItem, 0);
    if (itemContent.GetLength() > 0) {
        int pid = this->m_productListIdDict[pNMItemActivate->iItem];
        auto &product = m_productDict[pid];
        LoadMeasureImage(product.overlookPathMin.c_str());
    }
    *pResult = 0;
}


void CProductListDialog::OnNMDblclkProductList(NMHDR *pNMHDR, LRESULT *pResult) {
    LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
    this->m_chooseProductId = m_productListIdDict[pNMItemActivate->iItem];
    *pResult = 0;
    CString itemContent = this->m_productList.GetItemText(pNMItemActivate->iItem, 0);
    if (itemContent.GetLength() > 0) {
        this->EndDialog(0);
    }
}
