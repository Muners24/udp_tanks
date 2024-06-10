#include "..\t.h"

class SoundMsg
{
private:
    Vector2 origen;
    int id;

public:
    map<int, bool> send;

    SoundMsg(int id, list<int> clients_id, Vector2 origen);
    SoundMsg(){};

    bool operator==(const SoundMsg &t) const;
    Vector2 getOrg() { return origen; };
    bool shouldPlay(int id_client);
    ~SoundMsg() { send.clear(); };
    int getId() { return id; };
    bool readyToRemove();
};