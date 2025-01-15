#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_JPEG_Image.H>
#include <FL/Fl_File_Chooser.H>
#include <opencv2/opencv.hpp>
#include <memory>

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

    // Обновить отображение изображения
    void updateImageDisplay() {
        if (!image.empty()) {
            cv::Mat temp;
            cv::cvtColor(image, temp, cv::COLOR_BGR2RGB);

            Fl_RGB_Image* flImage = new Fl_RGB_Image(temp.data, temp.cols, temp.rows, 3);
            imageBox->image(flImage);
            imageBox->redraw();
        }
    }

public:
    ImageEditor(Fl_Box* box) : imageBox(box) {}

    // Открыть изображение из файла
    void openImage(const char* path) {
        image = cv::imread(path);
        if (image.empty()) {
            fl_message("Failed to load image");
        }
        else {
            updateImageDisplay();
        }
    }

    // Сохранить изображение в файл
    void saveImage(const char* path) {
        if (!image.empty()) {
            cv::imwrite(path, image);
        }
        else {
            fl_message("No image to save");
        }
    }

    // Применить фильтр
    void applyFilter(std::unique_ptr<Filter> filter) {
        if (!image.empty() && filter) {
            filter->apply(image);
            updateImageDisplay();
        }
    }
};

// Обработчик для кнопки открытия изображения
void openImageCallback(Fl_Widget*, void* data) {
    ImageEditor* editor = static_cast<ImageEditor*>(data);
    const char* path = fl_file_chooser("Open Image", "Images (*.jpg;*.png)", "");
    if (path) {
        editor->openImage(path);
    }
}

// Обработчик для кнопки сохранения изображения
void saveImageCallback(Fl_Widget*, void* data) {
    ImageEditor* editor = static_cast<ImageEditor*>(data);
    const char* path = fl_file_chooser("Save Image", "*.jpg", "output.jpg");
    if (path) {
        editor->saveImage(path);
    }
}

// Обработчик для применения фильтра "оттенки серого"
void applyGrayscaleCallback(Fl_Widget*, void* data) {
    ImageEditor* editor = static_cast<ImageEditor*>(data);
    editor->applyFilter(std::make_unique<GrayscaleFilter>());
}

// Обработчик для применения фильтра размытия
void applyBlurCallback(Fl_Widget*, void* data) {
    ImageEditor* editor = static_cast<ImageEditor*>(data);
    editor->applyFilter(std::make_unique<BlurFilter>());
}

// Обработчик для применения фильтра увеличения резкости
void applySharpenCallback(Fl_Widget*, void* data) {
    ImageEditor* editor = static_cast<ImageEditor*>(data);
    editor->applyFilter(std::make_unique<SharpenFilter>());
}

int main() {
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
