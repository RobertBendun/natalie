#pragma once

#include <assert.h>
#include <float.h>
#include <math.h>

#include "natalie/bignum_object.hpp"
#include "natalie/class_object.hpp"
#include "natalie/forward.hpp"
#include "natalie/global_env.hpp"
#include "natalie/integer_object.hpp"
#include "natalie/macros.hpp"
#include "natalie/object.hpp"
#include "natalie/symbol_object.hpp"

namespace Natalie {

class FloatObject : public Object {
public:
    FloatObject(double number)
        : Object { Object::Type::Float, GlobalEnv::the()->Float() }
        , m_double { number } { }

    FloatObject(nat_int_t number)
        : Object { Object::Type::Float, GlobalEnv::the()->Float() }
        , m_double { static_cast<double>(number) } { }

    FloatObject(const FloatObject &other)
        : Object { Object::Type::Float, const_cast<FloatObject &>(other).klass() }
        , m_double { other.m_double } { }

    static FloatObject *nan(Env *env) {
        return new FloatObject { 0.0 / 0.0 };
    }

    static FloatObject *positive_infinity(Env *env) {
        return new FloatObject { std::numeric_limits<double>::infinity() };
    }

    static FloatObject *negative_infinity(Env *env) {
        return new FloatObject { -std::numeric_limits<double>::infinity() };
    }

    static FloatObject *max(Env *env) {
        return new FloatObject { DBL_MAX };
    }

    static FloatObject *neg_max(Env *env) {
        return new FloatObject { -DBL_MAX };
    }

    static FloatObject *min(Env *env) {
        return new FloatObject { DBL_MIN };
    }

    double to_double() const {
        return m_double;
    }

    Value to_int_no_truncation(Env *env) {
        if (is_nan() || is_infinity()) return this;
        if (m_double == ::floor(m_double)) {
            if (m_double >= (double)NAT_INT_MAX || m_double <= (double)NAT_INT_MAX) {
                auto *bignum = new BignumObject { to_double() };
                if (bignum->has_to_be_bignum())
                    return bignum;
            }

            return Value::integer(static_cast<nat_int_t>(m_double));
        }
        return this;
    }

    bool is_zero() const {
        return m_double == 0 && !is_nan();
    }

    bool is_finite() const {
        return !(is_negative_infinity() || is_positive_infinity() || is_nan());
    }

    bool is_nan() const {
        return isnan(m_double);
    }

    bool is_infinity() const {
        return isinf(m_double);
    }

    bool is_negative() const {
        return m_double < 0.0;
    };

    bool is_positive() const {
        return m_double > 0.0;
    };

    // NOTE: even though this is a predicate method with a ? suffix, it returns an 1, -1, or nil.
    Value is_infinite(Env *) const;

    bool is_positive_infinity() const {
        return is_infinity() && m_double > 0;
    }

    bool is_negative_infinity() const {
        return is_infinity() && m_double < 0;
    }

    bool is_positive_zero() const {
        return m_double == 0 && !signbit(m_double);
    }

    bool is_negative_zero() const {
        return m_double == 0 && signbit(m_double);
    }

    FloatObject *negate() const {
        FloatObject *copy = new FloatObject { *this };
        copy->m_double *= -1;
        return copy;
    }

    bool eq(Env *, Value);

    bool eql(Value) const;

    Value abs(Env *) const;
    Value add(Env *, Value);
    Value arg(Env *);
    Value ceil(Env *, Value) const;
    Value cmp(Env *, Value);
    Value coerce(Env *, Value);
    Value div(Env *, Value);
    Value divmod(Env *, Value);
    Value floor(Env *, Value) const;
    Value mod(Env *, Value);
    Value mul(Env *, Value);
    Value next_float(Env *) const;
    Value pow(Env *, Value);
    Value prev_float(Env *) const;
    Value round(Env *, Value);
    Value sub(Env *, Value);
    Value to_f() { return this; }
    Value to_i(Env *) const;
    Value to_s(Env *) const;
    Value truncate(Env *, Value) const;

    bool lt(Env *, Value);
    bool lte(Env *, Value);
    bool gt(Env *, Value);
    bool gte(Env *, Value);

    Value uminus() const {
        return negate();
    }

    Value uplus() const {
        return new FloatObject { *this };
    }

    static void build_constants(Env *env, ClassObject *klass) {
        klass->const_set(SymbolObject::intern("DIG"), new FloatObject { double { DBL_DIG } });
        klass->const_set(SymbolObject::intern("EPSILON"), new FloatObject { std::numeric_limits<double>::epsilon() });
        klass->const_set(SymbolObject::intern("INFINITY"), FloatObject::positive_infinity(env));
        klass->const_set(SymbolObject::intern("MANT_DIG"), new FloatObject { double { DBL_MANT_DIG } });
        klass->const_set(SymbolObject::intern("MAX"), FloatObject::max(env));
        klass->const_set(SymbolObject::intern("MAX_10_EXP"), new FloatObject { double { DBL_MAX_10_EXP } });
        klass->const_set(SymbolObject::intern("MAX_EXP"), new FloatObject { double { DBL_MAX_EXP } });
        klass->const_set(SymbolObject::intern("MIN"), FloatObject::min(env));
        klass->const_set(SymbolObject::intern("MIN_10_EXP"), new FloatObject { double { DBL_MIN_10_EXP } });
        klass->const_set(SymbolObject::intern("MIN_EXP"), new FloatObject { double { DBL_MIN_EXP } });
        klass->const_set(SymbolObject::intern("NAN"), FloatObject::nan(env));
        klass->const_set(SymbolObject::intern("NAN"), FloatObject::nan(env));
        klass->const_set(SymbolObject::intern("RADIX"), new FloatObject { double { std::numeric_limits<double>::radix } });
    }

    virtual void gc_inspect(char *buf, size_t len) const override {
        snprintf(buf, len, "<FloatObject %p float=%f>", this, m_double);
    }

private:
    double m_double { 0.0 };
};
}
