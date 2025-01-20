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