// Cherry/src/Cherry/Scene/ScriptableEntity.h (Add this if it doesn't exist)
#pragma once
#include "Entity.h"

namespace Cherry {

    class ScriptableEntity {
    public:
        virtual ~ScriptableEntity() {}

        template<typename T>
        T& GetComponent() {
            return m_Entity.GetComponent<T>();
        }

    protected:
        virtual void OnCreate() {}
        virtual void OnDestroy() {}
        virtual void OnUpdate(TimeStep ts) {}

    private:
        Entity m_Entity;
        friend class Scene;
    };

} // namespace Cherry