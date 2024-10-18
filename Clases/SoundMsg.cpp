#include "SoundMsg.h"

_SoundMsg SoundMsg::toStruct()
{
    _SoundMsg s;
    s.id = this->id;
    s.origen = this->origen;
    return s;
}

SoundMsg::SoundMsg(int id, list<int> clients_id, Vector2 origen)
{
    for (int i : clients_id)
    {
        send[i] = true;
    }
    this->id = id;
    this->origen = origen;
    this->timer = 0;
}

bool SoundMsg::readyToRemove()
{
    
    if(timer > TTL_SOUNDS)
        return true;
    timer++;
    return false;
}

bool SoundMsg::shouldPlay(int id_client)
{
    auto it = send.find(id_client);
    if (it == send.end())
        return false;
    
    if(it->second)
    {
        it->second = false;
        return true;
    }
    return false;
}

bool SoundMsg::operator==(const SoundMsg &s) const
{
    if (this == &s)
    {
        return true;
    }
    return false;
}
