#pragma once

#include <assert.h>

#include "natalie/class_object.hpp"
#include "natalie/forward.hpp"
#include "natalie/global_env.hpp"
#include "natalie/macros.hpp"
#include "natalie/object.hpp"
#include "natalie/symbol_object.hpp"

namespace Natalie {

class TrueObject : public Object {
public:
    static TrueObject *the() {
        if (s_instance) {
            assert(s_instance->flags() == 0);
            return s_instance;
        }
        s_instance = new TrueObject();
        return s_instance;
    }

    Value to_s(Env *);

    virtual void gc_inspect(char *buf, size_t len) const override {
        snprintf(buf, len, "<TrueObject %p>", this);
    }

    virtual bool is_collectible() override {
        return false;
    }

private:
    inline static TrueObject *s_instance = nullptr;

    TrueObject()
        : Object { Object::Type::True, GlobalEnv::the()->Object()->const_fetch(SymbolObject::intern("TrueClass"))->as_class() } { }
};

}
