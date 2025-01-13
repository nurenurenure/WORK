#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#include <commdlg.h>
#endif

class ImageApp {
private:
    cv::Mat image;

    // ������� ���� ����� ���������� ����
    std::string openFileDialog() {
#ifdef _WIN32
        wchar_t filename[MAX_PATH] = L""; // ���������� wchar_t ��� ��������� Unicode
        OPENFILENAMEW ofn;                // ���������� ������ Unicode API
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = NULL;
        ofn.lpstrFilter = L"Image Files\0*.jpg;*.png;*.bmp;*.jpeg\0All Files\0*.*\0";
        ofn.lpstrFile = filename;
        ofn.nMaxFile = MAX_PATH;
        ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
        ofn.lpstrDefExt = L"jpg";

        if (GetOpenFileNameW(&ofn)) {
            // ����������� ������� ������ � std::string
            std::wstring ws(filename);
            return std::string(ws.begin(), ws.end());
        }
#endif
        return "";
    }

    // ��������� ���� ����� ���������� ����
    std::string saveFileDialog() {
#ifdef _WIN32
        wchar_t filename[MAX_PATH] = L""; // ���������� wchar_t ��� ��������� Unicode
        OPENFILENAMEW ofn;                // ���������� ������ Unicode API
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = NULL;
        ofn.lpstrFilter = L"Image Files\0*.jpg;*.png;*.bmp;*.jpeg\0All Files\0*.*\0";
        ofn.lpstrFile = filename;
        ofn.nMaxFile = MAX_PATH;
        ofn.Flags = OFN_OVERWRITEPROMPT;
        ofn.lpstrDefExt = L"jpg";

        if (GetSaveFileNameW(&ofn)) {
            // ����������� ������� ������ � std::string
            std::wstring ws(filename);
            return std::string(ws.begin(), ws.end());
        }
#endif
        return "";
    }

public:
    void run() {
        while (true) {
            std::cout << "����:\n1. ������� �����������\n2. ��������� �����������\n3. �����\n������� ��� �����: ";
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
                    std::cerr << "������: ��� ����������� ��� ����������!" << std::endl;
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
                        std::cerr << "������: ���� �� ������ ��� ����������!" << std::endl;
                    }
                }
                break;
            }
            case 3:
                std::cout << "����� �� ���������." << std::endl;
                return;
            default:
                std::cerr << "������: �������� �����!" << std::endl;
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
