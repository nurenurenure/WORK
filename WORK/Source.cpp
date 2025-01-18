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

private:
    // Сохранение текущего состояния изображения в стек
    void saveState() {
        history.push(image.clone()); // Сохраняем копию текущего состояния
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

    // Кнопка для добавления наложенного изображения
    Fl_Button* overlayButton = new Fl_Button(660, 50, 120, 30, "Overlay Image");
    overlayButton->callback([](Fl_Widget*, void* data) {
        ImageEditor* editor = static_cast<ImageEditor*>(data);
        std::wstring path = openFileDialog(L"Images (*.jpg;*.png)\0*.jpg;*.png\0");
        if (!path.empty()) {
            double alpha = 0.5; // Пример прозрачности
            editor->addOverlayImage(path, alpha);
        }
        }, &editor);

    // Ползунки для изменения яркости и насыщенности
    Fl_Slider* brightnessSlider = new Fl_Slider(10, 90, 760, 20, "Brightness");
    brightnessSlider->type(FL_HORIZONTAL);
    brightnessSlider->minimum(0.0);
    brightnessSlider->maximum(2.0);
    brightnessSlider->value(1.0);
    brightnessSlider->callback([](Fl_Widget* widget, void* data) {
        ImageEditor* editor = static_cast<ImageEditor*>(data);
        editor->setBrightness(static_cast<Fl_Slider*>(widget)->value());
        }, &editor);

    Fl_Slider* saturationSlider = new Fl_Slider(10, 130, 760, 20, "Saturation");
    saturationSlider->type(FL_HORIZONTAL);
    saturationSlider->minimum(0.0);
    saturationSlider->maximum(2.0);
    saturationSlider->value(1.0);
    saturationSlider->callback([](Fl_Widget* widget, void* data) {
        ImageEditor* editor = static_cast<ImageEditor*>(data);
        editor->setSaturation(static_cast<Fl_Slider*>(widget)->value());
        }, &editor);

    // Новый ползунок для масштабирования
    Fl_Slider* scaleSlider = new Fl_Slider(10, 170, 760, 20, "Scale");
    scaleSlider->type(FL_HORIZONTAL);
    scaleSlider->minimum(0.1);
    scaleSlider->maximum(3.0);
    scaleSlider->value(1.0);
    scaleSlider->callback([](Fl_Widget* widget, void* data) {
        ImageEditor* editor = static_cast<ImageEditor*>(data);
        editor->setScale(static_cast<Fl_Slider*>(widget)->value());
        }, &editor);
    // Ползунки для редактирования каналов RGB
    Fl_Slider* redSlider = new Fl_Slider(10, 210, 760, 20, "Red Channel");
    redSlider->type(FL_HORIZONTAL);
    redSlider->minimum(-255);
    redSlider->maximum(255);
    redSlider->value(0);
    redSlider->callback([](Fl_Widget* widget, void* data) {
        ImageEditor* editor = static_cast<ImageEditor*>(data);
        int red = static_cast<Fl_Slider*>(widget)->value();
        int green = editor->getGreen(); // Получаем текущие значения для других каналов
        int blue = editor->getBlue(); // Получаем текущие значения для синего канала
        editor->setRGB(red, green, blue); // Устанавливаем новые значения
        }, &editor);

    Fl_Slider* greenSlider = new Fl_Slider(10, 250, 760, 20, "Green Channel");
    greenSlider->type(FL_HORIZONTAL);
    greenSlider->minimum(-255);
    greenSlider->maximum(255);
    greenSlider->value(0);
    greenSlider->callback([](Fl_Widget* widget, void* data) {
        ImageEditor* editor = static_cast<ImageEditor*>(data);
        int red = editor->getRed(); // Получаем текущие значения для красного канала
        int green = static_cast<Fl_Slider*>(widget)->value();
        int blue = editor->getBlue(); // Получаем текущие значения для синего канала
        editor->setRGB(red, green, blue); // Устанавливаем новые значения
        }, &editor);

    Fl_Slider* blueSlider = new Fl_Slider(10, 290, 760, 20, "Blue Channel");
    blueSlider->type(FL_HORIZONTAL);
    blueSlider->minimum(-255);
    blueSlider->maximum(255);
    blueSlider->value(0);
    blueSlider->callback([](Fl_Widget* widget, void* data) {
        ImageEditor* editor = static_cast<ImageEditor*>(data);
        int red = editor->getRed(); // Получаем текущие значения для красного канала
        int green = editor->getGreen(); // Получаем текущие значения для зеленого канала
        int blue = static_cast<Fl_Slider*>(widget)->value();
        editor->setRGB(red, green, blue); // Устанавливаем новые значения
        }, &editor);

    buttonWindow->end();
    buttonWindow->show();
    return Fl::run();
}
