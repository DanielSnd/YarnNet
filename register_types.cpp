#include "register_types.h"

void initialize_yarnnet_module(ModuleInitializationLevel p_level) {
#ifdef TOOLS_ENABLED
	if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR) {
	}
#endif

 	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
 			return;
   }
 	ClassDB::register_class<YNet>();
	ClassDB::register_class<YNetPropertySyncer>();

	// Bind your singleton.
	Engine::get_singleton()->add_singleton(Engine::Singleton("YNet", memnew(YNet)));

	ClassDB::register_class<YnetMultiplayerPeer>();
}

void uninitialize_yarnnet_module(ModuleInitializationLevel p_level) {
 	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
 			return;
   }

	if (Engine::get_singleton()->has_singleton("YNet")) {
		Engine::get_singleton()->remove_singleton("YNet");
	}

   // Nothing to do here in this example.
}