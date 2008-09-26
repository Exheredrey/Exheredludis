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

#ifndef PALUDIS_GUARD_PALUDIS_EAPI_HH
#define PALUDIS_GUARD_PALUDIS_EAPI_HH 1

#include <paludis/repositories/e/eapi-fwd.hh>
#include <paludis/repositories/e/dep_parser-fwd.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/sr.hh>
#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/options.hh>
#include <paludis/util/named_value.hh>
#include <paludis/name.hh>
#include <paludis/dep_spec-fwd.hh>
#include <paludis/merger-fwd.hh>
#include <tr1/memory>

namespace paludis
{
    namespace n
    {
        struct binary_from_env_variables;
        struct bracket_merged_variables;
        struct breaks_portage;
        struct bugs_to;
        struct build_depend;
        struct can_be_pbin;
        struct dependencies;
        struct dependency_labels;
        struct dependency_spec_tree_parse_options;
        struct description;
        struct description_use;
        struct directory_if_exists_variables;
        struct directory_variables;
        struct doman_lang_filenames;
        struct dosym_mkdir;
        struct eapi;
        struct ebuild_config;
        struct ebuild_environment_variables;
        struct ebuild_functions;
        struct ebuild_info;
        struct ebuild_install;
        struct ebuild_metadata;
        struct ebuild_metadata_variables;
        struct ebuild_module_suffixes;
        struct ebuild_must_not_set_variables;
        struct ebuild_nofetch;
        struct ebuild_options;
        struct ebuild_phases;
        struct ebuild_pretend;
        struct ebuild_uninstall;
        struct ebuild_variable;
        struct ebuild_new_upgrade_phase_order;
        struct eclass_must_not_set_variables;
        struct env_a;
        struct env_aa;
        struct env_accept_keywords;
        struct env_arch;
        struct env_d;
        struct env_distdir;
        struct env_filesdir;
        struct env_kv;
        struct env_p;
        struct env_pf;
        struct env_portdir;
        struct env_t;
        struct env_use;
        struct env_use_expand;
        struct env_use_expand_hidden;
        struct exported_name;
        struct f_function_prefix;
        struct failure_is_fatal;
        struct flat_list_index;
        struct homepage;
        struct ignore_pivot_env_functions;
        struct ignore_pivot_env_variables;
        struct inherited;
        struct iuse;
        struct iuse_flag_parse_options;
        struct keywords;
        struct license;
        struct load_modules;
        struct long_description;
        struct merger_options;
        struct metadata_key;
        struct minimum_flat_list_size;
        struct must_not_change_variables;
        struct name;
        struct no_slot_or_repo;
        struct non_empty_variables;
        struct package_dep_spec_parse_options;
        struct pdepend;
        struct pipe_commands;
        struct properties;
        struct provide;
        struct rdepend_defaults_to_depend;
        struct remote_ids;
        struct require_use_expand_in_iuse;
        struct restrict_fetch;
        struct restrict_mirror;
        struct restrict_primaryuri;
        struct restrictions;
        struct rewrite_virtuals;
        struct run_depend;
        struct save_base_variables;
        struct save_unmodifiable_variables;
        struct save_variables;
        struct short_description;
        struct slot;
        struct source_merged_variables;
        struct src_uri;
        struct support_eclasses;
        struct support_exlibs;
        struct supported;
        struct tools_options;
        struct unpack_fix_permissions;
        struct unpack_unrecognised_is_fatal;
        struct upstream_changelog;
        struct upstream_documentation;
        struct upstream_release_notes;
        struct uri_labels;
        struct use;
        struct use_expand_separator;
        struct userpriv_cannot_use_root;
        struct utility_path_suffixes;
        struct vdb_from_env_unless_empty_variables;
        struct vdb_from_env_variables;
        struct want_portage_emulation_vars;
    }

    namespace erepository
    {
        class PALUDIS_VISIBLE EAPILabels :
            private PrivateImplementationPattern<EAPILabels>
        {
            public:
                EAPILabels(const std::string &);
                EAPILabels(const EAPILabels &);
                ~EAPILabels();

                const std::string class_for_label(const std::string &) const PALUDIS_ATTRIBUTE((warn_unused_result));
        };

        /**
         * Thrown if an EAPI configuration is broken.
         *
         * \see EAPI
         * \ingroup grpeapi
         * \ingroup grpexceptions
         * \nosubgrouping
         */
        class PALUDIS_VISIBLE EAPIConfigurationError :
            public ConfigurationError
        {
            public:
                EAPIConfigurationError(const std::string &) throw ();
        };

        /**
         * Holds information on recognised EAPIs.
         *
         * \see EAPI
         * \ingroup grpeapi
         * \nosubgrouping
         */
        class PALUDIS_VISIBLE EAPIData :
            private PrivateImplementationPattern<EAPIData>,
            public InstantiationPolicy<EAPIData, instantiation_method::SingletonTag>
        {
            friend class InstantiationPolicy<EAPIData, instantiation_method::SingletonTag>;

            private:
                EAPIData();
                ~EAPIData();

            public:
                /**
                 * Make an EAPI.
                 */
                std::tr1::shared_ptr<const EAPI> eapi_from_string(const std::string &) const;

                /**
                 * Make the unknown EAPI.
                 */
                std::tr1::shared_ptr<const EAPI> unknown_eapi() const;
        };

        struct EAPI
        {
            NamedValue<n::exported_name, std::string> exported_name;
            NamedValue<n::name, std::string> name;
            NamedValue<n::supported, std::tr1::shared_ptr<const SupportedEAPI> > supported;
        };

        struct SupportedEAPI
        {
            NamedValue<n::breaks_portage, bool> breaks_portage;
            NamedValue<n::can_be_pbin, bool> can_be_pbin;
            NamedValue<n::dependency_labels, std::tr1::shared_ptr<const EAPILabels> > dependency_labels;
            NamedValue<n::dependency_spec_tree_parse_options, erepository::DependencySpecTreeParseOptions> dependency_spec_tree_parse_options;
            NamedValue<n::ebuild_environment_variables, std::tr1::shared_ptr<const EAPIEbuildEnvironmentVariables> > ebuild_environment_variables;
            NamedValue<n::ebuild_metadata_variables, std::tr1::shared_ptr<const EAPIEbuildMetadataVariables> > ebuild_metadata_variables;
            NamedValue<n::ebuild_options, std::tr1::shared_ptr<const EAPIEbuildOptions> > ebuild_options;
            NamedValue<n::ebuild_phases, std::tr1::shared_ptr<const EAPIEbuildPhases> > ebuild_phases;
            NamedValue<n::iuse_flag_parse_options, IUseFlagParseOptions> iuse_flag_parse_options;
            NamedValue<n::merger_options, MergerOptions> merger_options;
            NamedValue<n::package_dep_spec_parse_options, ELikePackageDepSpecOptions> package_dep_spec_parse_options;
            NamedValue<n::pipe_commands, std::tr1::shared_ptr<const EAPIPipeCommands> > pipe_commands;
            NamedValue<n::tools_options, std::tr1::shared_ptr<const EAPIToolsOptions> > tools_options;
            NamedValue<n::uri_labels, std::tr1::shared_ptr<const EAPILabels> > uri_labels;
            NamedValue<n::userpriv_cannot_use_root, bool> userpriv_cannot_use_root;
        };

        struct EAPIEbuildEnvironmentVariables
        {
            NamedValue<n::description_use, std::string> description_use;
            NamedValue<n::env_a, std::string> env_a;
            NamedValue<n::env_aa, std::string> env_aa;
            NamedValue<n::env_accept_keywords, std::string> env_accept_keywords;
            NamedValue<n::env_arch, std::string> env_arch;
            NamedValue<n::env_d, std::string> env_d;
            NamedValue<n::env_distdir, std::string> env_distdir;
            NamedValue<n::env_filesdir, std::string> env_filesdir;
            NamedValue<n::env_kv, std::string> env_kv;
            NamedValue<n::env_p, std::string> env_p;
            NamedValue<n::env_pf, std::string> env_pf;
            NamedValue<n::env_portdir, std::string> env_portdir;
            NamedValue<n::env_t, std::string> env_t;
            NamedValue<n::env_use, std::string> env_use;
            NamedValue<n::env_use_expand, std::string> env_use_expand;
            NamedValue<n::env_use_expand_hidden, std::string> env_use_expand_hidden;
        };

        struct EAPIMetadataVariable
        {
            NamedValue<n::description, std::string> description;
            NamedValue<n::flat_list_index, int> flat_list_index;
            NamedValue<n::name, std::string> name;
        };

        struct EAPIEbuildMetadataVariables
        {
            NamedValue<n::bugs_to, EAPIMetadataVariable> bugs_to;
            NamedValue<n::build_depend, EAPIMetadataVariable> build_depend;
            NamedValue<n::dependencies, EAPIMetadataVariable> dependencies;
            NamedValue<n::eapi, EAPIMetadataVariable> eapi;
            NamedValue<n::homepage, EAPIMetadataVariable> homepage;
            NamedValue<n::inherited, EAPIMetadataVariable> inherited;
            NamedValue<n::iuse, EAPIMetadataVariable> iuse;
            NamedValue<n::keywords, EAPIMetadataVariable> keywords;
            NamedValue<n::license, EAPIMetadataVariable> license;
            NamedValue<n::long_description, EAPIMetadataVariable> long_description;
            NamedValue<n::minimum_flat_list_size, int> minimum_flat_list_size;
            NamedValue<n::pdepend, EAPIMetadataVariable> pdepend;
            NamedValue<n::properties, EAPIMetadataVariable> properties;
            NamedValue<n::provide, EAPIMetadataVariable> provide;
            NamedValue<n::remote_ids, EAPIMetadataVariable> remote_ids;
            NamedValue<n::restrictions, EAPIMetadataVariable> restrictions;
            NamedValue<n::run_depend, EAPIMetadataVariable> run_depend;
            NamedValue<n::short_description, EAPIMetadataVariable> short_description;
            NamedValue<n::slot, EAPIMetadataVariable> slot;
            NamedValue<n::src_uri, EAPIMetadataVariable> src_uri;
            NamedValue<n::upstream_changelog, EAPIMetadataVariable> upstream_changelog;
            NamedValue<n::upstream_documentation, EAPIMetadataVariable> upstream_documentation;
            NamedValue<n::upstream_release_notes, EAPIMetadataVariable> upstream_release_notes;
            NamedValue<n::use, EAPIMetadataVariable> use;
        };

        struct EAPIEbuildOptions
        {
            NamedValue<n::binary_from_env_variables, std::string> binary_from_env_variables;
            NamedValue<n::bracket_merged_variables, std::string> bracket_merged_variables;
            NamedValue<n::directory_if_exists_variables, std::string> directory_if_exists_variables;
            NamedValue<n::directory_variables, std::string> directory_variables;
            NamedValue<n::ebuild_functions, std::string> ebuild_functions;
            NamedValue<n::ebuild_module_suffixes, std::string> ebuild_module_suffixes;
            NamedValue<n::ebuild_must_not_set_variables, std::string> ebuild_must_not_set_variables;
            NamedValue<n::eclass_must_not_set_variables, std::string> eclass_must_not_set_variables;
            NamedValue<n::f_function_prefix, std::string> f_function_prefix;
            NamedValue<n::ignore_pivot_env_functions, std::string> ignore_pivot_env_functions;
            NamedValue<n::ignore_pivot_env_variables, std::string> ignore_pivot_env_variables;
            NamedValue<n::load_modules, std::string> load_modules;
            NamedValue<n::must_not_change_variables, std::string> must_not_change_variables;
            NamedValue<n::non_empty_variables, std::string> non_empty_variables;
            NamedValue<n::rdepend_defaults_to_depend, bool> rdepend_defaults_to_depend;
            NamedValue<n::require_use_expand_in_iuse, bool> require_use_expand_in_iuse;
            NamedValue<n::restrict_fetch, std::tr1::shared_ptr<Set<std::string> > > restrict_fetch;
            NamedValue<n::restrict_mirror, std::tr1::shared_ptr<Set<std::string> > > restrict_mirror;
            NamedValue<n::restrict_primaryuri, std::tr1::shared_ptr<Set<std::string> > > restrict_primaryuri;
            NamedValue<n::save_base_variables, std::string> save_base_variables;
            NamedValue<n::save_unmodifiable_variables, std::string> save_unmodifiable_variables;
            NamedValue<n::save_variables, std::string> save_variables;
            NamedValue<n::source_merged_variables, std::string> source_merged_variables;
            NamedValue<n::support_eclasses, bool> support_eclasses;
            NamedValue<n::support_exlibs, bool> support_exlibs;
            NamedValue<n::use_expand_separator, char> use_expand_separator;
            NamedValue<n::utility_path_suffixes, std::string> utility_path_suffixes;
            NamedValue<n::vdb_from_env_unless_empty_variables, std::string> vdb_from_env_unless_empty_variables;
            NamedValue<n::vdb_from_env_variables, std::string> vdb_from_env_variables;
            NamedValue<n::want_portage_emulation_vars, bool> want_portage_emulation_vars;
        };

        struct EAPIEbuildPhases
        {
            NamedValue<n::ebuild_config, std::string> ebuild_config;
            NamedValue<n::ebuild_info, std::string> ebuild_info;
            NamedValue<n::ebuild_install, std::string> ebuild_install;
            NamedValue<n::ebuild_metadata, std::string> ebuild_metadata;
            NamedValue<n::ebuild_new_upgrade_phase_order, bool> ebuild_new_upgrade_phase_order;
            NamedValue<n::ebuild_nofetch, std::string> ebuild_nofetch;
            NamedValue<n::ebuild_pretend, std::string> ebuild_pretend;
            NamedValue<n::ebuild_uninstall, std::string> ebuild_uninstall;
            NamedValue<n::ebuild_variable, std::string> ebuild_variable;
        };

        struct EAPIToolsOptions
        {
            NamedValue<n::doman_lang_filenames, bool> doman_lang_filenames;
            NamedValue<n::dosym_mkdir, bool> dosym_mkdir;
            NamedValue<n::failure_is_fatal, bool> failure_is_fatal;
            NamedValue<n::unpack_fix_permissions, bool> unpack_fix_permissions;
            NamedValue<n::unpack_unrecognised_is_fatal, bool> unpack_unrecognised_is_fatal;
        };

        struct EAPIPipeCommands
        {
            NamedValue<n::no_slot_or_repo, bool> no_slot_or_repo;
            NamedValue<n::rewrite_virtuals, bool> rewrite_virtuals;
        };
    }
#endif
}
