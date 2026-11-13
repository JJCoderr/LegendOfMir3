#pragma once
#include "chat/chat_input.hpp"
#include <godot_cpp/classes/line_edit.hpp>

class Mir3ChatInput : public godot::LineEdit {
    GDCLASS(Mir3ChatInput,godot::LineEdit)
public:
    void _ready() override;
    void _gui_input(const godot::Ref<godot::InputEvent>& event) override;
    void _unhandled_key_input(const godot::Ref<godot::InputEvent>& event) override;
    void configure(godot::String face);
    void layout(bool expanded);
    void focus_chat(godot::String prefix,bool replace);
    mir3::ChatMode mode;
protected:
    static void _bind_methods();
private:
    mir3::ChatInput model;
    void changed(godot::String value);
    void submitted(godot::String value);
};
