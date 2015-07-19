#pragma once
#include <string>
#include <vector>
#include <stdint.h>
#include <list>

namespace ImageMeasure {
    struct Point2I {
        int x;
        int y;
    };

    struct Point2D {
        double x;
        double y;
    };

    struct Point2F {
        float x;
        float y;
    };

    struct Catelog {
        int id;
        std::wstring name;
        std::wstring desc;
    };

    struct MeasureImage {
        int id;
        int pid;
        int imageType;
        double weight;
        bool canMeasure;
        std::wstring imagePath;
    };

    struct MeasureInfo {
        int pid;
        int id;
        std::wstring name;
        std::wstring desc;
        Point2I start;
        Point2I end;
        double length;
        int side;
    };

    struct Product {
        int id;
        Catelog type;
        std::wstring name;
        double length;
        double height;
        double width;
        double weight;
        std::wstring overlookPath1;
        std::wstring sidelookPath1;
        std::wstring overlookPath2;
        std::wstring sidelookPath2;
        std::wstring overlookPathMin;
        std::wstring sidelookPathMin;
        std::vector<std::wstring> panoPath;
        std::vector<MeasureImage> measureImages;
        std::vector<MeasureInfo> measureInfo;
        bool isReverse;
        int cameraHeight;
        int isHeight;
    };

#pragma pack(push,1)
    struct TransmissionHeader {
        char secret[8];
        uint8_t height;
        uint32_t isReverse;
        uint32_t type;
        uint32_t length;
        uint32_t weight;
        uint64_t id;
        uint64_t gid;
    };
#pragma pack(pop)

    struct ImageHolder {
        std::vector<ImageMeasure::MeasureImage> measureImages;
        std::list<std::wstring> panoPath;
    };
    struct Line {
        CPoint start;
        CPoint end;
    };
};