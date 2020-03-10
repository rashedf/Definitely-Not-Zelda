#pragma once

#include "Components.h"

inline size_t GetComponentTypeID()
{
    static size_t lastID = 0;
    return lastID++;
}

template <typename T>
inline size_t GetComponentTypeID()
{
    static size_t typeID = GetComponentTypeID();
    return typeID;
}

class EntityManager;

class Entity
{
    friend class EntityManager;

    bool                m_active    = true;
    std::string         m_tag       = "default";
    size_t              m_id        = 0;

    std::array<std::shared_ptr<Component>, MaxComponents>   m_componentArray;

    Entity(const size_t & id, const std::string & tag);

public:

    void                    destroy();
    size_t                  id()                const;
    bool                    isActive()          const;
    const std::string &     tag()               const;

    template <typename T>
    bool hasComponent() const
    {
        return m_componentArray[GetComponentTypeID<T>()].get() != nullptr;
    }

    template <typename T, typename... TArgs>
    std::shared_ptr<T> addComponent(TArgs&&... mArgs)
    {
        std::shared_ptr<T> component = std::make_shared<T>(std::forward<TArgs>(mArgs)...);
        m_componentArray[GetComponentTypeID<T>()] = component;
        return component;
    }

    template<typename T>
    std::shared_ptr<T> getComponent()
    {
        return std::dynamic_pointer_cast<T>(m_componentArray[GetComponentTypeID<T>()]);
    }

    template<typename T>
    void removeComponent()
    {
        m_componentArray[GetComponentTypeID<T>()] = std::shared_ptr<T>();
    }
};

