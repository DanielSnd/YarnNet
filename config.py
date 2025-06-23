def can_build(env, platform):
    return True

def configure(env):
    env.Append(CPPPATH=['#thirdparty/enet/'])

def get_doc_path():
	return "doc_classes"

def get_doc_classes():
	return [
		"YNet",
		"YNetPropertySyncer",
		"YNetMultiplayerPeer",
		"YSnapSyncer3D",
		"YNetSocketIO",
		"YNetEnet",
	]
