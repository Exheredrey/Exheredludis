/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Piotr Jaroszyński
 *
 * This file is part of the Paludis package manager. Paludis is free software;
 * you can redistribute it and/or modify it under the terms of the GNU General
 * Public License version 2, as published by the Free Software Foundation.
 *
 * Paludis is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef PALUDIS_GUARD_PYTHON_PALUDIS_PYTHON_HH
#define PALUDIS_GUARD_PYTHON_PALUDIS_PYTHON_HH 1

#include <python/mutex.hh>
#include <paludis/util/tr1_memory.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/attributes.hh>

#include <boost/python.hpp>

namespace paludis
{
    // Make Boost.Python work with tr1::shared_ptr<>
    template <typename T_>
    inline T_ * get_pointer(tr1::shared_ptr<T_> const & p)
    {
        return p.get();
    }

    // Make Boost.Python work with tr1::shared_ptr<const>
    template <typename T_>
    inline T_ * get_pointer(tr1::shared_ptr<const T_> const & p)
    {
        return const_cast<T_*>(p.get());
    }
}

namespace boost
{
    namespace python
    {
        // Make Boost.Python work with tr1::shared_ptr<>
        template <typename T_>
        struct pointee<paludis::tr1::shared_ptr<T_> >
        {
            typedef T_ type;
        };

        // Make Boost.Python work with tr1::shared_ptr<const>
        template <typename T_>
        struct pointee<paludis::tr1::shared_ptr<const T_> >
        {
            typedef T_ type;
        };
    }
}

namespace paludis
{
    namespace python
    {
        // Which shared_ptrs to expose
        enum RegisterSharedPointers
        {
            rsp_both,
            rsp_non_const,
            rsp_const
        };

        // register shared_ptrs
        template <typename T_>
        void
        register_shared_ptrs_to_python(RegisterSharedPointers rsp=rsp_both)
        {
            if (rsp == rsp_both || rsp == rsp_non_const)
                boost::python::register_ptr_to_python<tr1::shared_ptr<T_> >();
            if (rsp == rsp_both || rsp == rsp_const)
                boost::python::register_ptr_to_python<tr1::shared_ptr<const T_> >();
            boost::python::implicitly_convertible<tr1::shared_ptr<T_>, tr1::shared_ptr<const T_> >();
        }

        // expose stringifyable enums
        template <typename E_>
        void
        enum_auto(const std::string & name, E_ e_last, std::string doc)
        {
            doc += "\n\nPossible values:";
            boost::python::enum_<E_> enum_(name.c_str());
            for (E_ e(static_cast<E_>(0)); e != e_last ; e = static_cast<E_>(static_cast<int>(e) + 1))
            {
                const std::string e_name_low = stringify(e);
                std::string e_name_up;
                std::transform(e_name_low.begin(), e_name_low.end(), std::back_inserter(e_name_up), &::toupper);
                enum_.value(e_name_up.c_str(), e);
                doc += "\n\t" + e_name_up;
            }
            PyTypeObject * pto = reinterpret_cast<PyTypeObject *>(enum_.ptr());
            PyDict_SetItemString(pto->tp_dict, "__doc__", PyString_FromString(doc.c_str()));
        }

        // Compare
        template <typename T_>
        int py_cmp(const T_ & a, const T_ & b)
        {
            if (a == b)
                return 0;
            else if (a < b)
                return -1;
            else
                return 1;
        }

        // Equal
        template <typename T_>
        bool py_eq(const T_ & a, const T_ & b)
        {
            return (a == b);
        }

        // Not equal
        template <typename T_>
        bool py_ne(const T_ & a, const T_ & b)
        {
            return ! (a == b);
        }

        // Convert to string
        template <typename T_>
        struct to_string
        {
            static PyObject *
            convert(const T_ & x)
            {
                return PyString_FromString(stringify<T_>(x).c_str());
            }
        };

        // Convert pair to tuple
        template <typename first_, typename second_>
        struct pair_to_tuple
        {
            static PyObject *
            convert(const std::pair<first_, second_> & x)
            {
                return boost::python::incref(boost::python::make_tuple(x.first, x.second).ptr());
            }
        };

        // helper for kc getters
        template <typename C_, typename T_, typename K_>
        T_ kc_getter(const C_ & c)
        {
            return c[K_()];
        }

        // helper for kc setters
        template <typename C_, typename T_, typename K_>
        void kc_setter(C_ & c, const T_ & t)
        {
            c[K_()] = t;
        }
    } // namespace paludis::python
} // namespace paludis

void expose_about() PALUDIS_VISIBLE;
void expose_action() PALUDIS_VISIBLE;
void expose_contents() PALUDIS_VISIBLE;
void expose_dep_label() PALUDIS_VISIBLE;
void expose_dep_list() PALUDIS_VISIBLE;
void expose_dep_spec() PALUDIS_VISIBLE;
void expose_dep_tag() PALUDIS_VISIBLE;
void expose_environment() PALUDIS_VISIBLE;
void expose_exception() PALUDIS_VISIBLE;
void expose_formatter() PALUDIS_VISIBLE;
void expose_fs_entry() PALUDIS_VISIBLE;
void expose_log() PALUDIS_VISIBLE;
void expose_mask() PALUDIS_VISIBLE;
void expose_metadata_key() PALUDIS_VISIBLE;
void expose_name() PALUDIS_VISIBLE;
void expose_package_database() PALUDIS_VISIBLE;
void expose_package_id() PALUDIS_VISIBLE;
void expose_qa() PALUDIS_VISIBLE;
void expose_query() PALUDIS_VISIBLE;
void expose_repository() PALUDIS_VISIBLE;
void expose_use_requirements() PALUDIS_VISIBLE;
void expose_version_operator() PALUDIS_VISIBLE;
void expose_version_requirements() PALUDIS_VISIBLE;
void expose_version_spec() PALUDIS_VISIBLE;

#endif
