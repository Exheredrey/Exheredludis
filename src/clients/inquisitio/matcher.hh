/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_SRC_CLIENTS_INQUISITIO_MATCHER_HH
#define PALUDIS_GUARD_SRC_CLIENTS_INQUISITIO_MATCHER_HH 1

#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/exception.hh>
#include <string>
#include <tr1/memory>

namespace inquisitio
{
    class Matcher :
        private paludis::InstantiationPolicy<Matcher, paludis::instantiation_method::NonCopyableTag>
    {
        protected:
            Matcher();

        public:
            typedef bool result;

            virtual ~Matcher();

            virtual bool operator() (const std::string &) const = 0;
    };

    class NoSuchMatcherError :
        public paludis::Exception
    {
        public:
            NoSuchMatcherError(const std::string &) throw ();
    };

    class MatcherFactory :
        public paludis::InstantiationPolicy<MatcherFactory, paludis::instantiation_method::SingletonTag>
    {
        friend class paludis::InstantiationPolicy<MatcherFactory, paludis::instantiation_method::SingletonTag>;

        private:
            MatcherFactory();

        public:
            const std::tr1::shared_ptr<Matcher> create(const std::string &, const std::string &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));
    };
}

#endif
