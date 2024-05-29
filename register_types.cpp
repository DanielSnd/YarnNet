#include "register_types.h"

// This is your singleton reference.
static YNet* YNetPtr;

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
	// Initialize your singleton.
	YNetPtr = memnew(YNet);

	// Bind your singleton.
	Engine::get_singleton()->add_singleton(Engine::Singleton("YNet", YNet::get_singleton()));

	ClassDB::register_class<YnetMultiplayerPeer>();
}

void uninitialize_yarnnet_module(ModuleInitializationLevel p_level) {
 	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
 			return;
   }
	if (YNetPtr != nullptr) {
		Engine::get_singleton()->remove_singleton("YNet");
		memdelete(YNetPtr);
		YNetPtr = nullptr;
	}

   // Nothing to do here in this example.
}