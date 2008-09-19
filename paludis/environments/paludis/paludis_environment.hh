/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_DEFAULT_ENVIRONMENT_HH
#define PALUDIS_GUARD_PALUDIS_DEFAULT_ENVIRONMENT_HH 1

#include <paludis/package_database.hh>
#include <paludis/environment_implementation.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/name-fwd.hh>

/** \file
 * Declarations for the PaludisEnvironment class.
 *
 * \ingroup grppaludisenvironment
 */

namespace paludis
{
    namespace paludis_environment
    {
        class PaludisConfig;
    }

    /**
     * The PaludisEnvironment is an Environment that corresponds to the normal
     * operating evironment.
     *
     * \ingroup grppaludisenvironment
     */
    class PALUDIS_VISIBLE PaludisEnvironment :
        public EnvironmentImplementation,
        public InstantiationPolicy<PaludisEnvironment, instantiation_method::SingletonTag>,
        private PrivateImplementationPattern<PaludisEnvironment>
    {

        private:
            PrivateImplementationPattern<PaludisEnvironment>::ImpPtr & _imp;

        protected:
            virtual void need_keys_added() const;

            virtual std::tr1::shared_ptr<SetSpecTree::ConstItem> local_set(const SetName & id) const;

            virtual std::tr1::shared_ptr<SetSpecTree::ConstItem> world_set() const;

        public:
            ///\name Basic operations
            ///\{

            PaludisEnvironment(const std::string &);

            ~PaludisEnvironment();

            ///\}

            ///\name PaludisEnvironment-specific information
            ///\{

            /**
             * The config directory.
             */
            std::string config_dir() const;

            ///\}

            virtual bool query_use(const UseFlagName &, const PackageID &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::tr1::shared_ptr<const UseFlagNameSet> known_use_expand_names(
                    const UseFlagName &, const PackageID &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::tr1::shared_ptr<const FSEntrySequence> bashrc_files() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::tr1::shared_ptr<const FSEntrySequence> syncers_dirs() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::tr1::shared_ptr<const FSEntrySequence> fetchers_dirs() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::tr1::shared_ptr<const FSEntrySequence> hook_dirs() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual const FSEntry root() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual uid_t reduced_uid() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual gid_t reduced_gid() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::tr1::shared_ptr<const MirrorsSequence> mirrors(const std::string &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::tr1::shared_ptr<const SetNameSet> set_names() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual HookResult perform_hook(const Hook &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::string paludis_command() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual void set_paludis_command(const std::string &);

            virtual std::tr1::shared_ptr<PackageDatabase> package_database()
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::tr1::shared_ptr<const PackageDatabase> package_database() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::string distribution() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool accept_keywords(const std::tr1::shared_ptr<const KeywordNameSet> &, const PackageID &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool accept_license(const std::string &, const PackageID &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual const std::tr1::shared_ptr<const Mask> mask_for_breakage(const PackageID &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual const std::tr1::shared_ptr<const Mask> mask_for_user(const PackageID &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool unmasked_by_user(const PackageID &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual void add_to_world(const QualifiedPackageName &) const;

            virtual void add_to_world(const SetName &) const;

            virtual void remove_from_world(const QualifiedPackageName &) const;

            virtual void remove_from_world(const SetName &) const;

            virtual const std::tr1::shared_ptr<const MetadataValueKey<std::string> > format_key() const;
            virtual const std::tr1::shared_ptr<const MetadataValueKey<FSEntry> > config_location_key() const;
    };
}
#endif
