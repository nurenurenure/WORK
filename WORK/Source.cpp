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

// ������� ��� �������� ����� ����� ���������� ����
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

// ������� ��� ���������� ����� ����� ���������� ����
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

// ����� ��� ������ � �������������
class ImageEditor {
private:
    cv::Mat image;
    std::stack<cv::Mat> history; // ���� ��� �������� ������� ���������
    double brightness = 1.0; // ��������� �������
    double saturation = 1.0; // ��������� ������������

    // ������� ���������� �����������
    void updateImageDisplay() {
        if (!image.empty() && image.channels() == 3) {
            cv::Mat temp = image.clone();
            applyBrightnessAndSaturation(temp);
            cv::imshow("Image", temp); // ���������� ����������� ����� OpenCV
            cv::waitKey(1);  // ��������� ����
        }
        else {
            MessageBox(NULL, L"Invalid image format", L"Error", MB_OK | MB_ICONERROR);
        }
    }

    // ���������� ������� � ������������
    void applyBrightnessAndSaturation(cv::Mat& img) {
        // ��������� ��������� �������
        img.convertTo(img, -1, brightness, 0);

        // ����������� � �������� ������������ HSV ��� ��������� ������������
        cv::Mat hsvImage;
        cv::cvtColor(img, hsvImage, cv::COLOR_BGR2HSV);
        for (int y = 0; y < hsvImage.rows; y++) {
            for (int x = 0; x < hsvImage.cols; x++) {
                cv::Vec3b& pixel = hsvImage.at<cv::Vec3b>(y, x);
                pixel[1] = cv::saturate_cast<uchar>(pixel[1] * saturation); // ��������� ������������
            }
        }
        cv::cvtColor(hsvImage, img, cv::COLOR_HSV2BGR); // ����������� ������� � BGR
    }

public:
    // �������� �����������
    void openImage(const std::wstring& path) {
        image = cv::imread(cv::String(path.begin(), path.end()), cv::IMREAD_COLOR);
        if (image.empty()) {
            MessageBox(NULL, L"Failed to load image", L"Error", MB_OK | MB_ICONERROR);
        }
        else {
            saveState(); // ��������� ��������� ��� �������� �����������
            updateImageDisplay();
        }
    }

    // ���������� �����������
    void saveImage(const std::wstring& path) {
        if (!image.empty()) {
            cv::imwrite(cv::String(path.begin(), path.end()), image);
        }
        else {
            MessageBox(NULL, L"No image to save", L"Error", MB_OK | MB_ICONERROR);
        }
    }

    // ���������� �������
    void applyFilter(std::unique_ptr<Filter> filter) {
        if (!image.empty() && filter) {
            try {
                saveState(); // ��������� ��������� ����� ����������� �������
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

    // ��������� �������
    void setBrightness(double value) {
        saveState(); // ��������� ��������� ����� ���������� �������
        brightness = value;
        updateImageDisplay();
    }

    // ��������� ������������
    void setSaturation(double value) {
        saveState(); // ��������� ��������� ����� ���������� ������������
        saturation = value;
        updateImageDisplay();
    }

    // ������ ���������� ��������
    void undo() {
        if (!history.empty()) {
            image = history.top(); // ��������� ��������� ���������
            history.pop(); // ������� ��� �� �����
            updateImageDisplay();
        }
        else {
            MessageBox(NULL, L"No action to undo", L"Error", MB_OK | MB_ICONERROR);
        }
    }

private:
    // ���������� �������� ��������� ����������� � ����
    void saveState() {
        history.push(image.clone()); // ��������� ����� �������� ���������
    }
};

// ������ ��� ��������� ������� � ������������
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

// ������ ��� ������ Undo
void undoCallback(Fl_Widget*, void* data) {
    ImageEditor* editor = static_cast<ImageEditor*>(data);
    editor->undo();
}

// ������� ��� FLTK ������
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
    // ���� ��� ������ � ��������
    Fl_Window* buttonWindow = new Fl_Window(800, 350, "Image Editor");

    ImageEditor editor; // ������� ������ ��� ������ � �������������

    Fl_Button* openButton = new Fl_Button(10, 10, 120, 30, "Open");
    openButton->callback(openImageCallback, &editor);

    Fl_Button* saveButton = new Fl_Button(140, 10, 120, 30, "Save");
    saveButton->callback(saveImageCallback, &editor);

    Fl_Button* undoButton = new Fl_Button(270, 10, 120, 30, "Undo");
    undoButton->callback(undoCallback, &editor);

    // ������ ��� ���������� ��������
    Fl_Button* grayscaleButton = new Fl_Button(10, 50, 120, 30, "Grayscale");
    grayscaleButton->callback(applyGrayscaleCallback, &editor);

    Fl_Button* blurButton = new Fl_Button(140, 50, 120, 30, "Blur");
    blurButton->callback(applyBlurCallback, &editor);

    Fl_Button* sharpenButton = new Fl_Button(270, 50, 120, 30, "Sharpen");
    sharpenButton->callback(applySharpenCallback, &editor);

    Fl_Button* invertButton = new Fl_Button(400, 50, 120, 30, "Invert Colors");
    invertButton->callback(applyInvertCallback, &editor);

    // �������� ��� ��������� ������� � ������������
    BrightnessSlider* brightnessSlider = new BrightnessSlider(10, 90, 760, 20, "Brightness", &editor);
    SaturationSlider* saturationSlider = new SaturationSlider(10, 130, 760, 20, "Saturation", &editor);

    buttonWindow->end();
    buttonWindow->show();

    return Fl::run(); // ������ FLTK
}
