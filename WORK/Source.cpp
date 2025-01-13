#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <string>
#include <memory>

#ifdef _WIN32
#include <windows.h>
#include <commdlg.h>
#endif

// Абстрактный базовый класс для фильтров
class Filter {
public:
    virtual void apply(cv::Mat& image) = 0;
    virtual ~Filter() {}
};

// Фильтр для преобразования в оттенки серого
class GrayscaleFilter : public Filter {
public:
    void apply(cv::Mat& image) override {
        cv::cvtColor(image, image, cv::COLOR_BGR2GRAY);
        cv::cvtColor(image, image, cv::COLOR_GRAY2BGR); // Обратно в BGR для консистентности
    }
};

// Фильтр размытия
class BlurFilter : public Filter {
public:
    void apply(cv::Mat& image) override {
        cv::GaussianBlur(image, image, cv::Size(15, 15), 0);
    }
};

// Фильтр увеличения резкости
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

    // Открыть файл через диалоговое окно
    std::string openFileDialog() {
#ifdef _WIN32
        wchar_t filename[MAX_PATH] = L"";
        OPENFILENAMEW ofn;
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = NULL;
        ofn.lpstrFilter = L"Файлы изображений\0*.jpg;*.png;*.bmp;*.jpeg\0Все файлы\0*.*\0";
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

    // Сохранить файл через диалоговое окно
    std::string saveFileDialog() {
#ifdef _WIN32
        wchar_t filename[MAX_PATH] = L"";
        OPENFILENAMEW ofn;
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = NULL;
        ofn.lpstrFilter = L"Файлы изображений\0*.jpg;*.png;*.bmp;*.jpeg\0Все файлы\0*.*\0";
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

    // Применить выбранный фильтр
    void applyFilter(int filterChoice) {
        if (image.empty()) {
            std::cerr << "Ошибка: изображение не загружено, фильтр не может быть применён!" << std::endl;
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
            std::cerr << "Ошибка: неправильный выбор фильтра!" << std::endl;
            return;
        }

        filter->apply(image);
        cv::imshow("Изображение с фильтром", image);
        cv::waitKey(0);
        cv::destroyAllWindows();
    }

public:
    void run() {
        while (true) {
            std::cout << "Меню:\n1. Открыть изображение\n2. Сохранить изображение\n3. Применить фильтр оттенков серого\n4. Применить фильтр размытия\n5. Применить фильтр резкости\n6. Выйти\nВведите ваш выбор: ";
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
                    std::cerr << "Ошибка: нечего сохранять!" << std::endl;
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
                        std::cerr << "Ошибка: имя файла для сохранения не выбрано!" << std::endl;
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
                std::cout << "Выход из программы." << std::endl;
                return;
            default:
                std::cerr << "Ошибка: неправильный выбор!" << std::endl;
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
