#include "hud_view.hpp"
#include <godot_cpp/godot.hpp>
using namespace godot;
void initialize_client(ModuleInitializationLevel level) {
    if (level==MODULE_INITIALIZATION_LEVEL_SCENE) ClassDB::register_class<Mir3HudView>();
}
void uninitialize_client(ModuleInitializationLevel) {}
extern "C" GDExtensionBool GDE_EXPORT mir3_client_init(GDExtensionInterfaceGetProcAddress get_proc_address,GDExtensionClassLibraryPtr library,GDExtensionInitialization* initialization) {
    GDExtensionBinding::InitObject init(get_proc_address,library,initialization);
    init.register_initializer(initialize_client);
    init.register_terminator(uninitialize_client);
    init.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);
    return init.init();
}
