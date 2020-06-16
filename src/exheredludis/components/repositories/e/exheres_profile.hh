/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_E_REPOSITORY_EXHERES_PROFILE_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_E_REPOSITORY_EXHERES_PROFILE_HH 1

#include <paludis/repositories/e/profile.hh>
#include <paludis/repositories/e/e_repository.hh>
#include <string>

namespace paludis
{
    namespace erepository
    {
        class PALUDIS_VISIBLE ExheresProfile :
            public Profile
        {
            private:
                Pimp<ExheresProfile> _imp;

                void _load_dir(const FSPath &);

            public:
                ExheresProfile(
                        const Environment * const env,
                        const RepositoryName & name,
                        const EAPIForFileFunction & eapi_for_file,
                        const IsArchFlagFunction & is_arch_flag,
                        const FSPathSequence & dirs,
                        const std::string & arch_var_if_special,
                        const bool profiles_explicitly_set,
                        const bool has_master_repositories,
                        const bool ignore_deprecated_profiles);

                virtual ~ExheresProfile();

                virtual std::shared_ptr<const FSPathSequence> profiles_with_parents() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual bool use_masked(
                        const std::shared_ptr<const EbuildID> &,
                        const std::shared_ptr<const Choice> &,
                        const UnprefixedChoiceName & value_unprefixed,
                        const ChoiceNameWithPrefix & value_prefixed
                        ) const PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual bool use_forced(
                        const std::shared_ptr<const EbuildID> &,
                        const std::shared_ptr<const Choice> &,
                        const UnprefixedChoiceName & value_unprefixed,
                        const ChoiceNameWithPrefix & value_prefixed
                        ) const PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual Tribool use_state_ignoring_masks(
                        const std::shared_ptr<const PackageID> &,
                        const std::shared_ptr<const Choice> &,
                        const UnprefixedChoiceName & value_unprefixed,
                        const ChoiceNameWithPrefix & value_prefixed
                        ) const PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual const std::shared_ptr<const Set<UnprefixedChoiceName> > known_choice_value_names(
                        const std::shared_ptr<const erepository::ERepositoryID> &,
                        const std::shared_ptr<const Choice> &
                        ) const PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual const std::shared_ptr<const Set<std::string> > use_expand() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual const std::shared_ptr<const Set<std::string> > use_expand_hidden() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual const std::shared_ptr<const Set<std::string>> use_expand_no_describe() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual const std::shared_ptr<const Set<std::string> > use_expand_unprefixed() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual const std::shared_ptr<const Set<std::string> > use_expand_implicit() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual const std::shared_ptr<const Set<std::string> > iuse_implicit() const
                    PALUDIS_ATTRIBUTE((warn_unused_result));
                virtual const std::shared_ptr<const Set<std::string> > use_expand_values(const std::string &) const
                    PALUDIS_ATTRIBUTE((warn_unused_result));

                virtual const std::string environment_variable(const std::string &) const;

                virtual const std::shared_ptr<const MasksInfo> profile_masks(const std::shared_ptr<const PackageID> &) const;

                virtual const std::shared_ptr<const SetSpecTree> system_packages() const;
        };
    }
}

#endif
