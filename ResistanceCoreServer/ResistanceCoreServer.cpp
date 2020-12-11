#include "ServerRootHeader.h"
#include "MainCore.h"

void* Extern_MainCore;

int main()
{
    MainCore* ServerMainCore;
    ServerMainCore = new MainCore();

    cout << "ResistanceCoreServer2.0...Start" << endl;

    // Initialize
    ServerMainCore->Initialize();
    Extern_MainCore = ServerMainCore;

    // MainLoop
    cout << endl << "---Server Run Start---" << endl;
    while(ServerMainCore->Tick()) {}
    cout << "---Server Run End----" << endl << endl;

    // Destroy
    ServerMainCore->Destroy();
    delete(ServerMainCore);

    cout << "ResistanceCoreServer2.0...End" << endl;

    return 0;
}