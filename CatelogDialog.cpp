// CatelogDialob.cpp : implementation file
//

#include "stdafx.h"
#include "ImageMeasure.h"
#include "CatelogDialog.h"
#include "afxdialogex.h"
#include <vector>
#include "Types.h"
#include "strtools.hpp"


// CCatelogDialob dialog

IMPLEMENT_DYNAMIC(CCatelogDialog, CDialogEx)

CCatelogDialog::CCatelogDialog(CWnd* pParent /*=NULL*/)
    : CDialogEx(CCatelogDialog::IDD, pParent) {
}

CCatelogDialog::~CCatelogDialog() {
}

void CCatelogDialog::DoDataExchange(CDataExchange* pDX) {
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_CATELOG_LIST, m_catelogList);
}


BEGIN_MESSAGE_MAP(CCatelogDialog, CDialogEx)
    ON_BN_CLICKED(IDC_CATELOGSAVE_BUTTON, &CCatelogDialog::OnBnClickedCatelogSaveButton)
    ON_BN_CLICKED(IDC_CATELOG_ADD_BUTTON, &CCatelogDialog::OnBnClickedCatelogAddButton)
    ON_BN_CLICKED(IDC_CATELOG_DELETE_BUTTON, &CCatelogDialog::OnBnClickedCatelogDeleteButton)
END_MESSAGE_MAP()


// CCatelogDialob message handlers


void CCatelogDialog::OnBnClickedCatelogSaveButton() {
    int count = this->m_catelogList.GetItemCount();
    for (int i = 0; i < count; i++) {
        ImageMeasure::Catelog catelog;

        CString idString = this->m_catelogList.GetItemText(i, 0);

        const wchar_t *wccId = idString.GetBuffer();
        char *idChar = (char *)malloc(sizeof(char)*(2 * wcslen(wccId) + 1));
        memset(idChar, 0, 2 * wcslen(wccId) + 1);
        strtool::w2c(idChar, wccId, 2 * wcslen(wccId) + 1);

        int catelogId = atol(idChar);

        catelogId = m_listToCatelogIdDict[catelogId];

        catelog.id = catelogId;
        catelog.name = this->m_catelogList.GetItemText(i, 1).GetBuffer();
        catelog.desc = this->m_catelogList.GetItemText(i, 2).GetBuffer();

        if (catelog.id == -1) {
            this->m_imageStore->InsertCatelog(catelog);
        }
        else {
            this->m_imageStore->UpdateCatelog(catelog);
        }

        free(idChar);
		idChar = nullptr;
    }
}

BOOL CCatelogDialog::OnInitDialog() {
    CDialogEx::OnInitDialog();

    CRect rect;
    this->m_catelogList.GetClientRect(&rect);

    this->m_catelogList.SetExtendedStyle(m_catelogList.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP | LVS_EX_TWOCLICKACTIVATE);
    this->m_catelogList.InsertColumn(0, _T("序号"), LVCFMT_CENTER, rect.Width() * 2 / 10, 0);
    this->m_catelogList.InsertColumn(1, _T("名称"), LVCFMT_CENTER, rect.Width() * 4 / 10, 1);
    this->m_catelogList.InsertColumn(2, _T("备注"), LVCFMT_CENTER, rect.Width() * 4 / 10, 2);
    this->m_catelogList.SetNoEditCol(0);

    this->m_imageStore->GetAllCateLog(m_catelogDict);
    this->m_catelogSize = m_catelogDict.size();
    m_listToCatelogIdDict.clear();
    int counter = 0;
    for (auto i = this->m_catelogDict.begin(); i != this->m_catelogDict.end(); i++) {
        auto &catelog = i->second;
        m_listToCatelogIdDict[counter + 1] = catelog.id;
        int id = i->first;
        TCHAR catelogID[2048];
        swprintf_s(catelogID, 2048, L"%d", counter + 1);

        this->m_catelogList.InsertItem(counter, _T(""));
        this->m_catelogList.SetItemText(counter, 0, catelogID);
        this->m_catelogList.SetItemText(counter, 1, catelog.name.c_str());
        this->m_catelogList.SetItemText(counter, 2, catelog.desc.c_str());
        counter++;
    }

    return TRUE;
}

void CCatelogDialog::setImageStore(std::shared_ptr<CImageStore> imageStore) {
    this->m_imageStore = imageStore;
}

void CCatelogDialog::OnBnClickedCatelogAddButton() {
    this->m_catelogSize++;
    TCHAR catelogID[2048];
    swprintf_s(catelogID, 2048, L"%d", this->m_catelogSize);

    this->m_catelogList.InsertItem(this->m_catelogSize - 1, _T(""));
    this->m_catelogList.SetItemText(this->m_catelogSize - 1, 0, catelogID);
    this->m_catelogList.SetItemText(this->m_catelogSize - 1, 1, _T("名称"));
    this->m_catelogList.SetItemText(this->m_catelogSize - 1, 2, _T("详情"));
    m_listToCatelogIdDict[this->m_catelogSize] = -1;
}


void CCatelogDialog::OnBnClickedCatelogDeleteButton() {
    int row = this->m_catelogList.GetSelectionMark();
    if (row == -1) {
        return;
    }

    if (::MessageBox(this->m_hWnd, L"确认删除此类别？\n", L"删除", MB_OKCANCEL) == IDOK) {
        CString idString = this->m_catelogList.GetItemText(row, 0);

        const wchar_t *wccId = idString.GetBuffer();
        char *idChar = (char *)malloc(sizeof(char)*(2 * wcslen(wccId) + 1));
        memset(idChar, 0, 2 * wcslen(wccId) + 1);
        strtool::w2c(idChar, wccId, 2 * wcslen(wccId) + 1);

        int catelogId = atol(idChar);
        catelogId = m_listToCatelogIdDict[catelogId];
        this->m_imageStore->DeleteCatelog(catelogId);

        this->m_catelogList.DeleteItem(row);
        this->m_catelogSize--;

        m_listToCatelogIdDict.erase(catelogId);

        free(idChar);
		idChar = nullptr;
    }
}
