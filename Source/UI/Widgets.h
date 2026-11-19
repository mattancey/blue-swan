#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

inline void setupRotarySlider(juce::Slider& s, juce::Label& l, const juce::String& name, juce::Component& parent)
{
    s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    parent.addAndMakeVisible(s);
    l.setText(name, juce::dontSendNotification);
    l.setJustificationType(juce::Justification::centred);
    l.setColour(juce::Label::textColourId, juce::Colours::white);
    parent.addAndMakeVisible(l);
}

class CustomComboBox : public juce::ComboBox
{
public:
    void showPopup() override
    {
        juce::PopupMenu m;
        for (int i = 1; i <= getNumItems(); ++i) {
            m.addItem(i, getItemText(i - 1), true, i == getSelectedId());
        }
        auto options = juce::PopupMenu::Options()
            .withTargetComponent(this)
            .withPreferredPopupDirection(juce::PopupMenu::Options::PopupDirection::upwards)
            .withMinimumWidth(getWidth())
            .withMaximumNumColumns(1);
        juce::Component::SafePointer<CustomComboBox> safeThis(this);
        m.showMenuAsync(options, [safeThis](int result) {
            if (safeThis == nullptr) return;
            safeThis->hidePopup(); 
            if (result > 0) {
                juce::MessageManager::callAsync([safeThis, result] {
                    if (safeThis != nullptr) safeThis->setSelectedId(result, juce::sendNotificationSync);
                });
            }
        });
    }
};

class SquareToggleLookAndFeel : public juce::LookAndFeel_V4
{
public:
    void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour&, bool isMouseOverButton, bool isButtonDown) override
    {
        auto bounds = button.getLocalBounds().toFloat().reduced(2.0f);
        g.setColour(juce::Colours::black);
        g.fillRoundedRectangle(bounds, 6.0f);
        
        g.setColour(juce::Colours::cyan);
        g.drawRoundedRectangle(bounds, 6.0f, 2.0f);
        
        if (isMouseOverButton || isButtonDown) {
            g.setColour(juce::Colours::white.withAlpha(0.2f));
            g.fillRoundedRectangle(bounds, 6.0f);
        }
    }

    void drawButtonText(juce::Graphics& g, juce::TextButton& button, bool, bool) override
    {
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(juce::FontOptions(24.0f, juce::Font::bold)));
        g.drawFittedText(button.getButtonText(), button.getLocalBounds(), juce::Justification::centred, 1);
    }
};

// Nouveau LookAndFeel pour le Thumb des matrices
class MatrixLookAndFeel : public juce::LookAndFeel_V4
{
public:
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPosProportional, float rotaryStartAngle,
                          float rotaryEndAngle, juce::Slider& slider) override
    {
        auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(2.0f);
        auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
        auto centre = bounds.getCentre();
        auto angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

        // Dessin du cercle de fond
        g.setColour(juce::Colours::darkgrey);
        g.drawEllipse(bounds, 2.0f);

        // Le Thumb (petit bouton)
        bool isActive = std::abs(slider.getValue()) > 0.001f; 
        g.setColour(isActive ? juce::Colours::aquamarine : juce::Colours::cyan);
        
        // On dessine un petit cercle au bout de la ligne
        float thumbRadius = radius * 0.15f;
        float thumbX = centre.x + std::sin(angle) * (radius * 0.8f);
        float thumbY = centre.y - std::cos(angle) * (radius * 0.8f);
        g.fillEllipse(thumbX - thumbRadius, thumbY - thumbRadius, thumbRadius * 2.0f, thumbRadius * 2.0f);
    }
};
