#include "rdb_protocol/minidriver.hpp"

namespace ql {

const reql_t r = reql_t::make_r();

reql_t::reql_t(scoped_ptr_t<Term> &&term_) : term(std::move(term_)) { }

reql_t::reql_t(const double val) { set_datum(datum_t(val)); }

reql_t::reql_t(const std::string &val) { set_datum(datum_t(val.c_str())); }

reql_t::reql_t(const datum_t &d) { set_datum(d); }

reql_t::reql_t(const counted_t<const datum_t> &d) { set_datum(*d.get()); }

reql_t::reql_t(const Datum &d) : term(make_scoped<Term>()) {
    term->set_type(Term::DATUM);
    *term->mutable_datum() = d;
}

reql_t::reql_t(const Term &t) : term(make_scoped<Term>(t)) { }

reql_t::reql_t(std::vector<reql_t> &&val) : term(make_scoped<Term>()) {
    term->set_type(Term::MAKE_ARRAY);
    for (auto i = val.begin(); i != val.end(); i++) {
        add_arg(std::move(*i));
    }
}

reql_t::reql_t(const reql_t &other) : term(make_scoped<Term>(*other.term.get())) { }

reql_t::reql_t(reql_t &&other) : term(std::move(other.term)) { }

reql_t reql_t::boolean(bool b) {
    return reql_t(datum_t(datum_t::R_BOOL, b));
}

reql_t &reql_t::operator= (const reql_t &other) {
    auto t = make_scoped<Term>(*other.term);
    term.swap(t);
    return *this;
}

reql_t &reql_t::operator= (reql_t &&other) {
    term.swap(other.term);
    return *this;
}

reql_t reql_t::fun(reql_t&& body) {
    return r.call(Term::FUNC, r.array(), std::move(body));
}

reql_t reql_t::fun(const reql_t::variable& a, reql_t&& body) {
    std::vector<reql_t> v;
    v.emplace_back(static_cast<double>(a.id));
    return r.call(Term::FUNC, std::move(v), std::move(body));
}

reql_t reql_t::fun(const reql_t::variable& a, const reql_t::variable& b, reql_t&& body) {
    std::vector<reql_t> v;
    v.emplace_back(static_cast<double>(a.id));
    v.emplace_back(static_cast<double>(b.id));
    return r.call(Term::FUNC, std::move(v), std::move(body));
}

reql_t reql_t::null() {
    auto t = make_scoped<Term>();
    t->set_type(Term::DATUM);
    t->mutable_datum()->set_type(Datum::R_NULL);
    return reql_t(std::move(t));
}

reql_t::key_value reql_t::optarg(const std::string &key, reql_t &&value) {
    return key_value(key, std::move(value));
}

reql_t reql_t::copy() const {
    reql_t ret;
    if (term.has()) {
        ret.term = make_scoped<Term>(*term);
    }
    return ret;
}

Term* reql_t::release() {
    return term.release();
}

Term &reql_t::get() {
    return *term;
}

protob_t<Term> reql_t::release_counted() {
    protob_t<Term> ret = make_counted_term();
    auto t = scoped_ptr_t<Term>(term.release());
    ret->Swap(t.get());
    return ret;
}

void reql_t::swap(Term &t) {
    t.Swap(term.get());
}

void reql_t::set_datum(const datum_t &d) {
    term = make_scoped<Term>();
    term->set_type(Term::DATUM);
    d.write_to_protobuf(term->mutable_datum());
}

reql_t::reql_t() : term(NULL) { }

reql_t::variable::variable(env_t *env) : reql_t(), id(env->gensym()) {
    term = r.call(Term::VAR, static_cast<double>(id)).term;
}

reql_t::variable::variable(int id_) : reql_t(), id(id_) {
    term = r.call(Term::VAR, static_cast<double>(id)).term;
}

reql_t::variable::variable(const variable &var) : reql_t(var.copy()), id(var.id) { }


}
