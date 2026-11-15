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

// Force le menu à s'ouvrir vers le haut (Correction Pointeur avec '->')
class CustomComboBox : public juce::ComboBox
{
public:
    void showPopup() override
    {
        auto* rootMenu = getRootMenu();
        if (rootMenu == nullptr) return;

        auto options = juce::PopupMenu::Options()
            .withTargetComponent(this)
            .withPreferredPopupDirection(juce::PopupMenu::Options::PopupDirection::upwards);
            
        rootMenu->showMenuAsync(options, [this](int result) {
            if (result != 0) setSelectedId(result, juce::sendNotificationSync);
        });
    }
};
