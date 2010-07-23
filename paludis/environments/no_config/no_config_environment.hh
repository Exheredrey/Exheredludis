/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009, 2010 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_ENVIRONMENT_NO_CONFIG_NO_CONFIG_ENVIRONMENT_HH
#define PALUDIS_GUARD_PALUDIS_ENVIRONMENT_NO_CONFIG_NO_CONFIG_ENVIRONMENT_HH 1

#include <paludis/environment_implementation.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/pimp.hh>
#include <paludis/util/map-fwd.hh>

namespace paludis
{
    namespace n
    {
        typedef Name<struct accept_unstable_name> accept_unstable;
        typedef Name<struct disable_metadata_cache_name> disable_metadata_cache;
        typedef Name<struct extra_accept_keywords_name> extra_accept_keywords;
        typedef Name<struct extra_params_name> extra_params;
        typedef Name<struct extra_repository_dirs_name> extra_repository_dirs;
        typedef Name<struct master_repository_name_name> master_repository_name;
        typedef Name<struct profiles_if_not_auto_name> profiles_if_not_auto;
        typedef Name<struct repository_dir_name> repository_dir;
        typedef Name<struct repository_type_name> repository_type;
        typedef Name<struct write_cache_name> write_cache;
    }

    namespace no_config_environment
    {
#include <paludis/environments/no_config/no_config_environment-se.hh>

        /**
         * Parameters for a NoConfigEnvironment.
         *
         * \see NoConfigEnvironment
         * \ingroup grpnoconfigenvironment
         * \nosubgrouping
         */
        struct Params
        {
            NamedValue<n::accept_unstable, bool> accept_unstable;
            NamedValue<n::disable_metadata_cache, bool> disable_metadata_cache;
            NamedValue<n::extra_accept_keywords, std::string> extra_accept_keywords;
            NamedValue<n::extra_params, std::shared_ptr<Map<std::string, std::string> > > extra_params;
            NamedValue<n::extra_repository_dirs, std::shared_ptr<const FSEntrySequence> > extra_repository_dirs;
            NamedValue<n::master_repository_name, std::string> master_repository_name;

            /**
             * The profiles to use.
             *
             * Leave empty for automatic selection (which may not always be possible).
             *
             * \since 0.44
             */
            NamedValue<n::profiles_if_not_auto, std::string> profiles_if_not_auto;

            NamedValue<n::repository_dir, FSEntry> repository_dir;
            NamedValue<n::repository_type, no_config_environment::RepositoryType> repository_type;
            NamedValue<n::write_cache, FSEntry> write_cache;
        };
    }

    /**
     * An environment that uses a single repository, with no user configuration.
     *
     * \ingroup grpnoconfigenvironment
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE NoConfigEnvironment :
        public EnvironmentImplementation,
        private Pimp<NoConfigEnvironment>
    {
        private:
            Pimp<NoConfigEnvironment>::ImpPtr & _imp;

            virtual void need_keys_added() const;

        protected:
            virtual void populate_sets() const;

        public:
            ///\name Basic operations
            ///\{

            NoConfigEnvironment(const no_config_environment::Params & params);

            virtual ~NoConfigEnvironment();

            ///\}

            ///\name NoConfigEnvironment-specific configuration options
            ///\{

            /**
             * What is our top level directory for our main repository?
             */
            FSEntry main_repository_dir() const;

            /**
             * Should we accept unstable keywords?
             */
            void set_accept_unstable(const bool value);

            ///\}

            ///\name NoConfigEnvironment-specific repository information
            ///\{

            /**
             * Fetch our 'main' repository.
             */
            std::shared_ptr<Repository> main_repository();

            /**
             * Fetch our 'main' repository.
             */
            std::shared_ptr<const Repository> main_repository() const;

            /**
             * Fetch our 'master' repository (may be zero).
             */
            std::shared_ptr<Repository> master_repository();

            /**
             * Fetch our 'master' repository (may be zero).
             */
            std::shared_ptr<const Repository> master_repository() const;

            ///\}

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

            virtual std::shared_ptr<PackageDatabase> package_database()
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::shared_ptr<const PackageDatabase> package_database() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::string paludis_command() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual void set_paludis_command(const std::string &);

            virtual bool accept_license(const std::string &, const PackageID &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool accept_keywords(const std::shared_ptr<const KeywordNameSet> &, const PackageID &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual const std::shared_ptr<const Mask> mask_for_breakage(const PackageID &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual const std::shared_ptr<const Mask> mask_for_user(const PackageID &, const bool will_be_used_for_overridden) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool unmasked_by_user(const PackageID &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::shared_ptr<const FSEntrySequence> hook_dirs() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual const FSEntry root() const;

            virtual uid_t reduced_uid() const;

            virtual gid_t reduced_gid() const;

            virtual std::shared_ptr<const MirrorsSequence> mirrors(const std::string &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual HookResult perform_hook(const Hook &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual bool add_to_world(const QualifiedPackageName &) const;

            virtual bool add_to_world(const SetName &) const;

            virtual bool remove_from_world(const QualifiedPackageName &) const;

            virtual bool remove_from_world(const SetName &) const;

            virtual const std::shared_ptr<const MetadataValueKey<std::string> > format_key() const;
            virtual const std::shared_ptr<const MetadataValueKey<FSEntry> > config_location_key() const;

            virtual const std::shared_ptr<OutputManager> create_output_manager(
                    const CreateOutputManagerInfo &) const;

            virtual const std::shared_ptr<Repository> repository_from_new_config_file(
                    const FSEntry &) PALUDIS_ATTRIBUTE((noreturn));

            virtual void update_config_files_for_package_move(
                    const PackageDepSpec &, const QualifiedPackageName &) const;
    };
}

#endif
