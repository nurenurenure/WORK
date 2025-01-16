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

// Функция для открытия файла через диалоговое окно
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

// Функция для сохранения файла через диалоговое окно
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

// Фильтр зеркального отражения
class MirrorFilter : public Filter {
public:
    void apply(cv::Mat& image) override {
        cv::flip(image, image, 1); // Отражение по горизонтали
    }
};

// Класс для работы с изображениями
class ImageEditor {
private:
    cv::Mat image;
    std::stack<cv::Mat> history; // Стек для хранения истории изменений
    double brightness = 1.0; // Начальная яркость
    double saturation = 1.0; // Начальная насыщенность
    double scaleFactor = 1.0; // Коэффициент масштабирования
    int r = 0, g = 0, b = 0; // Значения RGB для отдельного управления каналами

    // Функция обновления изображения
    void updateImageDisplay() {
        if (!image.empty() && image.channels() == 3) {
            cv::Mat temp = image.clone();
            applyBrightnessAndSaturation(temp);
            applyRGBChannels(temp);
            cv::resize(temp, temp, cv::Size(), scaleFactor, scaleFactor); // Масштабируем изображение
            cv::imshow("Image", temp); // Отображаем изображение через OpenCV
            cv::waitKey(1);  // Обновляем окно
        }
        else {
            MessageBox(NULL, L"Invalid image format", L"Error", MB_OK | MB_ICONERROR);
        }
    }

    // Применение яркости и насыщенности
    void applyBrightnessAndSaturation(cv::Mat& img) {
        img.convertTo(img, -1, brightness, 0);

        cv::Mat hsvImage;
        cv::cvtColor(img, hsvImage, cv::COLOR_BGR2HSV);
        for (int y = 0; y < hsvImage.rows; y++) {
            for (int x = 0; x < hsvImage.cols; x++) {
                cv::Vec3b& pixel = hsvImage.at<cv::Vec3b>(y, x);
                pixel[1] = cv::saturate_cast<uchar>(pixel[1] * saturation);
            }
        }
        cv::cvtColor(hsvImage, img, cv::COLOR_HSV2BGR);
    }

    // Применение изменений в отдельных каналах RGB
    void applyRGBChannels(cv::Mat& img) {
        if (r != 0 || g != 0 || b != 0) {
            cv::Mat channels[3];
            cv::split(img, channels);
            channels[0] += r; // R-канал
            channels[1] += g; // G-канал
            channels[2] += b; // B-канал
            cv::merge(channels, 3, img);
        }
    }

public:
    // Открытие изображения
    void openImage(const std::wstring& path) {
        image = cv::imread(cv::String(path.begin(), path.end()), cv::IMREAD_COLOR);
        if (image.empty()) {
            MessageBox(NULL, L"Failed to load image", L"Error", MB_OK | MB_ICONERROR);
        }
        else {
            saveState(); // Сохраняем состояние при открытии изображения
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
                saveState(); // Сохраняем состояние перед применением фильтра
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
        saveState(); // Сохраняем состояние перед изменением яркости
        brightness = value;
        updateImageDisplay();
    }

    // Изменение насыщенности
    void setSaturation(double value) {
        saveState(); // Сохраняем состояние перед изменением насыщенности
        saturation = value;
        updateImageDisplay();
    }

    // Изменение масштаба
    void setScale(double value) {
        saveState(); // Сохраняем состояние перед изменением масштаба
        scaleFactor = value;
        updateImageDisplay(); // Обновляем отображение с учетом нового масштаба
    }

    // Отмена последнего действия
    void undo() {
        if (!history.empty()) {
            image = history.top(); // Извлекаем последнее состояние
            history.pop(); // Убираем его из стека
            updateImageDisplay();
        }
        else {
            MessageBox(NULL, L"No action to undo", L"Error", MB_OK | MB_ICONERROR);
        }
    }

    // Установить значения для RGB каналов
    void setRGB(int red, int green, int blue) {
        saveState();
        r = red;
        g = green;
        b = blue;
        updateImageDisplay();
    }

private:
    // Сохранение текущего состояния изображения в стек
    void saveState() {
        history.push(image.clone()); // Сохраняем копию текущего состояния
    }
};

// Классы для ползунков яркости и насыщенности
class BrightnessSlider : public Fl_Slider {
private:
    ImageEditor* editor;

public:
    BrightnessSlider(int x, int y, int w, int h, const char* label, ImageEditor* editor)
        : Fl_Slider(x, y, w, h, label), editor(editor) {
        type(FL_HORIZONTAL);
        range(0.0, 2.0); // Диапазон яркости от 0.0 до 2.0
        value(1.0); // Начальная яркость
        callback(sliderCallback, this);
    }

    static void sliderCallback(Fl_Widget* widget, void* data) {
        BrightnessSlider* slider = static_cast<BrightnessSlider*>(widget);
        slider->editor->setBrightness(slider->value());
    }
};

class SaturationSlider : public Fl_Slider {
private:
    ImageEditor* editor;

public:
    SaturationSlider(int x, int y, int w, int h, const char* label, ImageEditor* editor)
        : Fl_Slider(x, y, w, h, label), editor(editor) {
        type(FL_HORIZONTAL);
        range(0.0, 2.0); // Диапазон насыщенности от 0.0 до 2.0
        value(1.0); // Начальная насыщенность
        callback(sliderCallback, this);
    }

    static void sliderCallback(Fl_Widget* widget, void* data) {
        SaturationSlider* slider = static_cast<SaturationSlider*>(widget);
        slider->editor->setSaturation(slider->value());
    }
};

// Новый класс для ползунка масштабирования
class ScaleSlider : public Fl_Slider {
private:
    ImageEditor* editor;

public:
    ScaleSlider(int x, int y, int w, int h, const char* label, ImageEditor* editor)
        : Fl_Slider(x, y, w, h, label), editor(editor) {
        type(FL_HORIZONTAL);
        range(0.1, 2.0); // Диапазон масштабирования от 0.1 до 2.0
        value(1.0); // Начальный коэффициент масштаба
        callback(sliderCallback, this);
    }

    static void sliderCallback(Fl_Widget* widget, void* data) {
        ScaleSlider* slider = static_cast<ScaleSlider*>(widget);
        slider->editor->setScale(slider->value());
    }
};

// Новый класс для ползунков RGB каналов
class RGBSlider : public Fl_Window {
private:
    ImageEditor* editor;
    Fl_Slider* rSlider;
    Fl_Slider* gSlider;
    Fl_Slider* bSlider;

public:
    RGBSlider(int x, int y, int w, int h, ImageEditor* editor)
        : Fl_Window(x, y, w, h), editor(editor) {
        rSlider = new Fl_Slider(10, 10, w - 20, 20, "Red");
        rSlider->type(FL_HORIZONTAL);
        rSlider->range(-100, 100);
        rSlider->value(0);
        rSlider->callback(sliderCallback, this);

        gSlider = new Fl_Slider(10, 40, w - 20, 20, "Green");
        gSlider->type(FL_HORIZONTAL);
        gSlider->range(-100, 100);
        gSlider->value(0);
        gSlider->callback(sliderCallback, this);

        bSlider = new Fl_Slider(10, 70, w - 20, 20, "Blue");
        bSlider->type(FL_HORIZONTAL);
        bSlider->range(-100, 100);
        bSlider->value(0);
        bSlider->callback(sliderCallback, this);

        end();
    }

    static void sliderCallback(Fl_Widget* widget, void* data) {
        RGBSlider* rgbSlider = static_cast<RGBSlider*>(data);
        int r = rgbSlider->rSlider->value();
        int g = rgbSlider->gSlider->value();
        int b = rgbSlider->bSlider->value();
        rgbSlider->editor->setRGB(r, g, b);
    }
};

// Колбэк для кнопки Undo
void undoCallback(Fl_Widget*, void* data) {
    ImageEditor* editor = static_cast<ImageEditor*>(data);
    editor->undo();
}

// Колбэки для кнопок
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

void applyMirrorCallback(Fl_Widget*, void* data) {
    ImageEditor* editor = static_cast<ImageEditor*>(data);
    editor->applyFilter(std::make_unique<MirrorFilter>());
}

int main() {
    // Окно для панели с кнопками
    Fl_Window* buttonWindow = new Fl_Window(800, 400, "Image Editor");

    ImageEditor editor; // Создаем объект для работы с изображениями

    Fl_Button* openButton = new Fl_Button(10, 10, 120, 30, "Open");
    openButton->callback(openImageCallback, &editor);

    Fl_Button* saveButton = new Fl_Button(140, 10, 120, 30, "Save");
    saveButton->callback(saveImageCallback, &editor);

    Fl_Button* undoButton = new Fl_Button(270, 10, 120, 30, "Undo");
    undoButton->callback(undoCallback, &editor);

    // Кнопки для применения фильтров
    Fl_Button* grayscaleButton = new Fl_Button(10, 50, 120, 30, "Grayscale");
    grayscaleButton->callback(applyGrayscaleCallback, &editor);

    Fl_Button* blurButton = new Fl_Button(140, 50, 120, 30, "Blur");
    blurButton->callback(applyBlurCallback, &editor);

    Fl_Button* sharpenButton = new Fl_Button(270, 50, 120, 30, "Sharpen");
    sharpenButton->callback(applySharpenCallback, &editor);

    Fl_Button* invertButton = new Fl_Button(400, 50, 120, 30, "Invert Colors");
    invertButton->callback(applyInvertCallback, &editor);

    // Новая кнопка для зеркального отражения
    Fl_Button* mirrorButton = new Fl_Button(530, 50, 120, 30, "Mirror");
    mirrorButton->callback(applyMirrorCallback, &editor);

    // Ползунки для изменения яркости и насыщенности
    BrightnessSlider* brightnessSlider = new BrightnessSlider(10, 90, 760, 20, "Brightness", &editor);
    SaturationSlider* saturationSlider = new SaturationSlider(10, 130, 760, 20, "Saturation", &editor);

    // Новый ползунок для масштабирования
    ScaleSlider* scaleSlider = new ScaleSlider(10, 170, 760, 20, "Scale", &editor);

    // Кнопка для открытия окна RGB
    Fl_Button* rgbButton = new Fl_Button(10, 210, 120, 30, "Adjust RGB");
    rgbButton->callback([](Fl_Widget*, void* data) {
        RGBSlider* rgbWindow = new RGBSlider(200, 200, 300, 150, static_cast<ImageEditor*>(data));
        rgbWindow->show();
        }, &editor);

    buttonWindow->end();
    buttonWindow->show();

    return Fl::run();
}
