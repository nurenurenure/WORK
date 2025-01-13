#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <string>
#include <memory>

#ifdef _WIN32
#include <windows.h>
#include <commdlg.h>
#endif

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
        cv::cvtColor(image, image, cv::COLOR_GRAY2BGR); // ������� � BGR ��� ���������������
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

class ImageApp {
private:
    cv::Mat image;

    // ������� ���� ����� ���������� ����
    std::string openFileDialog() {
#ifdef _WIN32
        wchar_t filename[MAX_PATH] = L"";
        OPENFILENAMEW ofn;
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = NULL;
        ofn.lpstrFilter = L"����� �����������\0*.jpg;*.png;*.bmp;*.jpeg\0��� �����\0*.*\0";
        ofn.lpstrFile = filename;
        ofn.nMaxFile = MAX_PATH;
        ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
        ofn.lpstrDefExt = L"jpg";

        if (GetOpenFileNameW(&ofn)) {
            char result[MAX_PATH];
            WideCharToMultiByte(CP_UTF8, 0, filename, -1, result, MAX_PATH, NULL, NULL);
            return std::string(result);
        }
#endif
        return "";
    }

    // ��������� ���� ����� ���������� ����
    std::string saveFileDialog() {
#ifdef _WIN32
        wchar_t filename[MAX_PATH] = L"";
        OPENFILENAMEW ofn;
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = NULL;
        ofn.lpstrFilter = L"����� �����������\0*.jpg;*.png;*.bmp;*.jpeg\0��� �����\0*.*\0";
        ofn.lpstrFile = filename;
        ofn.nMaxFile = MAX_PATH;
        ofn.Flags = OFN_OVERWRITEPROMPT;
        ofn.lpstrDefExt = L"jpg";

        if (GetSaveFileNameW(&ofn)) {
            char result[MAX_PATH];
            WideCharToMultiByte(CP_UTF8, 0, filename, -1, result, MAX_PATH, NULL, NULL);
            return std::string(result);
        }
#endif
        return "";
    }

    // ��������� ��������� ������
    void applyFilter(int filterChoice) {
        if (image.empty()) {
            std::cerr << "������: ����������� �� ���������, ������ �� ����� ���� �������!" << std::endl;
            return;
        }

        std::unique_ptr<Filter> filter;
        switch (filterChoice) {
        case 1:
            filter = std::make_unique<GrayscaleFilter>();
            break;
        case 2:
            filter = std::make_unique<BlurFilter>();
            break;
        case 3:
            filter = std::make_unique<SharpenFilter>();
            break;
        default:
            std::cerr << "������: ������������ ����� �������!" << std::endl;
            return;
        }

        filter->apply(image);
        cv::imshow("����������� � ��������", image);
        cv::waitKey(0);
        cv::destroyAllWindows();
    }

public:
    void run() {
        while (true) {
            std::cout << "����:\n1. ������� �����������\n2. ��������� �����������\n3. ��������� ������ �������� ������\n4. ��������� ������ ��������\n5. ��������� ������ ��������\n6. �����\n������� ��� �����: ";
            int choice;
            std::cin >> choice;

            switch (choice) {
            case 1: {
                std::string filename = openFileDialog();
                if (!filename.empty()) {
                    image = cv::imread(filename);
                    if (image.empty()) {
                        std::cerr << "������: �� ������� ��������� �����������!" << std::endl;
                    }
                    else {
                        cv::imshow("�������� �����������", image);
                        cv::waitKey(0);
                        cv::destroyAllWindows();
                    }
                }
                else {
                    std::cerr << "������: ���� �� ������!" << std::endl;
                }
                break;
            }
            case 2: {
                if (image.empty()) {
                    std::cerr << "������: ������ ���������!" << std::endl;
                }
                else {
                    std::string filename = saveFileDialog();
                    if (!filename.empty()) {
                        if (cv::imwrite(filename, image)) {
                            std::cout << "����������� ������� ���������: " << filename << std::endl;
                        }
                        else {
                            std::cerr << "������: �� ������� ��������� �����������!" << std::endl;
                        }
                    }
                    else {
                        std::cerr << "������: ��� ����� ��� ���������� �� �������!" << std::endl;
                    }
                }
                break;
            }
            case 3:
                applyFilter(1);
                break;
            case 4:
                applyFilter(2);
                break;
            case 5:
                applyFilter(3);
                break;
            case 6:
                std::cout << "����� �� ���������." << std::endl;
                return;
            default:
                std::cerr << "������: ������������ �����!" << std::endl;
            }
        }
    }
};

int main() {
    setlocale(LC_ALL, "");
    ImageApp app;
    app.run();
    return 0;
}
