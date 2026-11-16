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

// Menu déroulant robuste : s'ouvre vers le haut et ne bloque plus après la première utilisation
class CustomComboBox : public juce::ComboBox
{
public:
    void showPopup() override
    {
        if (isMenuOpen || !isEnabled()) return;
        isMenuOpen = true;

        juce::PopupMenu menu;
        for (int i = 1; i <= getNumItems(); ++i) {
            menu.addItem(i, getItemText(i - 1), true, i == getSelectedId());
        }

        auto options = juce::PopupMenu::Options()
            .withTargetComponent(this)
            .withPreferredPopupDirection(juce::PopupMenu::Options::PopupDirection::upwards)
            .withMinimumWidth(getWidth())
            .withMaximumNumColumns(1);

        juce::Component::SafePointer<CustomComboBox> safeThis(this);
        
        menu.showMenuAsync(options, [safeThis](int result) {
            if (safeThis == nullptr) return;
            safeThis->isMenuOpen = false;

            if (result != 0) {
                // On utilise callAsync pour laisser le menu se fermer complètement avant de mettre à jour
                juce::MessageManager::callAsync([safeThis, result] {
                    if (safeThis != nullptr) {
                        safeThis->setSelectedId(result, juce::sendNotificationSync);
                    }
                });
            }
        });
    }
private:
    bool isMenuOpen = false;
};
