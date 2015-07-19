#include "StdAfx.h"
#include "ImageStore.h"
#include <string>
#include "strtools.hpp"
#include "Types.h"

CImageStore::CImageStore ( void ) : m_isConnected ( false ), m_dbPath ( NULL ) {
}

CImageStore::~CImageStore ( void ) {
    this->CloseDatabase();
}

void CImageStore::DeleteProduct ( int pid ) {
    if ( !this->m_isConnected ) {
        this->OpenDatabase();
    }

    char buff[1024];
    sprintf_s ( buff, 1024, "delete from product where pid = %d", pid );
    int result = this->m_db.execDML ( buff );
}

void CImageStore::DeleteCatelog ( int id ) {
    if ( !this->m_isConnected ) {
        this->OpenDatabase();
    }

    char buff[1024];
    sprintf_s ( buff, 1024, "delete from product_type where id = %d", id );
    int result = this->m_db.execDML ( buff );

    sprintf_s ( buff, 1024, "select * from product where type_id = %d", id );
    CppSQLite3Query query = this->m_db.execQuery ( buff );
    while ( !query.eof() ) {
        int pid = query.getIntField ( "pid" );
        sprintf_s ( buff, 1024, "update product set type_id = -1 where pid = %d", pid );
        int result = this->m_db.execDML ( buff );

        query.nextRow();
    }
    query.finalize();
}

void CImageStore::GetAllCateLog ( std::map<int, ImageMeasure::Catelog> &catelogDict ) {
    if ( !this->m_isConnected ) {
        this->OpenDatabase();
    }

    this->UpdateProductAndCateLog();
    catelogDict = this->m_catelogDict;
}

void CImageStore::GetAllProduct ( std::map<int, ImageMeasure::Product> &productDict ) {
    if ( !this->m_isConnected ) {
        this->OpenDatabase();
    }

    this->UpdateProductAndCateLog();
    productDict = this->m_productDict;
}

void CImageStore::UpdateCatelog ( const std::vector<ImageMeasure::Catelog> &catelogVector ) {
    for ( int i = 0; i < catelogVector.size(); i++ ) {
        auto &catelog = catelogVector[i];
        UpdateCatelog ( catelog );
    }
}

void CImageStore::UpdateCatelog ( const ImageMeasure::Catelog &catelog ) {
    if ( !this->m_isConnected ) {
        this->OpenDatabase();
    }

    if ( catelog.id == -1 ) {
        return;
    }

    const wchar_t* wccName = catelog.name.c_str();
    char* nameChar = ( char* ) malloc ( sizeof ( char ) * ( 2 * wcslen ( wccName ) + 1 ) );
    memset ( nameChar, 0, 2 * wcslen ( wccName ) + 1 );
    strtool::w2c ( nameChar, wccName, 2 * wcslen ( wccName ) + 1 );

    const wchar_t* wccDesc = catelog.desc.c_str();
    char* descChar = ( char* ) malloc ( sizeof ( char ) * ( 2 * wcslen ( wccDesc ) + 1 ) );
    memset ( descChar, 0, 2 * wcslen ( wccDesc ) + 1 );
    strtool::w2c ( descChar, wccDesc, 2 * wcslen ( wccDesc ) + 1 );

    char buff[2048];
    sprintf_s ( buff, 2048, "update product_type set name = '%s', desc = '%s' where id = %d", nameChar, descChar, catelog.id );
    int result = this->m_db.execDML ( buff );

    free ( nameChar );
    free ( descChar );
    descChar = nullptr;
    nameChar = nullptr;
}

void CImageStore::UpdateProduct ( const std::vector<ImageMeasure::Product> &productVector ) {
    for ( int i = 0; i < productVector.size(); i++ ) {
        auto &product = productVector[i];
        UpdateProduct ( product );
    }
}

void CImageStore::UpdateProduct ( const ImageMeasure::Product &product ) {
    if ( !this->m_isConnected ) {
        this->OpenDatabase();
    }

    if ( product.id == -1 ) {
        return;
    }

    const wchar_t* wccName = product.name.c_str();
    char* nameChar = ( char* ) malloc ( sizeof ( char ) * ( 2 * wcslen ( wccName ) + 1 ) );
    memset ( nameChar, 0, 2 * wcslen ( wccName ) + 1 );
    strtool::w2c ( nameChar, wccName, 2 * wcslen ( wccName ) + 1 );

    char buff[2048];
    sprintf_s ( buff, 2048, "update product set name = '%s', type_id = %d, length = %.2lf, height = %.2lf, width = %.2lf, weight = %.2lf where pid = %d", nameChar, product.type.id, product.length, product.height, product.width, product.weight, product.id );
    int result = this->m_db.execDML ( buff );
    free ( nameChar );
    nameChar = nullptr;
}

void CImageStore::InsertProduct ( const std::vector<ImageMeasure::Product> &productVector ) {
    for ( int i = 0; i < productVector.size(); i++ ) {
        auto &product = productVector[i];
        this->InsertProduct ( product );
    }
}

void CImageStore::InsertProduct ( const ImageMeasure::Product &product ) {
    const wchar_t* wccName = product.name.c_str();
    char* nameChar = ( char* ) malloc ( sizeof ( char ) * ( 2 * wcslen ( wccName ) + 1 ) );
    memset ( nameChar, 0, 2 * wcslen ( wccName ) + 1 );
    strtool::w2c ( nameChar, wccName, 2 * wcslen ( wccName ) + 1 );

    char buff[2048];
    sprintf_s ( buff, 2048, "insert into product('pid', 'type_id', 'name', 'length', 'height', 'width', 'weight', 'is_height') values(NULL, -1, '%s', %.3lf, %.3lf, %.3lf, %.3lf, %d)", nameChar, product.length, product.height, product.width, product.weight, ( int ) product.isHeight );
    free ( nameChar );
    nameChar = nullptr;

    if ( !this->m_isConnected ) {
        this->OpenDatabase();
    }

    int result = this->m_db.execDML ( buff );
    long long lastPID = this->m_db.lastRowId();

    for ( int i = 0; i < product.measureImages.size(); i++ ) {
        auto& image = product.measureImages[i];

        const wchar_t* wccPath = image.imagePath.c_str();
        char* pathChar = ( char* ) malloc ( sizeof ( char ) * ( 2 * wcslen ( wccPath ) + 1 ) );
        memset ( pathChar, 0, 2 * wcslen ( wccPath ) + 1 );
        strtool::w2c ( pathChar, wccPath, 2 * wcslen ( wccPath ) + 1 );

        int canMeasure = 0;
        if ( image.canMeasure ) {
            canMeasure = 1;
        }

        char buff[2048];
        sprintf_s ( buff, 2048, "insert into measure('id', 'pid', 'image_type', 'image_path', 'weight', 'can_measure') values(NULL, %lld, %d, '%s', %.3lf, %d)", lastPID, image.imageType, pathChar, image.weight, canMeasure );
        free ( pathChar );
        pathChar = nullptr;
        int result = this->m_db.execDML ( buff );
    }

    for ( int i = 0; i < product.panoPath.size(); i++ ) {
        auto &panoPath = product.panoPath[i];
        int id = 0;

        if ( product.isReverse ) {
            id = product.panoPath.size() - id - 1;
        }

        const wchar_t* wccPath = panoPath.c_str();
        char* pathChar = ( char* ) malloc ( sizeof ( char ) * ( 2 * wcslen ( wccPath ) + 1 ) );
        memset ( pathChar, 0, 2 * wcslen ( wccPath ) + 1 );
        strtool::w2c ( pathChar, wccPath, 2 * wcslen ( wccPath ) + 1 );

        char buff[2048];
        sprintf_s ( buff, 2048, "insert into pano('id', 'pid', 'image_id', 'image_path') values(NULL, %lld, %d, '%s')", lastPID, id, pathChar );
        free ( pathChar );
        pathChar = nullptr;
        int result = this->m_db.execDML ( buff );
    }
}

void CImageStore::InsertCatelog ( const std::vector<ImageMeasure::Catelog> &catelogVector ) {
    for ( int i = 0; i < catelogVector.size(); i++ ) {
        auto &catelog = catelogVector[i];
        this->InsertCatelog ( catelog );
    }
}

void CImageStore::InsertCatelog ( const ImageMeasure::Catelog &catelog ) {
    if ( !this->m_isConnected ) {
        this->OpenDatabase();
    }

    const wchar_t* wccName = catelog.name.c_str();
    char* nameChar = ( char* ) malloc ( sizeof ( char ) * ( 2 * wcslen ( wccName ) + 1 ) );
    memset ( nameChar, 0, 2 * wcslen ( wccName ) + 1 );
    strtool::w2c ( nameChar, wccName, 2 * wcslen ( wccName ) + 1 );

    const wchar_t* wccDesc = catelog.desc.c_str();
    char* descChar = ( char* ) malloc ( sizeof ( char ) * ( 2 * wcslen ( wccDesc ) + 1 ) );
    memset ( descChar, 0, 2 * wcslen ( wccDesc ) + 1 );
    strtool::w2c ( descChar, wccDesc, 2 * wcslen ( wccDesc ) + 1 );

    char buff[2048];
    sprintf_s ( buff, 2048, "insert into product_type('name', 'desc') values('%s', '%s')", nameChar, descChar );
    int result = this->m_db.execDML ( buff );

    free ( descChar );
    free ( nameChar );
    descChar = nullptr;
    nameChar = nullptr;
}

void CImageStore::OpenDatabase() {
    if ( this->m_isConnected ) {
        return;
    }

    if ( this->m_dbPath == NULL ) {
        TCHAR szPath[MAX_PATH];
        if ( !GetModuleFileName ( NULL, szPath, MAX_PATH ) ) {
            return;
        }

        std::wstring imageDatabasePath = strtool::getfilePathW ( szPath );
        imageDatabasePath += L"\\db\\ImageDB.db";

        const wchar_t* wcc = imageDatabasePath.c_str();
        this->m_dbPath = ( char* ) malloc ( sizeof ( char ) * ( 2 * wcslen ( wcc ) + 1 ) );
        memset ( this->m_dbPath, 0, 2 * wcslen ( wcc ) + 1 );
        strtool::w2c ( this->m_dbPath, wcc, 2 * wcslen ( wcc ) + 1 );
    }

    this->m_isConnected = false;

    try {
        this->m_db.open ( this->m_dbPath );
        this->m_isConnected = true;
    } catch ( CppSQLite3Exception e ) {
        this->m_isConnected = false;
    }

//    LoadProductAndCatelog();
    //DEBUG
//    this->Test();
}

void CImageStore::Test() {

    CppSQLite3Query query = this->m_db.execQuery ( "select * from target_table" );
    while ( !query.eof() ) {
        std::string name ( query.getStringField ( "name" ) );
        AfxMessageBox ( _T ( "" ) );
        query.nextRow();
    }
    query.finalize();
}

void CImageStore::CloseDatabase() {
    if ( !this->m_isConnected ) {
        return;
    }

    try {
        this->m_db.close();
    } catch ( CppSQLite3Exception e ) {
    }

    if ( this->m_dbPath != nullptr ) {
        free ( this->m_dbPath );
        this->m_dbPath = nullptr;
    }

    this->m_isConnected = false;
}

bool CImageStore::GetProductById(const int id, ImageMeasure::Product &product) {
    if (!this->m_isConnected) {
        this->OpenDatabase();
    }

    {
        char buff[1024];
        sprintf_s(buff, 1024, "select * from product where pid=%d", id);

        CppSQLite3Query query = this->m_db.execQuery(buff);
        if (query.eof()) {
            return false;
        }

        while (!query.eof()) {
            product.id = query.getIntField("pid");
            std::string name(query.getStringField("name"));
            product.name = strtool::stringToWstring(name);
            product.length = query.getFloatField("length");
            product.width = query.getFloatField("width");
            product.height = query.getFloatField("height");
            product.weight = query.getFloatField("weight");
            product.isHeight = query.getIntField("is_height");

            int catelogId = query.getIntField("type_id");
            ImageMeasure::Catelog catelog;

            if (catelogId != -1) {
                sprintf_s(buff, 1024, "select * from product_type where id = %d", catelogId);

                this->m_productDict[product.id] = product;
                CppSQLite3Query catelogQuery = this->m_db.execQuery(buff);
                while (!catelogQuery.eof()) {
                    catelog.id = catelogQuery.getIntField("id");
                    std::string name(catelogQuery.getStringField("name"));
                    std::string desc(catelogQuery.getStringField("desc"));
                    catelog.name = strtool::stringToWstring(name);
                    catelog.desc = strtool::stringToWstring(desc);

                    catelogQuery.nextRow();
                }

                catelogQuery.finalize();
            } else {
                catelog.id = -1;
                catelog.name = L"";
                catelog.desc = L"";
            }

            product.type = catelog;
            sprintf_s(buff, 1024, "select * from pano where pid = %d order by image_id asc", product.id);
            CppSQLite3Query panoQuery = this->m_db.execQuery(buff);
            while (!panoQuery.eof()) {
                std::string path(panoQuery.getStringField("image_path"));
                product.panoPath.push_back(strtool::stringToWstring(path));

                panoQuery.nextRow();
            }
            panoQuery.finalize();

            sprintf_s(buff, 1024, "select * from measure where pid = %d", product.id);
            CppSQLite3Query measureQuery = this->m_db.execQuery(buff);
            while (!measureQuery.eof()) {
                ImageMeasure::MeasureImage measureImage;
                std::string path(measureQuery.getStringField("image_path"));
                measureImage.imagePath = strtool::stringToWstring(path);
                measureImage.id = measureQuery.getIntField("id");
                measureImage.imageType = measureQuery.getIntField("image_type");
                measureImage.weight = measureQuery.getFloatField("weight");
                int isMeasureable = measureQuery.getIntField("can_measure");
                if (isMeasureable != -1) {
                    if (isMeasureable == 0) {
                        measureImage.canMeasure = false;
                    } else {
                        measureImage.canMeasure = true;
                    }
                } else {
                    measureImage.canMeasure = false;
                }
                product.measureImages.push_back(measureImage);

                measureQuery.nextRow();
            }
            measureQuery.finalize();

            product.overlookPath1 = product.measureImages[0].imagePath;
            product.sidelookPath1 = product.measureImages[1].imagePath;
            product.overlookPath2 = product.measureImages[2].imagePath;
            product.sidelookPath2 = product.measureImages[3].imagePath;

            if (product.measureImages[0].canMeasure) {
                product.overlookPathMin = product.measureImages[0].imagePath;
                product.sidelookPathMin = product.measureImages[1].imagePath;
            } else {
                product.overlookPathMin = product.measureImages[2].imagePath;
                product.sidelookPathMin = product.measureImages[3].imagePath;
            }


            sprintf_s(buff, 1024, "select * from product_measure_info where pid = %d", product.id);
            CppSQLite3Query measureInfoQuery = this->m_db.execQuery(buff);
            while (!measureInfoQuery.eof()) {
                ImageMeasure::MeasureInfo info;
                info.pid = measureInfoQuery.getIntField("pid");
                info.id = measureInfoQuery.getIntField("id");
                std::string name(measureInfoQuery.getStringField("name"));
                std::string desc(measureInfoQuery.getStringField("desc"));
                info.name = strtool::stringToWstring(name);
                info.desc = strtool::stringToWstring(desc);
                info.start.x = measureInfoQuery.getIntField("start_x");
                info.start.y = measureInfoQuery.getIntField("start_y");
                info.end.x = measureInfoQuery.getIntField("end_x");
                info.end.y = measureInfoQuery.getIntField("end_y");
                info.side = measureInfoQuery.getIntField("side");
                info.length = -1;

                product.measureInfo.push_back(info);
                measureInfoQuery.nextRow();
            }
            measureInfoQuery.finalize();

            this->m_productDict[product.id] = product;
            query.nextRow();
        }
        query.finalize();
    }

    return true;
}

void CImageStore::LoadProductAndCatelog() {
    if ( !this->m_isConnected ) {
        this->OpenDatabase();
    }

    {
        char buff[1024];
        sprintf_s ( buff, 1024, "select * from product_type where 1" );

        CppSQLite3Query query = this->m_db.execQuery ( buff );
        while ( !query.eof() ) {
            ImageMeasure::Catelog catelog;
            catelog.id = query.getIntField ( "id" );
            std::string name ( query.getStringField ( "name" ) );
            std::string desc ( query.getStringField ( "desc" ) );
            catelog.name = strtool::stringToWstring ( name );
            catelog.desc = strtool::stringToWstring ( desc );

            this->m_catelogDict[catelog.id] = catelog;

            query.nextRow();
        }

        query.finalize();
    }

    {
        char buff[1024];
        sprintf_s ( buff, 1024, "select * from product where 1" );

        CppSQLite3Query query = this->m_db.execQuery ( buff );
        while ( !query.eof() ) {
            ImageMeasure::Product product;
            product.id = query.getIntField ( "pid" );
            std::string name ( query.getStringField ( "name" ) );
            product.name = strtool::stringToWstring ( name );
            product.length = query.getFloatField ( "length" );
            product.width = query.getFloatField ( "width" );
            product.height = query.getFloatField ( "height" );
            product.weight = query.getFloatField ( "weight" );
            product.isHeight = query.getIntField ( "is_height" );

            int catelogId = query.getIntField ( "type_id" );
            ImageMeasure::Catelog catelog;

            if ( catelogId != -1 ) {
                sprintf_s ( buff, 1024, "select * from product_type where id = %d", catelogId );

                this->m_productDict[product.id] = product;
                CppSQLite3Query catelogQuery = this->m_db.execQuery ( buff );
                while ( !catelogQuery.eof() ) {
                    catelog.id = catelogQuery.getIntField ( "id" );
                    std::string name ( catelogQuery.getStringField ( "name" ) );
                    std::string desc ( catelogQuery.getStringField ( "desc" ) );
                    catelog.name = strtool::stringToWstring ( name );
                    catelog.desc = strtool::stringToWstring ( desc );

                    catelogQuery.nextRow();
                }

                catelogQuery.finalize();
            } else {
                catelog.id = -1;
                catelog.name = L"";
                catelog.desc = L"";
            }

            product.type = catelog;
            sprintf_s ( buff, 1024, "select * from pano where pid = %d order by image_id asc", product.id );
            CppSQLite3Query panoQuery = this->m_db.execQuery ( buff );
            while ( !panoQuery.eof() ) {
                std::string path ( panoQuery.getStringField ( "image_path" ) );
                product.panoPath.push_back ( strtool::stringToWstring ( path ) );

                panoQuery.nextRow();
            }
            panoQuery.finalize();

            sprintf_s ( buff, 1024, "select * from measure where pid = %d", product.id );
            CppSQLite3Query measureQuery = this->m_db.execQuery ( buff );
            while ( !measureQuery.eof() ) {
                ImageMeasure::MeasureImage measureImage;
                std::string path ( measureQuery.getStringField ( "image_path" ) );
                measureImage.imagePath = strtool::stringToWstring ( path );
                measureImage.id = measureQuery.getIntField ( "id" );
                measureImage.imageType = measureQuery.getIntField ( "image_type" );
                measureImage.weight = measureQuery.getFloatField ( "weight" );
                int isMeasureable = measureQuery.getIntField ( "can_measure" );
                if ( isMeasureable != -1 ) {
                    if ( isMeasureable == 0 ) {
                        measureImage.canMeasure = false;
                    } else {
                        measureImage.canMeasure = true;
                    }
                } else {
                    measureImage.canMeasure = false;
                }
                product.measureImages.push_back ( measureImage );

                measureQuery.nextRow();
            }
            measureQuery.finalize();

//             if (product.measureImages[0].canMeasure) {
//                 product.overlookPath = product.measureImages[0].imagePath;
//                 product.sidelookPath = product.measureImages[1].imagePath;
//             } else {
//                 product.overlookPath = product.measureImages[2].imagePath;
//                 product.sidelookPath = product.measureImages[3].imagePath;
//             }

            product.overlookPath1 = product.measureImages[0].imagePath;
            product.sidelookPath1 = product.measureImages[1].imagePath;
            product.overlookPath2 = product.measureImages[2].imagePath;
            product.sidelookPath2 = product.measureImages[3].imagePath;

            if (product.measureImages[0].canMeasure) {
                product.overlookPathMin = product.measureImages[0].imagePath;
                product.sidelookPathMin = product.measureImages[1].imagePath;
            } else {
                product.overlookPathMin = product.measureImages[2].imagePath;
                product.sidelookPathMin = product.measureImages[3].imagePath;
            }


            sprintf_s ( buff, 1024, "select * from product_measure_info where pid = %d", product.id );
            CppSQLite3Query measureInfoQuery = this->m_db.execQuery ( buff );
            while ( !measureInfoQuery.eof() ) {
                ImageMeasure::MeasureInfo info;
                info.pid = measureInfoQuery.getIntField ( "pid" );
                info.id = measureInfoQuery.getIntField ( "id" );
                std::string name ( measureInfoQuery.getStringField ( "name" ) );
                std::string desc ( measureInfoQuery.getStringField ( "desc" ) );
                info.name = strtool::stringToWstring ( name );
                info.desc = strtool::stringToWstring ( desc );
                info.start.x = measureInfoQuery.getIntField ( "start_x" );
                info.start.y = measureInfoQuery.getIntField ( "start_y" );
                info.end.x = measureInfoQuery.getIntField ( "end_x" );
                info.end.y = measureInfoQuery.getIntField ( "end_y" );
                info.side = measureInfoQuery.getIntField("side");
                info.length = -1;

                product.measureInfo.push_back ( info );
                measureInfoQuery.nextRow();
            }
            measureInfoQuery.finalize();

            this->m_productDict[product.id] = product;
            query.nextRow();
        }
        query.finalize();
    }
}

void CImageStore::UpdateProductAndCateLog() {
    this->m_productDict.clear();
    this->m_catelogDict.clear();

    this->LoadProductAndCatelog();
}

void CImageStore::InsertMeasureInfo ( std::vector<ImageMeasure::MeasureInfo> &infos ) {
    for ( int i = 0; i < infos.size(); i++ ) {
        auto &info = infos[i];
        InsertMeasureInfo ( info );
    }
}

void CImageStore::InsertMeasureInfo ( ImageMeasure::MeasureInfo &info ) {
    if ( !this->m_isConnected ) {
        this->OpenDatabase();
    }

    const wchar_t* wccName = info.name.c_str();
    char* nameChar = ( char* ) malloc ( sizeof ( char ) * ( 2 * wcslen ( wccName ) + 1 ) );
    memset ( nameChar, 0, 2 * wcslen ( wccName ) + 1 );
    strtool::w2c ( nameChar, wccName, 2 * wcslen ( wccName ) + 1 );

    const wchar_t* wccDesc = info.desc.c_str();
    char* descChar = ( char* ) malloc ( sizeof ( char ) * ( 2 * wcslen ( wccDesc ) + 1 ) );
    memset ( descChar, 0, 2 * wcslen ( wccDesc ) + 1 );
    strtool::w2c ( descChar, wccDesc, 2 * wcslen ( wccDesc ) + 1 );

    char buff[2048];
    sprintf_s ( buff, 2048, "insert into product_measure_info values(null, %d, '%s', '%s', %d, %d, %d, %d, %d)", info.pid, nameChar, descChar, info.start.x, info.start.y, info.end.x, info.end.y , info.side);
    int result = this->m_db.execDML ( buff );

    free ( descChar );
    free ( nameChar );
    descChar = nullptr;
    nameChar = nullptr;
}


void CImageStore::DeleteMeasureInfo ( int id ) {
    if ( !this->m_isConnected ) {
        this->OpenDatabase();
    }

    char buff[1024];
    sprintf_s ( buff, 1024, "delete from product_measure_info where id = %d", id );
    int result = this->m_db.execDML ( buff );
}