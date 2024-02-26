#include "register_types.h"

// This is your singleton reference.
static YarnNet* YarnNetPtr;

void initialize_yarnnet_module(ModuleInitializationLevel p_level) {
#ifdef TOOLS_ENABLED
	if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR) {
	}
#endif

 	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
 			return;
   }
 	ClassDB::register_class<YarnNet>();
	// Initialize your singleton.
	YarnNetPtr = memnew(YarnNet);

	// Bind your singleton.
	Engine::get_singleton()->add_singleton(Engine::Singleton("YarnNet", YarnNet::get_singleton()));

	ClassDB::register_class<YnetMultiplayerPeer>();
}

void uninitialize_yarnnet_module(ModuleInitializationLevel p_level) {
 	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
 			return;
   }
	if (YarnNetPtr != nullptr) {
		Engine::get_singleton()->remove_singleton("YarnNet");
		memdelete(YarnNetPtr);
		YarnNetPtr = nullptr;
	}

   // Nothing to do here in this example.
}