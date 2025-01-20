#include <opencv2/opencv.hpp>
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Slider.H>
#include <windows.h>
#include <commdlg.h>
#include <memory>
#include <string>
#include <iostream>
#include <stack>
// ����������� ������� ����� ��� ��������
class Filter {
public:
    virtual void apply(cv::Mat& image) = 0;
    virtual ~Filter() {}
};

// ������ ��� �������������� � ������� ������
class GrayscaleFilter : public Filter {
public:
    void apply(cv::Mat& image) override {
        cv::cvtColor(image, image, cv::COLOR_BGR2GRAY);
        cv::cvtColor(image, image, cv::COLOR_GRAY2BGR);
    }
};

// ������ ��������
class BlurFilter : public Filter {
public:
    void apply(cv::Mat& image) override {
        cv::GaussianBlur(image, image, cv::Size(15, 15), 0);
    }
};

// ������ ���������� ��������
class SharpenFilter : public Filter {
public:
    void apply(cv::Mat& image) override {
        cv::Mat kernel = (cv::Mat_<float>(3, 3) <<
            0, -1, 0,
            -1, 5, -1,
            0, -1, 0);
        cv::filter2D(image, image, -1, kernel);
    }
};

// ������ �������� �����
class InvertFilter : public Filter {
public:
    void apply(cv::Mat& image) override {
        image = cv::Scalar::all(255) - image; // ����������� ����� �����������
    }
};

// ������ ����������� ���������
class MirrorFilter : public Filter {
public:
    void apply(cv::Mat& image) override {
        cv::flip(image, image, 1); // ��������� �� �����������
    }
};