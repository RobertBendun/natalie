#pragma once

#include <assert.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "natalie/forward.hpp"
#include "natalie/macros.hpp"

namespace Natalie {

class Cell {
public:
    Cell() { }
    virtual ~Cell() { }

    void *operator new(size_t size);
    void operator delete(void *ptr);

    class Visitor {
    public:
        virtual void visit(Cell *) = 0;
        virtual void visit(ValuePtr) = 0;
    };

    void virtual visit_children(Visitor &) {
        NAT_UNREACHABLE();
    }

    // only for debugging the GC
    virtual char *gc_repr() {
        char *buf = new char[100];
        snprintf(buf, 100, "<Cell %p size=%zu>", this, sizeof(*this));
        return buf;
    }

    bool marked() {
        return m_marked;
    }

    void mark() {
        m_marked = true;
    }

    void unmark() {
        m_marked = false;
    }

private:
    bool m_marked { false };
};

}
