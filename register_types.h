#include "modules/register_module_types.h"
#include "core/object/class_db.h"
#include "yarnnet.h"
#include "core/config/engine.h"
#include "ynet_multiplayer_peer.h"
#include "ynetsyncer.h"
#include "ynet_transport.h"
#include "ynet_socketio.h"
#include "ynet_enet.h"
#include "ysnapsyncer.h"

void initialize_yarnnet_module(ModuleInitializationLevel p_level);
void uninitialize_yarnnet_module(ModuleInitializationLevel p_level);