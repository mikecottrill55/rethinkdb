#ifndef RDB_PROTOCOL_MINIDRIVER_HPP_
#define RDB_PROTOCOL_MINIDRIVER_HPP_

#include <string>
#include <vector>
#include <utility>
#include <algorithm>

#include "rdb_protocol/env.hpp"
#include "rdb_protocol/ql2.pb.h"
#include "rdb_protocol/datum.hpp"
#include "rdb_protocol/counted_term.hpp"

namespace ql {

class reql_t;
extern const reql_t r;

/** reql_t
 *
 * A thin wrapper around scoped_ptr_t<Term> that allows building Terms
 * using the ReQL syntax.
 *
 * Move semantics are used for non-const reql_t to avoid copying the inner Term.
 *
 * - Non-const *this is treated like an rvalue reference. Store reql_t
 *   values in const variables or use .copy() if you use the same
 *   reql_t more than once. GCC < 4.8 has no support for explicit
 *   rvalue references for *this, so they are commented out.
 *
 **/

class reql_t {
private:
    class variable;
    friend class variable;
public:

    typedef const variable var_t;

    typedef std::pair<std::string, reql_t> key_value;

    static reql_t make_r(){
        return reql_t();
    }

    explicit reql_t(scoped_ptr_t<Term> &&term_);
    explicit reql_t(const double val);
    explicit reql_t(const std::string &val);
    explicit reql_t(const datum_t &d);
    explicit reql_t(const counted_t<const datum_t> &d);
    explicit reql_t(const Datum &d);
    explicit reql_t(const Term &t);
    explicit reql_t(std::vector<reql_t> &&val);

    reql_t(const reql_t &other);
    reql_t(reql_t &&other);
    reql_t &operator= (const reql_t &other);
    reql_t &operator= (reql_t &&other);

    template <class T>
    static reql_t expr(T &&d){
        return reql_t(std::forward<T>(d));
    }

    static reql_t boolean(bool b);

    static reql_t fun(reql_t &&body);
    static reql_t fun(const variable &a, reql_t &&body);
    static reql_t fun(const variable &a, const variable &b, reql_t &&body);

    static reql_t array() {
        return r.call(Term::MAKE_ARRAY);
    }

    template<class T, class ... Ts>
    static reql_t array(T &&x, Ts &&... xs) {
        return reql_t(make_vector(reql_t(std::forward<T>(x)),
                                  reql_t(std::forward<T>(xs)) ...));
    }

    static reql_t null();

    template <class ... T>
    reql_t call(Term_TermType type, T &&... args) const /* & */ {
        return copy().call(type, std::forward<T>(args) ...);
    }

    template <class ... T>
    reql_t call(Term_TermType type, T &&... args) /* && */ {
        reql_t ret(make_scoped<Term>());
        ret.term->set_type(type);
        if (term.has()) {
            ret.add_arg(std::move(*this));
        }
        ret.add_args(std::forward<T>(args) ...);
        return ret;
    }

    key_value optarg(const std::string &key, reql_t &&value);

    reql_t copy() const;

    Term* release();

    Term &get();

    protob_t<Term> release_counted();

    void swap(Term &t);

#define REQL_METHOD(name, termtype)                             \
    template<class ... T>                                       \
    reql_t name(T &&... a) /* && */                             \
    { return call(Term::termtype, std::forward<T>(a) ...); }    \
    template<class ... T>                                       \
    reql_t name(T &&... a) const /* & */                        \
    { return call(Term::termtype, std::forward<T>(a) ...); }

    REQL_METHOD(operator +, ADD)
    REQL_METHOD(operator ==, EQ)
    REQL_METHOD(operator (), FUNCALL)
    REQL_METHOD(operator >, GT)
    REQL_METHOD(operator <, LT)
    REQL_METHOD(operator >=, GE)
    REQL_METHOD(operator <=, LE)
    REQL_METHOD(operator &&, ALL)
    REQL_METHOD(count, COUNT)
    REQL_METHOD(map, MAP)
    REQL_METHOD(db, DB)
    REQL_METHOD(branch, BRANCH)
    REQL_METHOD(error, ERROR)
    REQL_METHOD(operator [], GET_FIELD)
    REQL_METHOD(nth, NTH)
    REQL_METHOD(pluck, PLUCK)

#undef REQL_METHOD

private:

    void set_datum(const datum_t &d);

    template <class ... T>
    void add_args(T &&... args){
        UNUSED int _[] = { (add_arg(std::forward<T>(args)), 1) ... };
    }

    template<class T>
    void add_arg(T &&a){
        reql_t it(std::forward<T>(a));
        term->mutable_args()->AddAllocated(it.term.release());
    }

    reql_t();

    scoped_ptr_t<Term> term;
};

class reql_t::variable : public reql_t {
public:
    int id;
    explicit variable(env_t *env);
    explicit variable(int id_);
    explicit variable(const variable &var);
};

template <>
inline void reql_t::add_arg(key_value &&kv){
    auto ap = make_scoped<Term_AssocPair>();
    ap->set_key(kv.first);
    ap->mutable_val()->Swap(kv.second.term.get());
    term->mutable_optargs()->AddAllocated(ap.release());
}

} // namespace ql

#endif // RDB_PROTOCOL_MINIDRIVER_HPP_
