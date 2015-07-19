#include "StdAfx.h"
#include <iostream>
#include "ServerListener.h"
#include <afx.h>
#include "strtools.hpp"
#include <algorithm>
#include "ImageMeasureDlg.h"
#include "MD5.h"
#include "opencv2/opencv.hpp"

const char SECRET[8] = "1234567";

ImageMeasure::Semaphore QueueEmpty ( 0 );
char* g_buff[2];
int g_stopThread = 0;
int g_finish = 0;
uint64_t buffPosition = 0;
uint64_t lastestPosition = 0;
const int totalSize = 1024 * 1024 * 20;
const uint64_t FINISH_TYPE = 32767;
const int IMAGE_NUM = 40;

boost::mutex buffPositionLock;
boost::mutex lastPositionLock;
boost::mutex buffLock;
int g_restartFlag = 0;
CWnd* g_pMainWnd = nullptr;
void CheckMemBuffer();

boost::thread g_CheckBuffThread ( CheckMemBuffer );
//boost::thread g_CheckBuffThread;

struct ImageHolder {
    ImageMeasure::TransmissionHeader header;
    std::wstring imagePath;
};

ReceiveState g_threadState = START;

uint64_t htonll ( uint64_t value ) {
    // The answer is 42
    static const int num = 42;

    // Check the endianness
    if ( *reinterpret_cast<const char*> ( &num ) == num ) {
        const uint32_t high_part = htonl ( static_cast<uint32_t> ( value >> 32 ) );
        const uint32_t low_part = htonl ( static_cast<uint32_t> ( value & 0xFFFFFFFFLL ) );

        return ( static_cast<uint64_t> ( low_part ) << 32 ) | high_part;
    } else {
        return value;
    }
}

void GetObjectFromBuff ( void* data, size_t size, bool noAdding = false ) {
    int testA = lastestPosition / totalSize;
    int testB = ( lastestPosition + size ) / totalSize;
    uint64_t lasestOffset = lastestPosition - testA * totalSize;
    if ( testA == testB ) {
        buffLock.lock();
        memcpy ( data, g_buff[0] + lasestOffset, size );
        buffLock.unlock();
    } else {
        buffLock.lock();
        int sizeA = totalSize - lasestOffset;
        memcpy ( data, g_buff[0] + lasestOffset, sizeA );
        int sizeB = size - sizeA;
        memcpy ( ( BYTE* ) data + sizeA, g_buff[1], sizeB );
        std::swap ( g_buff[0], g_buff[1] );
        buffLock.unlock();
    }
    if ( !noAdding ) {
        lastPositionLock.lock();
        lastestPosition += size;
        lastPositionLock.unlock();
    } else {
        lastestPosition++;
    }
}


void PushObjectToBuff ( const BYTE* pData, int iLength ) {
    static char* targetBlock = g_buff[0];
    if ( g_restartFlag ) {
        targetBlock = g_buff[0];
        g_restartFlag = 0;
    }

    int testA = buffPosition / totalSize;
    int testB = ( buffPosition + iLength ) / totalSize;

    uint64_t buffOffset = buffPosition - testA * totalSize;
    if ( testA == testB ) {
        buffLock.lock();
        memcpy ( targetBlock + buffOffset, pData, iLength );
        buffLock.unlock();
    } else {
        int sizeA = testB * totalSize - buffPosition;
        buffLock.lock();
        memcpy ( g_buff[0] + buffOffset, pData, sizeA );
        memcpy ( g_buff[1], pData + sizeA, iLength - sizeA );
        if ( targetBlock == g_buff[0] ) {
            targetBlock = g_buff[1];
        } else {
            targetBlock = g_buff[0];
        }
        buffLock.unlock();
    }

    buffPositionLock.lock();
    buffPosition += iLength;
    buffPositionLock.unlock();
}


void CutPano ( std::wstring woverstr ) {
    {
        const wchar_t* oPathW = woverstr.c_str();
        char* oPath = ( char* ) malloc ( sizeof ( char ) * ( 2 * wcslen ( oPathW ) + 1 ) );
        memset ( oPath, 0, 2 * wcslen ( oPathW ) + 1 );
        strtool::w2c ( oPath, oPathW, 2 * wcslen ( oPathW ) + 1 );

        cv::Mat pano = cv::imread ( oPath );

        const int colOffset = 412;
        const int rowOffset = 250;

        cv::Mat temp = cv::Mat(710, 760, CV_8UC3);
        for ( int j = 0; j < temp.rows; j++ ) {
            for ( int i = 0; i < temp.cols; i++ ) {
                cv::Point3_<uchar>* p = pano.ptr<cv::Point3_<uchar> > ( j + rowOffset, i + colOffset );
                temp.ptr<cv::Point3_<uchar> > ( j, i )->x = p->x;
                temp.ptr<cv::Point3_<uchar> > ( j, i )->y = p->y;
                temp.ptr<cv::Point3_<uchar> > ( j, i )->z = p->z;
            }
        }

        cv::imwrite ( oPath, temp );

        free ( oPath );
        oPath = nullptr;
    }
}

void CheckMemBuffer() {
    CCameraCalculator calculator;
START:
    char* currentBuff = g_buff[0];
    std::vector<ImageHolder> imagesVector;
    while ( !g_stopThread ) {
        QueueEmpty.wait();
        if ( g_stopThread ) {
            return;
        }

ERROR_MARK:
        if ( g_threadState == HEADER_ERROR ) {
            ImageMeasure::TransmissionHeader header;
            while ( true ) {
                while ( lastestPosition + 8 > buffPosition ) {
                    QueueEmpty.wait();
                    if ( g_stopThread ) {
                        goto END;
                    }
                }
                GetObjectFromBuff ( &header, sizeof ( header ), true );

                header.length = htonl ( header.length );
                header.weight = htonl ( header.weight );
                header.type   = htonl ( header.type );
                header.id     = htonll ( header.id );
                header.gid    = htonll ( header.gid );
                header.height = ( header.height );

                if ( 0 == strcmp ( header.secret, SECRET ) && header.type == FINISH_TYPE ) {
                    g_threadState = START;
                    lastestPosition--;
                    break;
                }
            }
        }

        if ( g_threadState == START ) {
            while ( true ) {
                ImageHolder holder;
                std::shared_ptr<ImageMeasure::TransmissionHeader> header = std::make_shared<ImageMeasure::TransmissionHeader>();
                while ( lastestPosition + sizeof ( ImageMeasure::TransmissionHeader ) > buffPosition ) {
                    QueueEmpty.wait();
                    if ( g_stopThread ) {
                        goto END;
                    }
                }

                GetObjectFromBuff ( header.get(), sizeof ( ImageMeasure::TransmissionHeader ) );

                header->length = htonl ( header->length );
                header->weight = htonl ( header->weight );
                header->type = htonl ( header->type );
                header->id = htonll ( header->id );
                header->gid = htonll ( header->gid );
                header->height = ( header->height );

                if ( 0 != strcmp ( header->secret, SECRET ) ) {
                    g_threadState = HEADER_ERROR;
                    imagesVector.clear();
                    break;
                }

                if ( header->type == FINISH_TYPE ) {
                    g_threadState = FINISHING;
                    break;
                }

                memcpy ( &holder.header, header.get(), sizeof ( holder.header ) );

                while ( lastestPosition + header->length > buffPosition ) {
                    QueueEmpty.wait();
                    if ( g_stopThread ) {
                        goto END;
                    }
                }

                char* imageBuff = new char[header->length];
                GetObjectFromBuff ( imageBuff, header->length );
                SYSTEMTIME st;
                CString strDate, strTime;
                GetLocalTime ( &st );
                std::string str;
                strtool::rand_str ( 50, str );
                strDate.Format ( L"%4d%2d%2d%2d%2d%2d%2d%s", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds, str.c_str() );

                std::wstring wrandomStr = strDate.GetBuffer ( 0 );
                std::string randomStr = strtool::wstringToString ( wrandomStr );
                MD5 md5;
                md5.update ( randomStr );
                randomStr = md5.toString().c_str();
                std::wstring md5Filename = strtool::stringToWstring ( randomStr );

                TCHAR szPath[MAX_PATH];
                while ( !GetModuleFileName ( NULL, szPath, MAX_PATH ) ) {}

                std::wstring imagePath = strtool::getfilePathW ( szPath );

                if ( header->type <= 4 ) {
                    imagePath += L"\\data\\measure\\";
                } else {
                    imagePath += L"\\data\\pano\\";
                }

                CString folderNameCstring;
                folderNameCstring.Format(L"%04d", st.wYear);
                std::wstring folderName = folderNameCstring.GetBuffer(0);

                CreateDirectory((imagePath + folderName).c_str(), NULL);

                folderNameCstring.Format(L"%04d\\%02d%02d%02d", st.wYear, st.wMonth, st.wDay, st.wHour);
                folderName = folderNameCstring.GetBuffer(0);

                CreateDirectory((imagePath + folderName).c_str(), NULL);

                std::wstring imagePathBak = imagePath;
                imagePath += folderName + L"\\";
                imagePath += md5Filename;
                imagePath += L".jpg";

                CFileStatus fileStatus;
                CFile jpg;
                while ( jpg.GetStatus ( imagePath.c_str(), fileStatus ) ) {
                    GetObjectFromBuff ( imageBuff, header->length );
                    SYSTEMTIME st;
                    CString strDate, strTime;
                    GetLocalTime ( &st );
                    std::string str;
                    strtool::rand_str ( 50, str );
                    strDate.Format ( L"%4d%2d%2d%2d%2d%2d%2d%s", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds, str );

                    std::wstring wrandomStr = strDate.GetBuffer ( 0 );
                    std::string randomStr = strtool::wstringToString ( wrandomStr );
                    MD5 md5;
                    md5.update ( randomStr );
                    randomStr = md5.toString().c_str();
                    std::wstring md5Filename = strtool::stringToWstring ( randomStr );
                    imagePath = imagePathBak;
                    imagePath += folderName + L"\\";
                    imagePath += md5Filename;
                    imagePath += L".jpg";
                }

                jpg.Open ( imagePath.c_str(), CFile::modeCreate | CFile::modeReadWrite );
                jpg.Write ( imageBuff, header->length );

                holder.imagePath = imagePath;
                imagesVector.push_back ( holder );
                delete imageBuff;
            }
            //Do Finishing staff

            if ( g_threadState == HEADER_ERROR ) {
                g_threadState = HEADER_ERROR;
                goto ERROR_MARK;
            }

            g_threadState = PROCESSING;
            ImageMeasure::Catelog catelog = {};
            ImageMeasure::Product product = {};

            int minWeight = 32767;
            int minWeightType = -1;


            if ( imagesVector.size() != IMAGE_NUM ) {
                imagesVector.clear();
                g_threadState = START;
                continue;
            }

            for ( int i = 0; i < 4; i++ ) {
                auto &imageHolder = imagesVector[i];

                if ( minWeight > imageHolder.header.weight ) {
                    minWeight = imageHolder.header.weight;
                    minWeightType = imageHolder.header.type;
                }
            }

            minWeightType -= 1;
            minWeightType /= 2;

            //Temp
//            minWeightType = 1;

            catelog.id = -1;
            catelog.desc = L"未分组";
            catelog.name = L"未命名";
            product.name = L"未命名";
            product.weight = minWeight / 100.0f;
            product.length = -1;
            product.height = -1;
            product.width = -1;
            product.isHeight = imagesVector.front().header.height;

            for ( int i = 0; i < imagesVector.size(); i++ ) {
                auto &imageHolder = imagesVector[i];
                ImageMeasure::MeasureImage image;
                image.pid = -1;
                image.id = -1;
                image.imageType = imageHolder.header.type;
                image.imagePath = imageHolder.imagePath;
                int tmpType = imageHolder.header.type;
                tmpType -= 1;
                tmpType /= 2;
                if ( tmpType == minWeightType ) {
                    if ( image.imageType % 2 == 0 ) {
                        product.sidelookPathMin = image.imagePath;
                    } else {
                        product.overlookPathMin = image.imagePath;
                    }

                    image.canMeasure = true;
                } else {
                    image.canMeasure = false;
                }

                if ( i == 0 ) {
                    product.cameraHeight = imageHolder.header.height;
                }

                image.imagePath = imageHolder.imagePath;

                if ( image.imageType < 5 ) {
                    image.weight = imageHolder.header.weight;
                    product.measureImages.push_back ( image );
                } else {
                    image.weight = -1;
                    product.panoPath.push_back ( imageHolder.imagePath );
                    try {
                        boost::thread thrd(boost::bind(&CutPano, imageHolder.imagePath));
//                        thrd.join();
//                        CutPano(imageHolder.imagePath);
                    } catch (std::exception &e) {

                    }
                }
            }

            if ( g_pMainWnd != nullptr ) {
                CImageMeasureDlg* measureDlg = dynamic_cast<CImageMeasureDlg*> ( g_pMainWnd );

                try {
//                     boost::thread thrd(boost::bind(&CCameraCalculator::CalculateSize, &calculator, product.overlookPathMin.c_str(), product.sidelookPathMin.c_str(), product.width, product.height, product.length));
//                     thrd.join();
                    calculator.CutSideAndOver(product);
                    calculator.CalculateSize(product.overlookPathMin.c_str(), product.sidelookPathMin.c_str(), product.width, product.height, product.length);
//                     {
//                         for (size_t i = 0; i < product.measureImages.size(); i++) {
//                             auto &image = product.measureImages[i];
//                             std::string path = strtool::wstringToString(image.imagePath);
// 
//                             if (image.imageType == 1 || image.imageType == 3) {
//                                 calculator.ImageImporve(path.c_str());
//                             }
//                         }
// 
//                     }
                    measureDlg->DataStore()->InsertProduct(product);
                } catch (std::exception &e) {

                }

                char *done = new char[5];
                done[0] = 'd';
                done[0] = 'o';
                done[0] = 'n';
                done[0] = 'e';
                done[0] = '\0';
                int content_len = strlen(done);
                info_msg *msg = info_msg::Construct(0, EVT_ON_DONE, content_len, done);

                if (g_pMainWnd == nullptr ||
                        (g_pMainWnd)->GetSafeHwnd() == nullptr ||
                        !((g_pMainWnd)->PostMessage(USER_INFO_MSG, (WPARAM)msg))) {
//                    info_msg::Destruct(msg);
                }
            }

            imagesVector.clear();
            g_threadState = START;
        }
    }

END:
    lastestPosition = 0;
    buffPosition = 0;
    g_stopThread = 0;
    g_restartFlag = 1;
    QueueEmpty.set ( 0 );
    g_threadState = START;
    if ( g_finish ) {
        return;
    } else {
        goto START;
    }
}

info_msg* info_msg::Construct ( CONNID dwConnID, char* lpszEvent, int iContentLength, char* lpszContent ) {
    return new info_msg ( dwConnID, lpszEvent, iContentLength, lpszContent );
}

void info_msg::Destruct ( info_msg* pMsg ) {
    delete pMsg;
}

info_msg::info_msg ( CONNID dwConnID, char* lpszEvent, int iContentLength, char* lpszContent )
    : connID ( dwConnID ), evt ( lpszEvent ), contentLength ( iContentLength ), content ( lpszContent ) {

}

info_msg::~info_msg() {
    if ( contentLength > 0 && content != nullptr ) {
        delete[] content;
    }
}


CServerListener::CServerListener ( CWnd* pMainWnd ) {
    this->m_pMainWnd = pMainWnd;
    g_pMainWnd = pMainWnd;

    m_buff = new char[1024 * 1024 * 20];
    m_receivedByte = 0;
    m_totalByte = 0;
    m_receiveState = WAIT_FOR_HRAD;
    m_currentId = L"";

    g_buff[0] = new char[totalSize];
    g_buff[1] = new char[totalSize];
}

CServerListener::~CServerListener ( void ) {
    this->m_pMainWnd = nullptr;

    if (g_buff[0] != nullptr) {
        delete[] g_buff[0];
        g_buff[0] = nullptr;
    }

    if (g_buff[1] != nullptr) {
        delete[] g_buff[1];
        g_buff[1] = nullptr;
    }

    delete[] m_buff;
    m_buff = nullptr;
    //g_stopThread = 1;
    QueueEmpty.signal();
}

EnHandleResult CServerListener::OnAccept ( CONNID dwConnID, SOCKET soClient ) {
    return HR_OK;
}

EnHandleResult CServerListener::OnPrepareListen ( SOCKET soListen ) {
    return HR_OK;
}

EnHandleResult CServerListener::OnSend ( CONNID dwConnID, const BYTE* pData, int iLength ) {
    return HR_OK;
}


EnHandleResult CServerListener::OnReceive ( CONNID dwConnID, const BYTE* pData, int iLength ) {
    if ( iLength == 0 ) {
        return HR_OK;
    }

    PushObjectToBuff ( pData, iLength );
    QueueEmpty.signal();
    return HR_OK;
    /*    ImageMeasure::TransmissionHeader header;
        if(iLength < 0) {
            return HR_IGNORE;
        }

        if(this->m_receiveState == WAIT_FOR_HRAD) {
            if (iLength != sizeof(header)) {
                int error = 0;

                return HR_OK;
            }
            memcpy(&header, pData, iLength);

            header.length = htonl(header.length);
            header.weight = htonl(header.weight);
            header.type   = htonl(header.type);

            if (strcmp(header.secret, SECRET) != 0) {
                return HR_IGNORE;
            }

            //received end package
            if (header.type == 32767) {
                ImageMeasure::Catelog catelog = {};
                ImageMeasure::Product product = {};

                catelog.id = -1;
                product.type = catelog;

                double minWeight = 100000;
                int minID = -1;
                int minType = 0;
                for (int i = 0; i < this->m_imageHolder.measureImages.size(); i++) {
                    auto &image = this->m_imageHolder.measureImages[i];
                    if (minWeight > image.weight) {
                        minWeight = image.weight;
                        minID = i;
                        if (image.imageType <= 2) {
                            minType = 0;
                        }
                        else {
                            minType = 1;
                        }
                    }
                }

                for (int i = 0; i < this->m_imageHolder.measureImages.size(); i++) {
                    auto &image = this->m_imageHolder.measureImages[i];
                    if (minType == 0) {
                        if (image.imageType <= 2) {
                            image.canMeasure = true;
                        }
                    }
                    else {
                        if (image.imageType > 2) {
                            image.canMeasure = true;
                        }
                    }
                }

                product.weight = minWeight;
                //product.panoPath.assign(this->m_imageHolder.panoPath.begin(), this->m_imageHolder.panoPath.end());
                product.panoPath.swap(this->m_imageHolder.panoPath);
                //product.measureImages.assign(this->m_imageHolder.measureImages.begin(), this->m_imageHolder.measureImages.end());
                product.measureImages.swap(this->m_imageHolder.measureImages);
                product.length = -1;
                product.height = -1;
                product.width  = -1;
                product.isReverse = this->m_receivedHeader.isReverse;

                CImageMeasureDlg *measureDlg = dynamic_cast<CImageMeasureDlg*>(this->m_pMainWnd);
                measureDlg->DataStore()->InsertProduct(product);

                char *imageHeader = new char[7];
                imageHeader[0] = '1';
                imageHeader[1] = '0';

                int content_len = strlen(imageHeader);
                info_msg *msg = info_msg::Construct(dwConnID, EVT_ON_RECEIVE, content_len, imageHeader);

                PostInfoMsg(msg);

                this->m_imageHolder.measureImages.clear();
                this->m_imageHolder.panoPath.clear();

                return HR_OK;
            }

            this->m_totalByte = header.length;
            if (header.length == 0) {
                return HR_OK;
            }

            //DEEP COPY
            this->m_receivedHeader.length = header.length;
            this->m_receivedHeader.type = header.type;
            this->m_receivedHeader.weight = header.weight;
            this->m_receivedHeader.isReverse = header.isReverse;
            strcpy_s(this->m_receivedHeader.secret, header.secret);

            SYSTEMTIME st;
            CString strDate, strTime;
    //         GetLocalTime(&st);
    //         strDate.Format(L"%4d%2d%2d%2d%2d%2d%2d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

    //         char buff[128];
    //         srand(clock());
    //         sprintf_s(buff, 1024, "%d%d%d", rand(), rand(), rand());
    //        std::string str(buff);
            std::string str;
            strtool::rand_str(8, str);
            MD5 md5;
            md5.update(str);
            str = md5.toString().c_str();
            this->m_currentId = strtool::stringToWstring(str);
            this->m_receiveState = RECEIVING;

            return HR_OK;
        }

        if(this->m_receiveState == RECEIVING) {
            memcpy(this->m_buff + this->m_receivedByte, pData, iLength);
            this->m_receivedByte += iLength;

            if(this->m_receivedByte > m_totalByte) {
                this->m_receiveState = FINISHING;
                this->m_receivedByte = 0;
                this->m_totalByte = 0;
                this->m_receiveState = WAIT_FOR_HRAD;
                return HR_OK;
            }

            if(this->m_receivedByte == m_totalByte) {
                this->m_receiveState = FINISHING;

                TCHAR szPath[MAX_PATH];
                if (!GetModuleFileName(NULL, szPath, MAX_PATH)) {
                    return HR_ERROR;
                }

                std::wstring imagePath = strtool::getfilePathW(szPath);

                if (this->m_receivedHeader.type <= 4) {
                    imagePath += L"\\data\\measure\\";
                }
                else {
                    imagePath += L"\\data\\pano\\";
                }

                imagePath += this->m_currentId;
                imagePath += L".jpg";

                if (this->m_receivedHeader.type > 4) {
                    this->m_imageHolder.panoPath.push_back(imagePath);
                }
                else {
                    ImageMeasure::MeasureImage measureImage;
                    measureImage.imageType = this->m_receivedHeader.type;
                    measureImage.imagePath = imagePath;
                    measureImage.id = -1;
                    measureImage.pid = -1;
                    measureImage.weight = this->m_receivedHeader.weight;
                    measureImage.canMeasure = false;
                    this->m_imageHolder.measureImages.push_back(measureImage);
                }

                CFile jpg;
                jpg.Open(imagePath.c_str(), CFile::modeCreate | CFile::modeReadWrite);
                jpg.Write(this->m_buff, this->m_totalByte);

                this->m_receivedByte = 0;
                this->m_totalByte = 0;
                this->m_receiveState = WAIT_FOR_HRAD;
                return HR_OK;
            }

            return HR_OK;
        }

        return HR_ERROR;
    //     char *lpszContent = new char[iLength];
    //
    //     memcpy(lpszContent, pData, iLength);
    //
    //     CFile jpg;
    //     jpg.Open(L"../test.jpg", CFile::modeCreate | CFile::modeReadWrite);
    //
    //     jpg.Write(lpszContent, iLength);
    //
    //
    //     lpszContent[iLength] = '\0';
    //
    //     if(m_pMainWnd != nullptr)
    //     {
    //         int content_len = strlen(lpszContent);
    //         info_msg *msg = info_msg::Construct(dwConnID, EVT_ON_RECEIVE, content_len, lpszContent);
    //
    //         PostInfoMsg(msg);
    //     }

    //    return HR_OK;
    */
}

void CServerListener::PostInfoMsg ( info_msg* msg ) {
    if ( this->m_pMainWnd == nullptr                              ||
            ( this->m_pMainWnd )->GetSafeHwnd() == nullptr              ||
            ! ( ( this->m_pMainWnd )->PostMessage ( USER_INFO_MSG, ( WPARAM ) msg ) ) ) {
        info_msg::Destruct ( msg );
    }
}

EnHandleResult CServerListener::OnReceive ( CONNID dwConnID, int iLength ) {
    return HR_OK;
}

EnHandleResult CServerListener::OnClose ( CONNID dwConnID ) {
    return HR_OK;
}

EnHandleResult CServerListener::OnError ( CONNID dwConnID, EnSocketOperation enOperation, int iErrorCode ) {
    g_threadState = HEADER_ERROR;
    return HR_OK;
}

EnHandleResult CServerListener::OnShutdown() {
    return HR_OK;
}
