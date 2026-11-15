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

// Forcer les menus à s'ouvrir vers le HAUT
class CustomComboBox : public juce::ComboBox
{
public:
    void showPopup() override
    {
        juce::PopupMenu m;
        m.addItemList(getItems(), 1);
        
        auto area = getScreenBounds();
        // On place la zone cible AU-DESSUS du bouton pour forcer l'ouverture vers le haut
        auto targetArea = juce::Rectangle<int>(area.getX(), area.getY() - area.getHeight(), area.getWidth(), area.getHeight());
        
        m.showMenuAsync(juce::PopupMenu::Options()
            .withTargetScreenArea(targetArea)
            .withMinimumWidth(getWidth())
            .withMaximumNumColumns(1),
            [this](int result) {
                if (result > 0) setSelectedId(result);
            });
    }
};
