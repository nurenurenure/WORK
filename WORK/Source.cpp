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
#include "Filters.h"


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


// Класс для работы с изображениями
class ImageEditor {
private:
        // Сохранение текущего состояния изображения в стек
    void saveState() {
        history.push(image.clone()); // Сохраняем копию текущего состояния
    }
    cv::Mat image;
    std::stack<cv::Mat> history; // Стек для хранения истории изменений
    double brightness = 1.0; // Начальная яркость
    double saturation = 1.0; // Начальная насыщенность
    double scaleFactor = 1.0; // Коэффициент масштабирования
    int r = 0, g = 0, b = 0; // Значения RGB для отдельного управления каналами
    cv::Mat overlayImage; // Для хранения наложенного изображения
    double transparency = 1.0; // Прозрачность наложенного изображения

    // Функция обновления изображения
    void updateImageDisplay() {
        if (!image.empty() && image.channels() == 3) {
            cv::Mat temp = image.clone();
            applyBrightnessAndSaturation(temp);
            applyRGBChannels(temp);

            if (!overlayImage.empty()) {
                cv::Mat overlayResized;
                cv::resize(overlayImage, overlayResized, temp.size());
                cv::addWeighted(temp, 1.0, overlayResized, transparency, 0, temp); // Наложение изображения с прозрачностью
            }

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
    cv::Mat getCurrentImage() const {
        return image.clone();
    }
    int getRed() const { return r; }
    int getGreen() const { return g; }
    int getBlue() const { return b; }
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

    // Добавление изображения поверх
    void addOverlayImage(const std::wstring& path, double alpha) {
        cv::Mat overlay = cv::imread(cv::String(path.begin(), path.end()), cv::IMREAD_COLOR);
        if (!overlay.empty()) {
            overlayImage = overlay;
            transparency = alpha;
            updateImageDisplay();
        }
        else {
            MessageBox(NULL, L"Failed to load overlay image", L"Error", MB_OK | MB_ICONERROR);
        }
    }
};


// Класс для извлечения палитры изображения
class Palette {
public:
    static std::vector<cv::Vec3b> extractPalette(const cv::Mat& image, int numColors) {
        if (image.empty() || image.channels() != 3) {
            throw std::runtime_error("Invalid image for palette extraction");
        }

        cv::Mat data;
        image.convertTo(data, CV_32F); // Преобразуем изображение в формат с плавающей точкой
        data = data.reshape(1, image.rows * image.cols); // Перекладываем пиксели в один столбец

        // Используем алгоритм кластеризации k-means
        cv::Mat labels, centers;
        cv::kmeans(data, numColors, labels,
            cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::MAX_ITER, 10, 1.0),
            3, cv::KMEANS_PP_CENTERS, centers);

        centers.convertTo(centers, CV_8UC1); // Преобразуем центры обратно в целочисленный формат
        centers = centers.reshape(3); // Преобразуем обратно в трехканальный формат

        // Преобразуем центры кластеров в вектор цветов
        std::vector<cv::Vec3b> palette;
        for (int i = 0; i < centers.rows; ++i) {
            palette.push_back(centers.at<cv::Vec3b>(i, 0));
        }
        return palette;
    }
};

// Добавление функции для отображения палитры
void displayPalette(const std::vector<cv::Vec3b>& palette) {
    const int swatchSize = 50;
    cv::Mat paletteImage(swatchSize, palette.size() * swatchSize, CV_8UC3);

    for (size_t i = 0; i < palette.size(); ++i) {
        cv::Rect rect(i * swatchSize, 0, swatchSize, swatchSize);
        cv::rectangle(paletteImage, rect, cv::Scalar(palette[i][0], palette[i][1], palette[i][2]), -1);
    }

    cv::imshow("Palette", paletteImage);
    cv::waitKey(1);
}

// Колбэк для извлечения и отображения палитры
void extractPaletteCallback(Fl_Widget*, void* data) {
    ImageEditor* editor = static_cast<ImageEditor*>(data);

    try {
        const int numColors = 5; // Количество цветов в палитре
        auto palette = Palette::extractPalette(editor->getCurrentImage(), numColors);
        displayPalette(palette);
    }
    catch (const std::exception& e) {
        MessageBox(NULL, std::wstring(L"Error extracting palette: " + std::wstring(e.what(), e.what() + strlen(e.what()))).c_str(), L"Error", MB_OK | MB_ICONERROR);
    }
}


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







class ButtonPanel {
public:
    ButtonPanel(Fl_Window* parent, int x, int y, int w, int h, ImageEditor* editor) {
        Fl_Button* openButton = new Fl_Button(x, y, w, h, "Open");
        openButton->callback(openImageCallback, editor);

        Fl_Button* saveButton = new Fl_Button(x + w + 10, y, w, h, "Save");
        saveButton->callback(saveImageCallback, editor);

        Fl_Button* undoButton = new Fl_Button(x + 2 * (w + 10), y, w, h, "Undo");
        undoButton->callback(undoCallback, editor);

        Fl_Button* grayscaleButton = new Fl_Button(x, y + h + 10, w, h, "Grayscale");
        grayscaleButton->callback(applyGrayscaleCallback, editor);

        Fl_Button* blurButton = new Fl_Button(x + w + 10, y + h + 10, w, h, "Blur");
        blurButton->callback(applyBlurCallback, editor);

        Fl_Button* sharpenButton = new Fl_Button(x + 2 * (w + 10), y + h + 10, w, h, "Sharpen");
        sharpenButton->callback(applySharpenCallback, editor);

        Fl_Button* invertButton = new Fl_Button(x + 3 * (w + 10), y + h + 10, w, h, "Invert Colors");
        invertButton->callback(applyInvertCallback, editor);

        Fl_Button* mirrorButton = new Fl_Button(x + 4 * (w + 10), y + h + 10, w, h, "Mirror");
        mirrorButton->callback(applyMirrorCallback, editor);

        Fl_Button* overlayButton = new Fl_Button(x + 5 * (w + 10), y + h + 10, w, h, "Overlay Image");
        overlayButton->callback([](Fl_Widget*, void* data) {
            ImageEditor* editor = static_cast<ImageEditor*>(data);
            std::wstring path = openFileDialog(L"Images (*.jpg;*.png)\0*.jpg;*.png\0");
            if (!path.empty()) {
                double alpha = 0.5; // Example transparency
                editor->addOverlayImage(path, alpha);
            }
            }, editor);

        Fl_Button* paletteButton = new Fl_Button(x, parent->h() - h - 10, w, h, "Extract Palette");
        paletteButton->callback(extractPaletteCallback, editor);
    }
};

class SliderPanel {
public:
    SliderPanel(Fl_Window* parent, int x, int y, int w, int h, ImageEditor* editor) {
        int spacing = 30; // Adjusted spacing between sliders

        Fl_Slider* brightnessSlider = new Fl_Slider(x, y, w, h, "Brightness");
        brightnessSlider->type(FL_HORIZONTAL);
        brightnessSlider->minimum(0.0);
        brightnessSlider->maximum(2.0);
        brightnessSlider->value(1.0);
        brightnessSlider->callback([](Fl_Widget* widget, void* data) {
            ImageEditor* editor = static_cast<ImageEditor*>(data);
            editor->setBrightness(static_cast<Fl_Slider*>(widget)->value());
            }, editor);

        Fl_Slider* saturationSlider = new Fl_Slider(x, y + h + spacing, w, h, "Saturation");
        saturationSlider->type(FL_HORIZONTAL);
        saturationSlider->minimum(0.0);
        saturationSlider->maximum(2.0);
        saturationSlider->value(1.0);
        saturationSlider->callback([](Fl_Widget* widget, void* data) {
            ImageEditor* editor = static_cast<ImageEditor*>(data);
            editor->setSaturation(static_cast<Fl_Slider*>(widget)->value());
            }, editor);

        Fl_Slider* scaleSlider = new Fl_Slider(x, y + 2 * (h + spacing), w, h, "Scale");
        scaleSlider->type(FL_HORIZONTAL);
        scaleSlider->minimum(0.1);
        scaleSlider->maximum(3.0);
        scaleSlider->value(1.0);
        scaleSlider->callback([](Fl_Widget* widget, void* data) {
            ImageEditor* editor = static_cast<ImageEditor*>(data);
            editor->setScale(static_cast<Fl_Slider*>(widget)->value());
            }, editor);

        Fl_Slider* redSlider = new Fl_Slider(x, y + 3 * (h + spacing), w, h, "Red Channel");
        redSlider->type(FL_HORIZONTAL);
        redSlider->minimum(-255);
        redSlider->maximum(255);
        redSlider->value(0);
        redSlider->callback([](Fl_Widget* widget, void* data) {
            ImageEditor* editor = static_cast<ImageEditor*>(data);
            int red = static_cast<Fl_Slider*>(widget)->value();
            int green = editor->getGreen();
            int blue = editor->getBlue();
            editor->setRGB(red, green, blue);
            }, editor);

        Fl_Slider* greenSlider = new Fl_Slider(x, y + 4 * (h + spacing), w, h, "Green Channel");
        greenSlider->type(FL_HORIZONTAL);
        greenSlider->minimum(-255);
        greenSlider->maximum(255);
        greenSlider->value(0);
        greenSlider->callback([](Fl_Widget* widget, void* data) {
            ImageEditor* editor = static_cast<ImageEditor*>(data);
            int red = editor->getRed();
            int green = static_cast<Fl_Slider*>(widget)->value();
            int blue = editor->getBlue();
            editor->setRGB(red, green, blue);
            }, editor);

        Fl_Slider* blueSlider = new Fl_Slider(x, y + 5 * (h + spacing), w, h, "Blue Channel");
        blueSlider->type(FL_HORIZONTAL);
        blueSlider->minimum(-255);
        blueSlider->maximum(255);
        blueSlider->value(0);
        blueSlider->callback([](Fl_Widget* widget, void* data) {
            ImageEditor* editor = static_cast<ImageEditor*>(data);
            int red = editor->getRed();
            int green = editor->getGreen();
            int blue = static_cast<Fl_Slider*>(widget)->value();
            editor->setRGB(red, green, blue);
            }, editor);
    }
};

class MainWindow {
public:
    MainWindow() {
        Fl_Window* window = new Fl_Window(800, 400, "Image Editor");
        ImageEditor editor;

        ButtonPanel buttonPanel(window, 10, 10, 120, 30, &editor);
        SliderPanel sliderPanel(window, 10, 90, 760, 20, &editor);

        window->end();
        window->show();
        Fl::run();
    }
};

int main() {
    MainWindow mainWindow;
    return 0;
}

