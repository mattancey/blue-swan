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

// LookAndFeel pour le bouton carré stylé
class SquareToggleLookAndFeel : public juce::LookAndFeel_V4
{
public:
    void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour&, bool isMouseOverButton, bool isButtonDown) override
    {
        auto bounds = button.getLocalBounds().toFloat().reduced(2.0f);
        g.setColour(juce::Colours::black);
        g.fillRoundedRectangle(bounds, 6.0f);
        
        // Couleur de la bordure (Cyan du plug-in)
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
        g.setFont(juce::Font(juce::FontOptions(24.0f, juce::Font::bold))); // Grande police
        g.drawFittedText(button.getButtonText(), button.getLocalBounds(), juce::Justification::centred, 1);
    }
};
