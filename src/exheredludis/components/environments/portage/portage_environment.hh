/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009, 2010, 2011, 2014 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_ENVIRONMENTS_E_E_ENVIRONMENT_HH
#define PALUDIS_GUARD_PALUDIS_ENVIRONMENTS_E_E_ENVIRONMENT_HH 1

#include <paludis/environment_implementation.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/pimp.hh>

namespace paludis
{
    namespace portage_environment
    {
        /**
         * Thrown if a configuration error occurs in a PortageEnvironment.
         *
         * \ingroup grpportageenvironment
         * \ingroup grpexceptions
         * \nosubgrouping
         */
        class PALUDIS_VISIBLE PortageEnvironmentConfigurationError :
            public ConfigurationError
        {
            public:
                ///\name Basic operations
                ///\{

                PortageEnvironmentConfigurationError(const std::string &) noexcept;

                ///\}
        };
    }

    /**
     * Environment using Portage-like configuration files.
     *
     * \ingroup grpportageenvironment
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE PortageEnvironment :
        public EnvironmentImplementation
    {
        private:
            Pimp<PortageEnvironment> _imp;

            void _load_profile(const FSPath &);
            void _add_portdir_repository(const FSPath &);
            void _add_portdir_overlay_repository(const FSPath &);
            void _add_ebuild_repository(const FSPath &, const std::string &,
                    const std::string &, int importance);
            void _add_vdb_repository();

            template<typename I_>
            void _load_lined_file(const FSPath &, I_);

            template<typename I_>
            void _load_atom_file(const FSPath &, I_, const std::string &, const bool);

            bool _add_string_to_world(const std::string &) const;
            bool _remove_string_from_world(const std::string &) const;

            std::string reduced_groupname() const;

        protected:
            virtual void need_keys_added() const;
            virtual void populate_sets() const;

        public:
            ///\name Basic operations
            ///\{

            PortageEnvironment(const std::string &);
            virtual ~PortageEnvironment();

            ///\}

            virtual Tribool interest_in_suggestion(
                    const std::shared_ptr<const PackageID> & from_id,
                    const PackageDepSpec & spec) const;

            virtual const Tribool want_choice_enabled(
                    const std::shared_ptr<const PackageID> &,
                    const std::shared_ptr<const Choice> &,
                    const UnprefixedChoiceName &
                    ) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual const std::string value_for_choice_parameter(
                    const std::shared_ptr<const PackageID> &,
                    const std::shared_ptr<const Choice> &,
                    const UnprefixedChoiceName &
                    ) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::shared_ptr<const Set<UnprefixedChoiceName> > known_choice_value_names(
                    const std::shared_ptr<const PackageID> &,
                    const std::shared_ptr<const Choice> &
                    ) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::shared_ptr<const FSPathSequence> bashrc_files() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::shared_ptr<const FSPathSequence> hook_dirs() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::shared_ptr<const MirrorsSequence> mirrors(const std::string &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual HookResult perform_hook(
                    const Hook &,
                    const std::shared_ptr<OutputManager> &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool accept_license(const std::string &, const std::shared_ptr<const PackageID> &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool accept_keywords(const std::shared_ptr<const KeywordNameSet> &, const std::shared_ptr<const PackageID> &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual const std::shared_ptr<const Mask> mask_for_user(const std::shared_ptr<const PackageID> &, const bool) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool unmasked_by_user(const std::shared_ptr<const PackageID> &, const std::string &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::string reduced_username() const;

            virtual uid_t reduced_uid() const;

            virtual gid_t reduced_gid() const;

            virtual bool add_to_world(const QualifiedPackageName &) const;

            virtual bool add_to_world(const SetName &) const;

            virtual bool remove_from_world(const QualifiedPackageName &) const;

            virtual bool remove_from_world(const SetName &) const;

            virtual const std::shared_ptr<const MetadataValueKey<std::string> > format_key() const;
            virtual const std::shared_ptr<const MetadataValueKey<FSPath> > config_location_key() const;
            virtual const std::shared_ptr<const MetadataValueKey<FSPath> > preferred_root_key() const;
            virtual const std::shared_ptr<const MetadataValueKey<FSPath> > system_root_key() const;

            virtual const std::shared_ptr<OutputManager> create_output_manager(
                    const CreateOutputManagerInfo &) const;

            virtual const std::shared_ptr<Repository> repository_from_new_config_file(
                    const FSPath &) PALUDIS_ATTRIBUTE((noreturn));

            virtual void update_config_files_for_package_move(
                    const PackageDepSpec &, const QualifiedPackageName &) const;
    };
}

#endif
