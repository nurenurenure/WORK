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

// ������ ����������� ���������
class MirrorFilter : public Filter {
public:
    void apply(cv::Mat& image) override {
        cv::flip(image, image, 1); // ��������� �� �����������
    }
};

// ����� ��� ������ � �������������
class ImageEditor {
private:
    cv::Mat image;
    std::stack<cv::Mat> history; // ���� ��� �������� ������� ���������
    double brightness = 1.0; // ��������� �������
    double saturation = 1.0; // ��������� ������������
    double scaleFactor = 1.0; // ����������� ���������������
    int r = 0, g = 0, b = 0; // �������� RGB ��� ���������� ���������� ��������
    cv::Mat overlayImage; // ��� �������� ����������� �����������
    double transparency = 1.0; // ������������ ����������� �����������
    bool showRGBSliders = false; // ���� ��� ����������� ��������� RGB

    // ������� ���������� �����������
    void updateImageDisplay() {
        if (!image.empty() && image.channels() == 3) {
            cv::Mat temp = image.clone();
            applyBrightnessAndSaturation(temp);
            applyRGBChannels(temp);

            if (!overlayImage.empty()) {
                cv::Mat overlayResized;
                cv::resize(overlayImage, overlayResized, temp.size());
                cv::addWeighted(temp, 1.0, overlayResized, transparency, 0, temp); // ��������� ����������� � �������������
            }

            cv::resize(temp, temp, cv::Size(), scaleFactor, scaleFactor); // ������������ �����������
            cv::imshow("Image", temp); // ���������� ����������� ����� OpenCV
            cv::waitKey(1);  // ��������� ����
        }
        else {
            MessageBox(NULL, L"Invalid image format", L"Error", MB_OK | MB_ICONERROR);
        }
    }

    // ���������� ������� � ������������
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

    // ���������� ��������� � ��������� ������� RGB
    void applyRGBChannels(cv::Mat& img) {
        if (r != 0 || g != 0 || b != 0) {
            cv::Mat channels[3];
            cv::split(img, channels);
            channels[0] += r; // R-�����
            channels[1] += g; // G-�����
            channels[2] += b; // B-�����
            cv::merge(channels, 3, img);
        }
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

    // ��������� ��������
    void setScale(double value) {
        saveState(); // ��������� ��������� ����� ���������� ��������
        scaleFactor = value;
        updateImageDisplay(); // ��������� ����������� � ������ ������ ��������
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

    // ���������� �������� ��� RGB �������
    void setRGB(int red, int green, int blue) {
        saveState();
        r = red;
        g = green;
        b = blue;
        updateImageDisplay();
    }

    // ���������� ����������� ������
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

    // �������� ��������� ��� RGB
    void toggleRGBSliders() {
        showRGBSliders = !showRGBSliders;
    }

    bool isRGBSlidersVisible() const {
        return showRGBSliders;
    }

private:
    // ���������� �������� ��������� ����������� � ����
    void saveState() {
        history.push(image.clone()); // ��������� ����� �������� ���������
    }
};

// ������ ��� ������ Undo
void undoCallback(Fl_Widget*, void* data) {
    ImageEditor* editor = static_cast<ImageEditor*>(data);
    editor->undo();
}

// ������� ��� ������
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

// ������ ��� �����������/������� ��������� RGB
void toggleRGBSlidersCallback(Fl_Widget*, void* data) {
    ImageEditor* editor = static_cast<ImageEditor*>(data);
    editor->toggleRGBSliders();
}

int main() {
    // ���� ��� ������ � ��������
    Fl_Window* buttonWindow = new Fl_Window(800, 500, "Image Editor");

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

    // ����� ������ ��� ����������� ���������
    Fl_Button* mirrorButton = new Fl_Button(530, 50, 120, 30, "Mirror");
    mirrorButton->callback(applyMirrorCallback, &editor);

    // ������ ��� ���������� ����������� �����������
    Fl_Button* overlayButton = new Fl_Button(660, 50, 120, 30, "Overlay Image");
    overlayButton->callback([](Fl_Widget*, void* data) {
        ImageEditor* editor = static_cast<ImageEditor*>(data);
        std::wstring path = openFileDialog(L"Images (*.jpg;*.png)\0*.jpg;*.png\0");
        if (!path.empty()) {
            double alpha = 0.5; // ������ ������������
            editor->addOverlayImage(path, alpha);
        }
        }, &editor);

    // ����� ������ ��� ����������� ��������� RGB
    Fl_Button* rgbSlidersButton = new Fl_Button(10, 90, 120, 30, "RGB Sliders");
    rgbSlidersButton->callback(toggleRGBSlidersCallback, &editor);

    // �������� ��� ��������� ������� � ������������
    Fl_Slider* brightnessSlider = new Fl_Slider(10, 130, 760, 20, "Brightness");
    brightnessSlider->type(FL_HORIZONTAL);
    brightnessSlider->minimum(0.0);
    brightnessSlider->maximum(2.0);
    brightnessSlider->value(1.0);
    brightnessSlider->callback([](Fl_Widget* widget, void* data) {
        ImageEditor* editor = static_cast<ImageEditor*>(data);
        editor->setBrightness(static_cast<Fl_Slider*>(widget)->value());
        }, &editor);

    Fl_Slider* saturationSlider = new Fl_Slider(10, 170, 760, 20, "Saturation");
    saturationSlider->type(FL_HORIZONTAL);
    saturationSlider->minimum(0.0);
    saturationSlider->maximum(2.0);
    saturationSlider->value(1.0);
    saturationSlider->callback([](Fl_Widget* widget, void* data) {
        ImageEditor* editor = static_cast<ImageEditor*>(data);
        editor->setSaturation(static_cast<Fl_Slider*>(widget)->value());
        }, &editor);

    // �������� ��� ���������������
    Fl_Slider* scaleSlider = new Fl_Slider(10, 210, 760, 20, "Scale");
    scaleSlider->type(FL_HORIZONTAL);
    scaleSlider->minimum(0.1);
    scaleSlider->maximum(2.0);
    scaleSlider->value(1.0);
    scaleSlider->callback([](Fl_Widget* widget, void* data) {
        ImageEditor* editor = static_cast<ImageEditor*>(data);
        editor->setScale(static_cast<Fl_Slider*>(widget)->value());
        }, &editor);

    // ������ �������� ��� RGB �������, ���� ���������
    Fl_Slider* rSlider = new Fl_Slider(10, 250, 760, 20, "Red");
    rSlider->type(FL_HORIZONTAL);
    rSlider->minimum(-255);
    rSlider->maximum(255);
    rSlider->value(0);
    rSlider->callback([](Fl_Widget* widget, void* data) {
        ImageEditor* editor = static_cast<ImageEditor*>(data);
        editor->setRGB(static_cast<Fl_Slider*>(widget)->value(), 0, 0);
        }, &editor);

    Fl_Slider* gSlider = new Fl_Slider(10, 290, 760, 20, "Green");
    gSlider->type(FL_HORIZONTAL);
    gSlider->minimum(-255);
    gSlider->maximum(255);
    gSlider->value(0);
    gSlider->callback([](Fl_Widget* widget, void* data) {
        ImageEditor* editor = static_cast<ImageEditor*>(data);
        editor->setRGB(0, static_cast<Fl_Slider*>(widget)->value(), 0);
        }, &editor);

    Fl_Slider* bSlider = new Fl_Slider(10, 330, 760, 20, "Blue");
    bSlider->type(FL_HORIZONTAL);
    bSlider->minimum(-255);
    bSlider->maximum(255);
    bSlider->value(0);
    bSlider->callback([](Fl_Widget* widget, void* data) {
        ImageEditor* editor = static_cast<ImageEditor*>(data);
        editor->setRGB(0, 0, static_cast<Fl_Slider*>(widget)->value());
        }, &editor);

    // �������� �������� RGB �� ���������
    rSlider->hide();
    gSlider->hide();
    bSlider->hide();

    buttonWindow->end();
    buttonWindow->show();

    return Fl::run();
}
