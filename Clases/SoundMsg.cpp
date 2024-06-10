#include "SoundMsg.h"

SoundMsg::SoundMsg(int id, list<int> clients_id, Vector2 origen)
{
    for (int i : clients_id)
    {
        send[i] = false;
    }
    this->id = id;
    this->origen = origen;
}

bool SoundMsg::readyToRemove()
{
    if (send.empty())
        return true;

    for (auto flag = send.begin(); flag != send.end(); flag++)
    {
        if (!(*flag).second)
        {
            return false;
        }
    }
    return true;
}

bool SoundMsg::shouldPlay(int id_client)
{
    auto it = send.find(id_client);
    if (it == send.end())
        return false;
    
    if(it->second)
        return false;
        
    it->second = true;
    return true;
}

bool SoundMsg::operator==(const SoundMsg &s) const
{
    if (this == &s)
    {
        return true;
    }
    return false;
}
