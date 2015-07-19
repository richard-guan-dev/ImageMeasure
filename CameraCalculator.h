#pragma once
#include "Types.h"
#include "opencv2/opencv.hpp"
#include "pcl/visualization/cloud_viewer.h"
#include "pcl/io/io.h"
#include "pcl/io/pcd_io.h"

class CCameraCalculator {
  public:
    CCameraCalculator();
    ~CCameraCalculator();
    void CalculateLocation ( const ImageMeasure::Point2I &src, ImageMeasure::Point2D &des );
    void CalculateSize ( const TCHAR* overlookPath, const TCHAR* sidelookPath, double &width, double &height, double &length );
    void CalculateLength ( const TCHAR* overlookBinPath, const TCHAR* sidelookBinPath, const ImageMeasure::Point2D from, const ImageMeasure::Point2D to, double &length, int type, ImageMeasure::Product &product );
    void RotateImage ( cv::Mat& image, double degree );
    void CutOverlookImage ( cv::Mat& image );
    void CutSidelookImage ( cv::Mat& image );
    void ChromaKey ( cv::Mat &front, cv::Mat &background, cv::Mat &dst, cv::Vec3i &rgb , bool isOver );
    void CalculateChromaKeyValue ( cv::Mat &background, cv::Vec3i &rgb );
    void GetMinAreaRect ( cv::Mat &src, cv::RotatedRect &rect, std::vector<cv::Point2f>&points );
    void GetBoundRect ( cv::Mat&src, cv::Rect& rect, std::vector<cv::Point2f>&points );
    void IteratorCalcuate ( double & height, double &x );
    void CutSideAndOver(const ImageMeasure::Product &product);
    void ImageImporve(const char* over);
};

