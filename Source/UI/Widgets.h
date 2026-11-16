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

// Force le menu à s'ouvrir vers le haut ET corrige le bug de blocage
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

        // On stocke un pointeur sécurisé pour le callback
        juce::Component::SafePointer<CustomComboBox> safeThis(this);

        m.showMenuAsync(options, [safeThis](int result) {
            if (safeThis == nullptr) return;
            
            // LA SOLUTION EST ICI : On force JUCE à oublier que le menu était ouvert
            safeThis->hidePopup(); 
            
            if (result > 0) {
                // On laisse le menu se fermer avant de mettre à jour la valeur
                juce::MessageManager::callAsync([safeThis, result] {
                    if (safeThis != nullptr) {
                        safeThis->setSelectedId(result, juce::sendNotificationSync);
                    }
                });
            }
        });
    }
};
