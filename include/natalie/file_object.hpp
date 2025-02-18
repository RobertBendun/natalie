#pragma once

#include <assert.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "natalie/forward.hpp"
#include "natalie/integer_object.hpp"
#include "natalie/io_object.hpp"
#include "natalie/regexp_object.hpp"
#include "natalie/string_object.hpp"
#include "natalie/symbol_object.hpp"
#include "tm/defer.hpp"

namespace Natalie {

class FileObject : public IoObject {
public:
    FileObject()
        : IoObject { GlobalEnv::the()->Object()->const_fetch(SymbolObject::intern("File"))->as_class() } { }

    Value initialize(Env *, Value, Value, Block *);

    static Value open(Env *env, Value filename, Value flags_obj, Block *block) {
        Value args[] = { filename, flags_obj };
        size_t argc = 1;
        if (flags_obj) argc++;
        auto obj = _new(env, GlobalEnv::the()->Object()->const_fetch(SymbolObject::intern("File"))->as_class(), argc, args, nullptr);
        if (block) {
            Defer close_file([&]() {
                obj->as_file()->close(env);
            });
            Value block_args[] = { obj };
            Value result = NAT_RUN_BLOCK_AND_POSSIBLY_BREAK(env, block, 1, block_args, nullptr);
            return result;
        } else {
            return obj;
        }
    }

    static Value expand_path(Env *env, Value path, Value root);
    static Value unlink(Env *env, Value path);

    static void build_constants(Env *env, ClassObject *klass);

    static bool exist(Env *env, Value path) {
        struct stat sb;
        path->assert_type(env, Object::Type::String, "String");
        return stat(path->as_string()->c_str(), &sb) != -1;
    }

    static bool file(Env *env, Value path);
    static bool directory(Env *env, Value path);

    const String *path() { return m_path; }
    void set_path(String *path) { m_path = path; };

    virtual void gc_inspect(char *buf, size_t len) const override {
        snprintf(buf, len, "<FileObject %p>", this);
    }

    virtual void visit_children(Visitor &visitor) override final {
        Object::visit_children(visitor);
        visitor.visit(m_path);
    }

private:
    const String *m_path { nullptr };
};

}
