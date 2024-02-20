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

}

void uninitialize_yarnnet_module(ModuleInitializationLevel p_level) {
 	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
 			return;
   }
   // Nothing to do here in this example.
}