#pragma once
#include <vector>
#include "CppSQLite3.h"
#include "Types.h"
#include <map>

class CImageStore {
  public:
    CImageStore(void);
    ~CImageStore(void);

    void OpenDatabase();
    void CloseDatabase();
    void Test();
    void GetAllProduct(std::map<int, ImageMeasure::Product> &productDict);
    void UpdateProduct(const std::vector<ImageMeasure::Product> &productVector);
    void UpdateProduct(const ImageMeasure::Product &productVector);
    void InsertProduct(const std::vector<ImageMeasure::Product> &productVector);
    void InsertProduct(const ImageMeasure::Product &product);
    void GetAllCateLog(std::map<int, ImageMeasure::Catelog> &catelogDict);
    void UpdateCatelog(const std::vector<ImageMeasure::Catelog> &catelogVector);
    void UpdateCatelog(const ImageMeasure::Catelog &catelogVector);
    void InsertCatelog(const std::vector<ImageMeasure::Catelog> &catelog);
    void InsertCatelog(const ImageMeasure::Catelog &catelog);
    void DeleteProduct(int id);
    void DeleteCatelog(int id);
    void UpdateProductAndCateLog();

    std::map<int, ImageMeasure::Product> ProductDict() const {
        return m_productDict;
    }
    void ProductDict(std::map<int, ImageMeasure::Product> val) {
        m_productDict = val;
    }
    std::map<int, ImageMeasure::Catelog> CatelogDict() const {
        return m_catelogDict;
    }
    void CatelogDict(std::map<int, ImageMeasure::Catelog> val) {
        m_catelogDict = val;
    }
    bool Connected() const {
        return m_isConnected;
    }
    void Connected(bool val) {
        m_isConnected = val;
    }

    void LoadProductAndCatelog();
    void InsertMeasureInfo(std::vector<ImageMeasure::MeasureInfo> &infos);
    void InsertMeasureInfo(ImageMeasure::MeasureInfo &infos);
    void DeleteMeasureInfo(int id);
    bool GetProductById(const int id, ImageMeasure::Product &product);

  protected:

  protected:
    CppSQLite3DB m_db;
    bool m_isConnected;

    char* m_dbPath;

    std::map<int, ImageMeasure::Product> m_productDict;

    std::map<int, ImageMeasure::Catelog> m_catelogDict;

};

