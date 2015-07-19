#include "stdafx.h"
#include "CameraCalculator.h"
#include "opencv2/opencv.hpp"
#include "strtools.hpp"
#include <iostream>

#include "pcl/io/pcd_io.h"
#include "pcl/point_types.h"
#include "pcl/filters/statistical_outlier_removal.h"

const int _OFFSET_Y = 0;
const int SIDE_CENTER_Y = 1944 / 2 + _OFFSET_Y;
const double coH = 1;
const double coL = 0.03 / 100;
const double coW = 0.03 / 100;
const double BASE_HEIGHT = 20;
const double centerCol = 900;
const double centerRow = 0;
const double overCameraHeight = 100;
const double sideCameraHeight = 180;

const double sideCameraRadio = 0.085;
const double overCameraRadio = 0.051;


CCameraCalculator::CCameraCalculator() {
}


CCameraCalculator::~CCameraCalculator() {
}

void CCameraCalculator::CalculateLocation ( const ImageMeasure::Point2I &src, ImageMeasure::Point2D &des ) {
    des.x = src.x;
    des.y = src.y;
}

void CCameraCalculator::CutSideAndOver(const ImageMeasure::Product &product) {
    for (size_t i = 0; i < product.measureImages.size(); i++) {
        auto &image = product.measureImages[i];
        std::string path = strtool::wstringToString(image.imagePath);
        cv::Mat cvMat;
        cvMat = cv::imread(path.c_str());

        if (image.imageType == 1 || image.imageType == 3) {
            if (cvMat.cols > 1944) {
                this->CutOverlookImage(cvMat);
            }
        } else {
            if (cvMat.cols > 1900) {
                this->CutSidelookImage(cvMat);
            }
        }

        cv::imwrite(path.c_str(), cvMat);

    }
}

void CCameraCalculator::CalculateSize ( const TCHAR* overlookPath, const TCHAR* sidelookPath, double &width, double &height, double &length ) {
    int r, g, b;
    CFile params;

    TCHAR szPath[MAX_PATH];
    while ( !GetModuleFileName ( NULL, szPath, MAX_PATH ) ) {}

    std::wstring backgroundPath = strtool::getfilePathW ( szPath );
    backgroundPath += L"\\data\\background.jpg";

    std::wstring sideBackPath = strtool::getfilePathW ( szPath );
    sideBackPath += L"\\data\\sideback.jpg";
//     CFileStatus fileStatus;
//     if ( !params.GetStatus ( paramPath.c_str(), fileStatus ) ) {
//         r = 10;
//         g = 10;
//         b = 10;
// //       params.Open ( paramPath.c_str(), CFile::modeReadWrite | CFile::modeCreate);
// //         params.Write ( &r, sizeof ( r ) );
// //         params.Write ( &g, sizeof ( g ) );
// //         params.Write ( &b, sizeof ( b ) );
// //         params.Close();
//     } else {
//         params.Open ( paramPath.c_str(), CFile::modeReadWrite | CFile::modeCreate );
//         params.Read ( &r, sizeof ( r ) );
//         params.Read ( &g, sizeof ( g ) );
//         params.Read ( &b, sizeof ( b ) );
//         params.Close();
//     }

    cv::Vec3i rgb;
//     rgb[0] = r;
//     rgb[1] = g;
//     rgb[2] = b;

    const wchar_t* oPathW = overlookPath;
    char* oPath = ( char* ) malloc ( sizeof ( char ) * ( 2 * wcslen ( oPathW ) + 1 ) );
    memset ( oPath, 0, 2 * wcslen ( oPathW ) + 1 );
    strtool::w2c ( oPath, oPathW, 2 * wcslen ( oPathW ) + 1 );

    const wchar_t* sPathW = sidelookPath;
    char* sPath = ( char* ) malloc ( sizeof ( char ) * ( 2 * wcslen ( sPathW ) + 1 ) );
    memset ( sPath, 0, 2 * wcslen ( sPathW ) + 1 );
    strtool::w2c ( sPath, sPathW, 2 * wcslen ( sPathW ) + 1 );

    const wchar_t* bPathW = backgroundPath.c_str();
    char* bPath = ( char* ) malloc ( sizeof ( char ) * ( 2 * wcslen ( bPathW ) + 1 ) );
    memset ( bPath, 0, 2 * wcslen ( bPathW ) + 1 );
    strtool::w2c ( bPath, bPathW, 2 * wcslen ( bPathW ) + 1 );

    const wchar_t* sbPathW = sideBackPath.c_str();
    char* sbPath = ( char* ) malloc ( sizeof ( char ) * ( 2 * wcslen ( sbPathW ) + 1 ) );
    memset ( sbPath, 0, 2 * wcslen ( sbPathW ) + 1 );
    strtool::w2c ( sbPath, sbPathW, 2 * wcslen ( sbPathW ) + 1 );

    cv::Mat overlook;
    overlook = cv::imread ( oPath );
//     if ( overlook.cols > 1944 ) {
//         this->CutOverlookImage ( overlook );
//         cv::imwrite ( oPath, overlook );
//     }

    cv::Mat sidelook;
    sidelook = cv::imread ( sPath );
//     if ( sidelook.cols > 2000 ) {
//         this->CutSidelookImage ( sidelook );
//         cv::imwrite ( sPath, sidelook );
//     }

    cv::Mat background;
    background = cv::imread ( bPath );

    cv::Mat sideback;
    sideback = cv::imread ( sbPath );

    free ( oPath );
    free ( sPath );
    free ( bPath );
    free ( sbPath );
    oPath = nullptr;
    sPath = nullptr;
    bPath = nullptr;
    sbPath = nullptr;

    cv::Mat overDst, sideDst;

    this->ChromaKey ( overlook, background, overDst, rgb, true );
    this->ChromaKey ( sidelook, sideback, sideDst, rgb, false );

    cv::Mat overBin = cv::Mat ( overlook.rows, overlook.cols, CV_8UC1, cv::Scalar ( 0 ) );
    cv::Mat sideBin = cv::Mat ( sidelook.rows, sidelook.cols, CV_8UC1, cv::Scalar ( 0 ) );

    //Begin PCL
    {
        {
            pcl::PointCloud<pcl::PointXYZ>::Ptr cloud ( new pcl::PointCloud<pcl::PointXYZ> );
            pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_filtered ( new pcl::PointCloud<pcl::PointXYZ> );

            for ( int i = 0; i < overDst.cols; i++ ) {
                for ( int j = 0; j < overDst.rows; j++ ) {
                    if ( overDst.at<uchar> ( j, i ) == 255 ) {
                        pcl::PointXYZ point;
                        point.x = i;
                        point.y = j;
                        point.z = 0;

                        cloud->push_back ( point );
                    }
                }
            }

            pcl::StatisticalOutlierRemoval<pcl::PointXYZ> sor;
            sor.setInputCloud ( cloud );
            sor.setMeanK ( 50 );
            sor.setStddevMulThresh ( 1.0 );
            sor.filter ( *cloud_filtered );

            if (!cloud_filtered->empty()) {
                for ( size_t i = 0; i < cloud_filtered->size(); i++ ) {
                    auto &point = ( *cloud_filtered ) [i];
                    overBin.at<uchar> ( ( int ) point.y, ( int ) point.x ) = 255;
                }
            }
        }

        {
            pcl::PointCloud<pcl::PointXYZ>::Ptr cloud ( new pcl::PointCloud<pcl::PointXYZ> );
            pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_filtered ( new pcl::PointCloud<pcl::PointXYZ> );

            for ( int i = 0; i < sideDst.cols; i++ ) {
                for ( int j = 0; j < sideDst.rows; j++ ) {
                    if ( sideDst.at<uchar> ( j, i ) == 255 ) {
                        pcl::PointXYZ point;
                        point.x = i;
                        point.y = j;
                        point.z = 0;

                        cloud->push_back ( point );
                    }
                }
            }

            pcl::StatisticalOutlierRemoval<pcl::PointXYZ> sor;
            sor.setInputCloud ( cloud );
            sor.setMeanK ( 50 );
            sor.setStddevMulThresh ( 1.0 );
            sor.filter ( *cloud_filtered );

            if (!cloud_filtered->empty()) {
                for ( size_t i = 0; i < cloud_filtered->size(); i++ ) {
                    auto &point = ( *cloud_filtered ) [i];
                    sideBin.at<uchar> ( ( int ) point.y, ( int ) point.x ) = 255;
                }
            }
        }
    }

    {
        std::wstring woverstr = overlookPath;
        woverstr += L".bin.jpg";
        const wchar_t* oPathW = woverstr.c_str();
        char* oPath = ( char* ) malloc ( sizeof ( char ) * ( 2 * wcslen ( oPathW ) + 1 ) );
        memset ( oPath, 0, 2 * wcslen ( oPathW ) + 1 );
        strtool::w2c ( oPath, oPathW, 2 * wcslen ( oPathW ) + 1 );
        cv::imwrite ( oPath, overBin );
        free ( oPath );
        oPath = nullptr;
    }

    {
        std::wstring wsidestr = sidelookPath;
        wsidestr += L".bin.jpg";
        const wchar_t* sPathW = wsidestr.c_str();
        char* sPath = ( char* ) malloc ( sizeof ( char ) * ( 2 * wcslen ( sPathW ) + 1 ) );
        memset ( sPath, 0, 2 * wcslen ( sPathW ) + 1 );
        strtool::w2c ( sPath, sPathW, 2 * wcslen ( sPathW ) + 1 );
        cv::imwrite ( sPath, sideBin );
        free ( sPath );
        sPath = nullptr;
    }

    cv::RotatedRect overlookRect;
    std::vector<cv::Point2f> overPoints;

    this->GetMinAreaRect ( overBin, overlookRect, overPoints );

    cv::Rect sideLookRect;
    std::vector<cv::Point2f> sidePoints;
    this->GetBoundRect ( sideBin, sideLookRect, sidePoints );

    {
        if ( overlookRect.size.width > overlookRect.size.height ) {
            length = overlookRect.size.width;
            width = overlookRect.size.height;
        } else {
            width = overlookRect.size.width;
            length = overlookRect.size.height;
        }

        height = sideLookRect.x + sideLookRect.width + BASE_HEIGHT;

        cv::Point2f vertices[4];
        overlookRect.points ( vertices );

        int minX = 1800;
        for ( size_t i = 0; i < overPoints.size(); i++ ) {
            auto &vertex = overPoints[i];

            if ( minX > vertex.x ) {
                minX = vertex.x;
            }
        }

        double tempX = 900 - minX;
        tempX *= overCameraRadio;
        height *= sideCameraRadio;

        this->IteratorCalcuate ( height, tempX );

        width *= overCameraRadio * ( overCameraHeight - height ) / overCameraHeight;
        length *= overCameraRadio * ( overCameraHeight - height ) / overCameraHeight;
    }

    //DEBUG
    return;
//
//     double rad = overlookRect.angle * 3.1415926 / 180;
//
//     cv::Point2f overImgWidthPoint;
//     overImgWidthPoint.x = overlookRect.center.x + ( overlookRect.size.width / 2.0f ) * std::cosf ( rad );
//     overImgWidthPoint.y = overlookRect.center.y + ( overlookRect.size.width / 2.0f ) * std::sinf ( rad );
//
//     rad = ( 90 - overlookRect.angle ) * 3.1415926 / 180;
//
//     cv::Point2f overImgHeightPoint;
//     overImgHeightPoint.x = overlookRect.center.x + ( overlookRect.size.height / 2.0f ) * std::cosf ( rad );
//     overImgHeightPoint.y = overlookRect.center.y + ( overlookRect.size.height / 2.0f ) * std::sinf ( rad );
//
//     double x = overImgHeightPoint.x - overlookRect.center.x;
//     double h = 0;
//
//     cv::Point2i minPoint;
//     int minX = 32767;
//     for ( int i = 0; i < sidePoints.size(); i++ ) {
//         auto &point = sidePoints[i];
//         if ( point.x < minX ) {
//             minX = point.x;
//             minPoint = point;
//         }
//     }
//
//     int offsetY = minPoint.y - SIDE_CENTER_Y;
//
//     cv::Point2i overlookEdgeTargetPoint;
//     overlookEdgeTargetPoint.y = overlookRect.center.y + offsetY;
//
//     for ( int i = 0; i < overlook.cols; i++ ) {
//         int data = overlook.at<uchar> ( overlookEdgeTargetPoint.y, i );
//         if ( data == 255 ) {
//             overlookEdgeTargetPoint.x = i;
//             break;
//         }
//     }
//
//     height = sidelook.cols - minPoint.x;
//     width = overlookRect.size.width;
//     length = overlookRect.size.height;
//
//     height *= coH;
//     width *= coW;
//     length *= coL;
//
//     if ( width > length ) {
//         std::swap ( width, length );
//     }
//
//     if ( height > length && height > 30 ) {
//         this->IteratorCalcuate ( height, length );
//         this->IteratorCalcuate ( height, width );
//     }

    //TODO: Transform

//     this->IteratorCalcuate(height, overlookEdgeTargetPoint.x);
//
//     {
//         cv::Mat rotateMat = cv::Mat(2, 2, CV_64FC1);
//         double rotateRad = -overlookRect.angle * 3.1415926 / 180;
//
//         rotateMat.at<double>(0, 0) = std::cosf(rotateRad);
//         rotateMat.at<double>(0, 1) = -std::sinf(rotateRad);
//         rotateMat.at<double>(1, 0) = std::sinf(rotateRad);
//         rotateMat.at<double>(1, 1) = std::cosf(rotateRad);
//
//         cv::Mat invRotateMat = cv::Mat(2, 2, CV_64FC1);
//
//         invRotateMat.at<double>(0, 0) = std::cosf(rotateRad);
//         invRotateMat.at<double>(0, 1) = std::sinf(rotateRad);
//         invRotateMat.at<double>(1, 0) = -std::sinf(rotateRad);
//         invRotateMat.at<double>(1, 1) = std::cosf(rotateRad);
//
//         for (int i = 0; i < overPoints.size(); i++) {
//             auto &point = overPoints[i];
//             cv::Mat pointMat = cv::Mat(2, 1, CV_64FC1);
//             pointMat.at<double>(0, 0) = point.x;
//             pointMat.at<double>(1, 0) = point.y;
//             pointMat = rotateMat * pointMat;
//             point.x = pointMat.at<double>(0, 0);
//             point.y = pointMat.at<double>(1, 0);
//         }
//
//         double minX, maxX, minY, maxY;
//         int minXID, maxXID, minYID, maxYID;
//         for (int i = 0; i < overPoints.size(); i++) {
//             auto &point = overPoints[i];
//             if (i == 0) {
//                 minXID = 0;
//                 minYID = 0;
//                 maxXID = 0;
//                 maxYID = 0;
//
//                 minX = maxX = point.x;
//                 minY = maxY = point.y;
//             }
//             else {
//                 if (maxX < point.x) {
//                     maxX = point.x;
//                     maxXID = i;
//                 }
//
//                 if (maxY < point.y) {
//                     maxY = point.y;
//                     maxYID = i;
//                 }
//
//                 if (minX > point.x) {
//                     minX = point.x;
//                     minXID = i;
//                 }
//
//                 if (minY > point.y) {
//                     minY = point.y;
//                     minYID = i;
//                 }
//             }
//         }
//
//         {
//             cv::Point2d minWidthPoint, maxWidthPoint, minHeightPoint, maxHeightPoint;
//
//             cv::Mat pointMat = cv::Mat(2, 1, CV_64FC1);
//             pointMat.at<double>(0, 0) = overPoints[minXID].x;
//             pointMat.at<double>(1, 0) = overPoints[minXID].y;
//             pointMat = invRotateMat * pointMat;
//             minWidthPoint.x = pointMat.at<double>(0, 0);
//             minWidthPoint.y = pointMat.at<double>(1, 0);
//
//             pointMat.at<double>(0, 0) = overPoints[minYID].x;
//             pointMat.at<double>(1, 0) = overPoints[minYID].y;
//             pointMat = invRotateMat * pointMat;
//             minHeightPoint.x = pointMat.at<double>(0, 0);
//             minHeightPoint.y = pointMat.at<double>(1, 0);
//
//             pointMat.at<double>(0, 0) = overPoints[maxXID].x;
//             pointMat.at<double>(1, 0) = overPoints[maxXID].y;
//             pointMat = invRotateMat * pointMat;
//             maxWidthPoint.x = pointMat.at<double>(0, 0);
//             maxWidthPoint.y = pointMat.at<double>(1, 0);
//
//             pointMat.at<double>(0, 0) = overPoints[maxYID].x;
//             pointMat.at<double>(1, 0) = overPoints[maxYID].y;
//             pointMat = invRotateMat * pointMat;
//             maxHeightPoint.x = pointMat.at<double>(0, 0);
//             maxHeightPoint.y = pointMat.at<double>(1, 0);
//
//             int minWidthH = 0;
//
//
//         }
//
//     }
}

void CCameraCalculator::GetBoundRect ( cv::Mat&src, cv::Rect& rect, std::vector<cv::Point2f>&points ) {
    for ( int i = 0; i < src.cols; i++ ) {
        for ( int j = 0; j < src.rows; j++ ) {
            int data = src.at<uchar> ( j, i );
            if ( data == 255 ) {
                cv::Point2f point;
                point.x = i;
                point.y = j;
                points.push_back ( point );
            }
        }
    }
    if ( !points.empty() ) {
        rect = cv::boundingRect ( points );
    } else {
        rect = cv::Rect ( cv::Point ( 0, 0 ), cv::Point ( 0, 0 ) );
    }
}

void CCameraCalculator::GetMinAreaRect ( cv::Mat &src, cv::RotatedRect &rect, std::vector<cv::Point2f>&points ) {
    for ( int i = 0; i < src.cols; i++ ) {
        for ( int j = 0; j < src.rows; j++ ) {
            int data = src.at<uchar> ( j, i );
            if ( data == 255 ) {
                cv::Point2f point;
                point.x = i;
                point.y = j;
                points.push_back ( point );
            }
        }
    }

    rect = cv::minAreaRect ( points );
}


void CCameraCalculator::CalculateLength ( const TCHAR* overlookBinPath, const TCHAR* sidelookBinPath, const ImageMeasure::Point2D from, const ImageMeasure::Point2D to, double &length, int type , ImageMeasure::Product &product ) {
    double COX = 0.003;
    double COY = 0.003;

    double CO = 0.13;
    if ( type == 1 ) {
        CO = overCameraRadio;
    } else {
        CO = overCameraRadio;
    }

    double fromX = from.x * 1800.0 / 472.0;
    double fromY = from.y * 1800.0 / 472.0;
    double toX = to.x * 1800.0 / 472.0;
    double toY = to.y * 1800.0 / 472.0;

    ImageMeasure::Point2D vec;
    vec.x = fromX - toX;
    vec.y = fromY - toY;

    //vec.x *= COX;
    //vec.y *= COY;

    length = std::sqrtf ( vec.x * vec.x + vec.y * vec.y );
    length *= CO;

}

void CCameraCalculator::RotateImage ( cv::Mat& image, double degree ) {
    cv::Point2f center ( image.cols / 2, image.rows / 2 );

    float m[6];
    cv::Mat M = cv::getRotationMatrix2D ( center, degree, 1 );
    cv::warpAffine ( image, image, M, cv::Size ( image.cols, image.rows ) );

}

void CCameraCalculator::CutOverlookImage(cv::Mat& image) {
    const int colOffset = 396;
    const int rowOffset = 5;

    cv::Mat temp = cv::Mat(1800, 1800, CV_8UC3);
    for (int j = 0; j < temp.rows; j++) {
        for (int i = 0; i < temp.cols; i++) {
            if ((j + rowOffset) > image.rows || (i + colOffset) > image.cols) {
                continue;
            }

            cv::Point3_<uchar>* p = image.ptr<cv::Point3_<uchar> >(j + rowOffset, i + colOffset);
            temp.ptr<cv::Point3_<uchar> >(j, i)->x = p->x;
            temp.ptr<cv::Point3_<uchar> >(j, i)->y = p->y;
            temp.ptr<cv::Point3_<uchar> >(j, i)->z = p->z;
        }
    }

    image = temp.clone();
}

//Change image direction
void CCameraCalculator::CutSidelookImage(cv::Mat& image) {
    const int colOffset = 1055;
    const int rowOffset = 300;

    cv::Mat temp = cv::Mat(480, 700, CV_8UC3);
    for (int j = 0; j < temp.rows; j++) {
        for (int i = 0; i < temp.cols; i++) {
            if ((j + rowOffset) > image.rows || (i + colOffset) > image.cols) {
                continue;
            }

            cv::Point3_<uchar>* p = image.ptr<cv::Point3_<uchar> >(j + rowOffset, i + colOffset);
            temp.ptr<cv::Point3_<uchar> >(j, i)->x = p->x;
            temp.ptr<cv::Point3_<uchar> >(j, i)->y = p->y;
            temp.ptr<cv::Point3_<uchar> >(j, i)->z = p->z;
        }
    }

    image = temp.clone();
}
void CCameraCalculator::CalculateChromaKeyValue ( cv::Mat &background, cv::Vec3i &rgb ) {
    double sumR = 0, sumG = 0, sumB = 0;
    for ( int i = 0; i < background.cols; i++ ) {
        for ( int j = 0; j < background.rows; j++ ) {
            cv::Point3_<uchar>* p = background.ptr<cv::Point3_<uchar> > ( j, i );
            sumB += p->x;
            sumG += p->y;
            sumR += p->z;
        }
    }

    long long totalPixel = background.cols * background.rows;

    sumR /= totalPixel;
    sumG /= totalPixel;
    sumB /= totalPixel;

    rgb[0] = sumR;
    rgb[1] = sumG;
    rgb[2] = sumB;
}

void CCameraCalculator::ChromaKey ( cv::Mat &front, cv::Mat &background, cv::Mat &dst, cv::Vec3i &rgb, bool isOver ) {
    const int minDistance = 65;
    const double maxDistance = std::sqrtf ( ( 255 * 255 ) * 3 );
    cv::Mat distanceImage = cv::Mat ( front.rows, front.cols, CV_8UC1 , cv::Scalar ( 0 ) );
    dst = cv::Mat ( front.rows, front.cols, CV_8UC1 , cv::Scalar ( 0 ) );


    for ( int i = 0; i < front.cols; i++ ) {
        for ( int j = 0; j < front.rows; j++ ) {

//             if ( isOver ) {
//                 if ( ( i < 200 && j < 200 ) || ( i > front.cols - 200 || j > front.rows - 200 ) ) {
//                     continue;
//                 }
//             }

            long sumR = 0, sumG = 0, sumB = 0;
            for ( int x = -1; x <= 1; x++ ) {
                for ( int y = -1; y <= 1; y++ ) {
                    int xIndex = i + x;
                    int yIndex = j + y;

                    if ( xIndex < 0 ) {
                        xIndex = 0;
                    }

                    if ( yIndex < 0 ) { 
                        yIndex = 0;
                    }

                    if ( xIndex >= background.cols - 1 ) {
                        xIndex = background.cols - 1;
                    }

                    if ( yIndex >= background.rows - 1 ) {
                        yIndex = background.rows - 1;
                    }

                    cv::Point3_<uchar>* p = background.ptr<cv::Point3_<uchar> > ( j, i );
                    sumB += p->x;
                    sumG += p->y;
                    sumR += p->z;
                }
            }

            sumR /= ( double ) ( 3 * 3 );
            sumG /= ( double ) ( 3 * 3 );
            sumB /= ( double ) ( 3 * 3 );

            rgb[0] = sumR;
            rgb[1] = sumG;
            rgb[2] = sumB;

            cv::Point3_<uchar>* p = front.ptr<cv::Point3_<uchar> > ( j, i );
            int B = p->x;
            int G = p->y;
            int R = p->z;

//             cv::Vec3b intensity = front.at<cv::Vec3b> ( i, j );
//             uchar B = intensity.val[0];
//             uchar G = intensity.val[1];
//             uchar R = intensity.val[2];

            double distance = std::sqrtf ( ( R - rgb[0] ) * ( R - rgb[0] ) + ( G - rgb[1] ) * ( G - rgb[1] ) + ( B - rgb[2] ) * ( B - rgb[2] ) );

            distance /= maxDistance;
            distance *= 255;

            if ( distance > 255 ) {
                distance = 255;
            }

            if ( distance < 0 ) {
                distance = 0;
            }

            distanceImage.at<uchar> ( j, i ) = ( uchar ) distance;
        }
    }

    for ( int i = 0; i < distanceImage.cols; i++ ) {
        for ( int j = 0; j < distanceImage.rows; j++ ) {
            int distance = distanceImage.at<uchar> ( j, i );

            if ( distance > minDistance ) {
                dst.at<uchar> ( j, i ) = 255;
            } else {
                dst.at<uchar> ( j, i ) = 0;
            }
        }
    }
}

void CCameraCalculator::ImageImporve(const char* over) {
    cv::Mat src, dst;
    double alpha = 1.2;
    double beta = 60;

    src = cv::imread(over);
    if (!src.data) {
        cout << "Failed to load image!" << endl;
        return;
    }

    dst = cv::Mat::zeros(src.size(), src.type());
    for (int i = 0; i < src.rows; ++i)
        for (int j = 0; j < src.cols; ++j)
            for (int k = 0; k < 3; ++k) {
                dst.at<cv::Vec3b>(i, j)[k] = cv::saturate_cast<uchar>(src.at<cv::Vec3b>(i, j)[k] * alpha + beta);
            }

    cv::imwrite(over, dst);
}

void CCameraCalculator::IteratorCalcuate ( double & h0, double &x0 ) {
    double a = overCameraHeight;
    double b = sideCameraHeight;
    double c = h0;
    double d = x0;

    x0 = (a * b * d - c * b * d) / (a * b - c * d);

	d /= 2;
    h0 = (a * b * c - a * c * d) / (a * b - c * d);
    /*
    double x = x0;
    double h = h0;
    double cameraLength = sideCameraHeight;
    double cameraHeight = overCameraHeight;

    for ( int i = 0; i < 10; i++ ) {
        h = ( cameraLength - x ) * h0 / cameraLength;

        x = ( cameraHeight - h ) * x0 / cameraHeight;
    }

    h0 = h;
    x0 = x;
    */
}