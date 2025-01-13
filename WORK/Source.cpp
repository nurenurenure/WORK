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

    // Открыть файл через диалоговое окно
    std::string openFileDialog() {
#ifdef _WIN32
        wchar_t filename[MAX_PATH] = L""; // Используем wchar_t для поддержки Unicode
        OPENFILENAMEW ofn;                // Используем версию Unicode API
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = NULL;
        ofn.lpstrFilter = L"Image Files\0*.jpg;*.png;*.bmp;*.jpeg\0All Files\0*.*\0";
        ofn.lpstrFile = filename;
        ofn.nMaxFile = MAX_PATH;
        ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
        ofn.lpstrDefExt = L"jpg";

        if (GetOpenFileNameW(&ofn)) {
            // Преобразуем широкую строку в std::string
            std::wstring ws(filename);
            return std::string(ws.begin(), ws.end());
        }
#endif
        return "";
    }

    // Сохранить файл через диалоговое окно
    std::string saveFileDialog() {
#ifdef _WIN32
        wchar_t filename[MAX_PATH] = L""; // Используем wchar_t для поддержки Unicode
        OPENFILENAMEW ofn;                // Используем версию Unicode API
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = NULL;
        ofn.lpstrFilter = L"Image Files\0*.jpg;*.png;*.bmp;*.jpeg\0All Files\0*.*\0";
        ofn.lpstrFile = filename;
        ofn.nMaxFile = MAX_PATH;
        ofn.Flags = OFN_OVERWRITEPROMPT;
        ofn.lpstrDefExt = L"jpg";

        if (GetSaveFileNameW(&ofn)) {
            // Преобразуем широкую строку в std::string
            std::wstring ws(filename);
            return std::string(ws.begin(), ws.end());
        }
#endif
        return "";
    }

public:
    void run() {
        while (true) {
            std::cout << "Меню:\n1. Открыть изображение\n2. Сохранить изображение\n3. Выйти\nВведите ваш выбор: ";
            int choice;
            std::cin >> choice;

            switch (choice) {
            case 1: {
                std::string filename = openFileDialog();
                if (!filename.empty()) {
                    image = cv::imread(filename);
                    if (image.empty()) {
                        std::cerr << "Ошибка: не удалось загрузить изображение!" << std::endl;
                    }
                    else {
                        cv::imshow("Открытое изображение", image);
                        cv::waitKey(0);
                        cv::destroyAllWindows();
                    }
                }
                else {
                    std::cerr << "Ошибка: файл не выбран!" << std::endl;
                }
                break;
            }
            case 2: {
                if (image.empty()) {
                    std::cerr << "Ошибка: нет изображения для сохранения!" << std::endl;
                }
                else {
                    std::string filename = saveFileDialog();
                    if (!filename.empty()) {
                        if (cv::imwrite(filename, image)) {
                            std::cout << "Изображение успешно сохранено: " << filename << std::endl;
                        }
                        else {
                            std::cerr << "Ошибка: не удалось сохранить изображение!" << std::endl;
                        }
                    }
                    else {
                        std::cerr << "Ошибка: файл не выбран для сохранения!" << std::endl;
                    }
                }
                break;
            }
            case 3:
                std::cout << "Выход из программы." << std::endl;
                return;
            default:
                std::cerr << "Ошибка: неверный выбор!" << std::endl;
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
