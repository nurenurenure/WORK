#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <opencv2/opencv.hpp>
#include <windows.h>
#include <commdlg.h>
#include <memory>
#include <string>
#include <opencv2/core.hpp>
#include <opencv2/core/utils/logger.hpp>


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

class ImageEditor {
private:
    cv::Mat image;
    Fl_Box* imageBox;

    void updateImageDisplay() {
        if (!image.empty() && image.channels() == 3) {
            cv::Mat temp;
            cv::cvtColor(image, temp, cv::COLOR_BGR2RGB);

            Fl_Image* oldImage = imageBox->image();  // Сохраняем старое изображение
            auto* flImage = new Fl_RGB_Image(temp.data, temp.cols, temp.rows, 3);
            imageBox->image(flImage);

            if (oldImage) {
                delete oldImage;  // Удаляем старое изображение
            }

            imageBox->redraw();
        }
        else {
            MessageBox(NULL, L"Invalid image format", L"Error", MB_OK | MB_ICONERROR);
        }
    }

public:
    ImageEditor(Fl_Box* box) : imageBox(box) {}

    void openImage(const std::wstring& path) {
        image = cv::imread(cv::String(path.begin(), path.end()), cv::IMREAD_COLOR);
        if (image.empty()) {
            MessageBox(NULL, L"Failed to load image", L"Error", MB_OK | MB_ICONERROR);
        }
        else {
            updateImageDisplay();
        }
    }

    void saveImage(const std::wstring& path) {
        if (!image.empty()) {
            cv::imwrite(cv::String(path.begin(), path.end()), image);
        }
        else {
            MessageBox(NULL, L"No image to save", L"Error", MB_OK | MB_ICONERROR);
        }
    }

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
};

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

int main() {
    cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_SILENT);
    Fl_Window* window = new Fl_Window(800, 600, "Image Editor");

    Fl_Box* imageBox = new Fl_Box(10, 10, 780, 480);
    imageBox->box(FL_BORDER_BOX);

    ImageEditor editor(imageBox);

    Fl_Button* openButton = new Fl_Button(10, 500, 120, 30, "Open");
    openButton->callback(openImageCallback, &editor);

    Fl_Button* saveButton = new Fl_Button(140, 500, 120, 30, "Save");
    saveButton->callback(saveImageCallback, &editor);

    Fl_Button* grayscaleButton = new Fl_Button(270, 500, 120, 30, "Grayscale");
    grayscaleButton->callback(applyGrayscaleCallback, &editor);

    Fl_Button* blurButton = new Fl_Button(400, 500, 120, 30, "Blur");
    blurButton->callback(applyBlurCallback, &editor);

    Fl_Button* sharpenButton = new Fl_Button(530, 500, 120, 30, "Sharpen");
    sharpenButton->callback(applySharpenCallback, &editor);

    window->end();
    window->show();

    return Fl::run();
}
