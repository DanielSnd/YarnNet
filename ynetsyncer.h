//
// Created by Daniel on 2024-04-02.
//

#ifndef YNETSYNCER_H
#define YNETSYNCER_H
#include "scene/main/node.h"

///
/// YNetSyncer has a list of other nodes it observes. This is set on edit time
/// On ready the YNetSyncer takes inventory of all the properties of the nodes in the list
/// If a property starts with _sync_ it gets added to a list of observable properties it'll sync.
/// Set an interval on it and it'll on that interval check the properties for changes.
/// Those properties get stored in a RefCounted type that has a signal you can query and subscribe to to receive
/// change notifications for the property.
///
/// This could also handle spawning. It could use unique ids, and authority. You'd set that info before adding it to the
/// tree, and when it gets added to the tree it'd then request the other connections to spawn it.
///
/// When a new connection joins all ynet syncers syncronize their info into a big packet that gets sent to the new
/// connection. The new connection needs to aknowledge that packet.
///
/// I think that's mostly it. Could possibly also route rpcs through here maybe. Add some GDVirtual callbacks.
///
class YNetSyncer : public Node {
    GDCLASS(YNetSyncer, Node);


private:

public:
    YNetSyncer() = default;
};

#endif //YNETSYNCER_H
