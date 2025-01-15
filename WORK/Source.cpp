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
        cv::cvtColor(image, image, cv::COLOR_GRAY2BGR);
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

// Фильтр инверсии цвета
class InvertFilter : public Filter {
public:
    void apply(cv::Mat& image) override {
        image = cv::Scalar::all(255) - image; // Инвертируем цвета изображения
    }
};

// Класс для работы с изображениями
class ImageEditor {
private:
    cv::Mat image;
    double brightness = 1.0; // Начальная яркость
    double saturation = 1.0; // Начальная насыщенность

    // Функция обновления изображения
    void updateImageDisplay() {
        if (!image.empty() && image.channels() == 3) {
            cv::Mat temp = image.clone();
            applyBrightnessAndSaturation(temp);
            cv::imshow("Image", temp); // Отображаем изображение через OpenCV
            cv::waitKey(1);  // Обновляем окно
        }
        else {
            MessageBox(NULL, L"Invalid image format", L"Error", MB_OK | MB_ICONERROR);
        }
    }

    // Применение яркости и насыщенности
    void applyBrightnessAndSaturation(cv::Mat& img) {
        // Применяем изменение яркости
        img.convertTo(img, -1, brightness, 0);

        // Преобразуем в цветовое пространство HSV для изменения насыщенности
        cv::Mat hsvImage;
        cv::cvtColor(img, hsvImage, cv::COLOR_BGR2HSV);
        for (int y = 0; y < hsvImage.rows; y++) {
            for (int x = 0; x < hsvImage.cols; x++) {
                cv::Vec3b& pixel = hsvImage.at<cv::Vec3b>(y, x);
                pixel[1] = cv::saturate_cast<uchar>(pixel[1] * saturation); // Изменение насыщенности
            }
        }
        cv::cvtColor(hsvImage, img, cv::COLOR_HSV2BGR); // Преобразуем обратно в BGR
    }

public:
    // Открытие изображения
    void openImage(const std::wstring& path) {
        image = cv::imread(cv::String(path.begin(), path.end()), cv::IMREAD_COLOR);
        if (image.empty()) {
            MessageBox(NULL, L"Failed to load image", L"Error", MB_OK | MB_ICONERROR);
        }
        else {
            updateImageDisplay();
        }
    }

    // Сохранение изображения
    void saveImage(const std::wstring& path) {
        if (!image.empty()) {
            cv::imwrite(cv::String(path.begin(), path.end()), image);
        }
        else {
            MessageBox(NULL, L"No image to save", L"Error", MB_OK | MB_ICONERROR);
        }
    }

    // Применение фильтра
    void applyFilter(std::unique_ptr<Filter> filter) {
        if (!image.empty() && filter) {
            try {
                filter->apply(image);
                updateImageDisplay();
            }
            catch (const std::exception& e) {
                MessageBox(NULL, std::wstring(L"Filter error: " + std::wstring(e.what(), e.what() + strlen(e.what()))).c_str(), L"Error", MB_OK | MB_ICONERROR);
            }
        }
        else {
            MessageBox(NULL, L"No image to apply filter", L"Error", MB_OK | MB_ICONERROR);
        }
    }

    // Изменение яркости
    void setBrightness(double value) {
        brightness = value;
        updateImageDisplay();
    }

    // Изменение насыщенности
    void setSaturation(double value) {
        saturation = value;
        updateImageDisplay();
    }
};

// Класс для ползунка яркости
class BrightnessSlider : public Fl_Slider {
private:
    ImageEditor* editor;

public:
    BrightnessSlider(int x, int y, int w, int h, const char* label, ImageEditor* editor)
        : Fl_Slider(x, y, w, h, label), editor(editor) {
        type(FL_HORIZONTAL);
        range(0, 2);
        value(1);
        callback(sliderCallback, this);
    }

    static void sliderCallback(Fl_Widget* widget, void* data) {
        BrightnessSlider* slider = static_cast<BrightnessSlider*>(widget);
        slider->editor->setBrightness(slider->value());
    }
};

// Класс для ползунка насыщенности
class SaturationSlider : public Fl_Slider {
private:
    ImageEditor* editor;

public:
    SaturationSlider(int x, int y, int w, int h, const char* label, ImageEditor* editor)
        : Fl_Slider(x, y, w, h, label), editor(editor) {
        type(FL_HORIZONTAL);
        range(0, 2);
        value(1);
        callback(sliderCallback, this);
    }

    static void sliderCallback(Fl_Widget* widget, void* data) {
        SaturationSlider* slider = static_cast<SaturationSlider*>(widget);
        slider->editor->setSaturation(slider->value());
    }
};

// Открытие файла через диалог
std::wstring openFileDialog(const wchar_t* filter) {
    wchar_t fileName[MAX_PATH] = { 0 };
    OPENFILENAME ofn = { 0 };
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = NULL;
    ofn.lpstrFilter = filter;
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

    if (GetOpenFileName(&ofn)) {
        return fileName;
    }
    return L"";
}

// Сохранение файла через диалог
std::wstring saveFileDialog(const wchar_t* filter) {
    wchar_t fileName[MAX_PATH] = L"output.jpg";
    OPENFILENAME ofn = { 0 };
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = NULL;
    ofn.lpstrFilter = filter;
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_OVERWRITEPROMPT;

    if (GetSaveFileName(&ofn)) {
        return fileName;
    }
    return L"";
}

// Колбэки для FLTK кнопок
void openImageCallback(Fl_Widget*, void* data) {
    ImageEditor* editor = static_cast<ImageEditor*>(data);
    std::wstring path = openFileDialog(L"Images (*.jpg;*.png)\0*.jpg;*.png\0");
    if (!path.empty()) {
        editor->openImage(path);
    }
}

void saveImageCallback(Fl_Widget*, void* data) {
    ImageEditor* editor = static_cast<ImageEditor*>(data);
    std::wstring path = saveFileDialog(L"JPEG Files (*.jpg)\0*.jpg\0PNG Files (*.png)\0*.png\0");
    if (!path.empty()) {
        editor->saveImage(path);
    }
}

void applyGrayscaleCallback(Fl_Widget*, void* data) {
    ImageEditor* editor = static_cast<ImageEditor*>(data);
    editor->applyFilter(std::make_unique<GrayscaleFilter>());
}

void applyBlurCallback(Fl_Widget*, void* data) {
    ImageEditor* editor = static_cast<ImageEditor*>(data);
    editor->applyFilter(std::make_unique<BlurFilter>());
}

void applySharpenCallback(Fl_Widget*, void* data) {
    ImageEditor* editor = static_cast<ImageEditor*>(data);
    editor->applyFilter(std::make_unique<SharpenFilter>());
}

void applyInvertCallback(Fl_Widget*, void* data) {
    ImageEditor* editor = static_cast<ImageEditor*>(data);
    editor->applyFilter(std::make_unique<InvertFilter>());
}

int main() {
    // Окно для панели с кнопками
    Fl_Window* buttonWindow = new Fl_Window(800, 300, "Button Panel");

    ImageEditor editor; // Создаем объект для работы с изображениями

    Fl_Button* openButton = new Fl_Button(10, 10, 120, 30, "Open");
    openButton->callback(openImageCallback, &editor);

    Fl_Button* saveButton = new Fl_Button(140, 10, 120, 30, "Save");
    saveButton->callback(saveImageCallback, &editor);

    Fl_Button* grayscaleButton = new Fl_Button(270, 10, 120, 30, "Grayscale");
    grayscaleButton->callback(applyGrayscaleCallback, &editor);

    Fl_Button* blurButton = new Fl_Button(400, 10, 120, 30, "Blur");
    blurButton->callback(applyBlurCallback, &editor);

    Fl_Button* sharpenButton = new Fl_Button(530, 10, 120, 30, "Sharpen");
    sharpenButton->callback(applySharpenCallback, &editor);

    // Кнопка для инверсии цвета
    Fl_Button* invertButton = new Fl_Button(660, 10, 120, 30, "Invert Colors");
    invertButton->callback(applyInvertCallback, &editor);

    // Ползунки для изменения яркости и насыщенности
    BrightnessSlider* brightnessSlider = new BrightnessSlider(10, 50, 760, 20, "Brightness", &editor);
    SaturationSlider* saturationSlider = new SaturationSlider(10, 90, 760, 20, "Saturation", &editor);

    buttonWindow->end();
    buttonWindow->show();

    return Fl::run(); // Запуск FLTK
}
