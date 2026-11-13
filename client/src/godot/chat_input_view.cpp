#include "chat_input_view.hpp"
#include <godot_cpp/classes/input_event_key.hpp>
#include <godot_cpp/classes/style_box_empty.hpp>
#include <godot_cpp/classes/system_font.hpp>
#include <godot_cpp/classes/text_server.hpp>
#include <godot_cpp/classes/viewport.hpp>
using namespace godot;
void Mir3ChatInput::_bind_methods() {
    ADD_SIGNAL(MethodInfo("chat_requested",PropertyInfo(Variant::STRING,"text")));
}
void Mir3ChatInput::_ready() {
    set_name("ChatInput");set_z_index(500);
    // Limit is ANSI bytes, enforced by the model, not LineEdit's Unicode count.
    set_max_length(0);set_context_menu_enabled(false);
    for (const auto* style:{"normal","focus","read_only"}) {
        Ref<StyleBoxEmpty> empty;empty.instantiate();add_theme_stylebox_override(style,empty);
    }
    connect("text_changed",callable_mp(this,&Mir3ChatInput::changed));
    connect("text_submitted",callable_mp(this,&Mir3ChatInput::submitted));
    layout(false);
}
void Mir3ChatInput::configure(String face) {
    Ref<SystemFont> font;font.instantiate();
    PackedStringArray names;names.push_back(face);font->set_font_names(names);
    font->set_font_weight(mir3::ChatInputSpec::font_weight);
    font->set_antialiasing(TextServer::FONT_ANTIALIASING_NONE);
    font->set_subpixel_positioning(TextServer::SUBPIXEL_POSITIONING_DISABLED);
    add_theme_font_override("font",font);
    add_theme_font_size_override("font_size",mir3::ChatInputSpec::font_pixels);
}
void Mir3ChatInput::layout(bool expanded) {
    set_position(Vector2(185,expanded?534:575));set_size(Vector2(expanded?432:344,15));
}
void Mir3ChatInput::focus_chat(String prefix,bool replace) {
    if (replace) {
        const auto utf8=prefix.utf8();
        set_text(String::utf8(model.set({utf8.get_data(),static_cast<std::size_t>(utf8.length())}).c_str()));
    }
    grab_focus();set_caret_column(get_text().length());
}
void Mir3ChatInput::changed(String) {
    const auto current=get_text();
    const auto utf8=current.utf8();
    const auto edit=model.edit({utf8.get_data(),static_cast<std::size_t>(utf8.length())},get_caret_column());
    const auto accepted=String::utf8(edit.text.c_str());
    if (accepted!=current) {
        set_text(accepted);set_caret_column(edit.caret);
        if (edit.selection_from>=0 && edit.selection_to>edit.selection_from) select(edit.selection_from,edit.selection_to);
    }
}
String Mir3ChatInput::limited_text() const {
    const auto utf8=get_text().utf8();
    return String::utf8(mir3::limit_chat_input(
        {utf8.get_data(),static_cast<std::size_t>(utf8.length())},model.byte_limit).c_str());
}
void Mir3ChatInput::submitted(String value) {
    const auto utf8=value.utf8();
    const auto accepted=model.set({utf8.get_data(),static_cast<std::size_t>(utf8.length())});
    if (!accepted.empty()) emit_signal("chat_requested",String::utf8(accepted.c_str()));
    model.set("");set_text("");release_focus();
}
void Mir3ChatInput::_gui_input(const Ref<InputEvent>& event) {
    Ref<InputEventKey> key=event;
    if (key.is_valid() && key->is_pressed() && key->get_keycode()==KEY_ESCAPE) { release_focus();accept_event(); }
}
void Mir3ChatInput::_unhandled_key_input(const Ref<InputEvent>& event) {
    Ref<InputEventKey> key=event;
    if (key.is_null() || !key->is_pressed() || key->is_echo() || get_viewport()->gui_get_focus_owner()) return;
    const auto code=key->get_keycode();
    if (code!=KEY_ENTER && code!=KEY_KP_ENTER && code!=KEY_SPACE) return;
    focus_chat(String::utf8(mode.prefix().data(),mode.prefix().size()),mode.mode!=0);
    get_viewport()->set_input_as_handled();
}
