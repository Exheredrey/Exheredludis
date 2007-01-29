/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_EBUILD_HH
#define PALUDIS_GUARD_PALUDIS_EBUILD_HH 1

#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/collection.hh>
#include <paludis/package_database.hh>
#include <string>

/** \file
 * Declarations for the EbuildCommand classes.
 *
 * \ingroup grpebuildinterface
 */

namespace paludis
{
    /**
     * Keys for EbuildCommandParams.
     *
     * \see EbuildCommandParams
     *
     * \ingroup grpebuildinterface
     */
    enum EbuildCommandParamsKeys
    {
        ecpk_environment,
        ecpk_db_entry,
        ecpk_ebuild_dir,
        ecpk_files_dir,
        ecpk_eclassdirs,
        ecpk_portdir,
        ecpk_distdir,
        ecpk_buildroot,
        last_ecpk
    };

    class Environment;
    class MakeEnvCommand;

#include <paludis/ebuild-sr.hh>

    class EbuildVersionMetadata :
        public VersionMetadata,
        public VersionMetadataEbuildInterface,
        public VersionMetadataDepsInterface,
        public VersionMetadataLicenseInterface
    {
        public:
            typedef CountedPtr<EbuildVersionMetadata, count_policy::InternalCountTag> Pointer;
            typedef CountedPtr<const EbuildVersionMetadata, count_policy::InternalCountTag> ConstPointer;

            EbuildVersionMetadata();
            virtual ~EbuildVersionMetadata();
    };

    /**
     * An EbuildCommand is the base class from which specific ebuild
     * command interfaces are descended.
     *
     * \ingroup grpebuildinterface
     */
    class EbuildCommand :
        private InstantiationPolicy<EbuildCommand, instantiation_method::NonCopyableTag>
    {
        protected:
            /**
             * Our parameters.
             */
            const EbuildCommandParams params;

            /**
             * Constructor.
             */
            EbuildCommand(const EbuildCommandParams &);

            /**
             * Override in descendents: which commands (for example, 'prerm
             * unmerge postrm') do we give to ebuild.bash?
             */
            virtual std::string commands() const = 0;

            /**
             * Actions to be taken after a successful command.
             *
             * The return value of this function is used for the return value
             * of operator().
             */
            virtual bool success();

            /**
             * Should the sandbox, if available, be used?
             */
            virtual bool use_sandbox() const;

            /**
             * Actions to be taken after a failed command.
             *
             * The return value of this function is used for the return value
             * of operator(). In some descendents, this function throws and
             * does not return.
             */
            virtual bool failure() = 0;

            /**
             * Run the specified command. Can be overridden if, for example,
             * the command output needs to be captured.
             *
             * \return Whether the command succeeded.
             */
            virtual bool do_run_command(const std::string &);

            /**
             * Add Portage emulation vars.
             */
            virtual MakeEnvCommand add_portage_vars(const MakeEnvCommand &) const;

            /**
             * Extend the command to be run.
             */
            virtual MakeEnvCommand extend_command(const MakeEnvCommand &) = 0;

        public:
            /**
             * Destructor.
             */
            virtual ~EbuildCommand();

            /**
             * Run the command.
             */
            virtual bool operator() ();
    };

    /**
     * An EbuildMetadataCommand is used to generate metadata for a particular
     * ebuild in a PortageRepository.
     *
     * \ingroup grpebuildinterface
     */
    class EbuildMetadataCommand :
        public EbuildCommand
    {
        private:
            EbuildVersionMetadata::Pointer _metadata;

        protected:
            virtual std::string commands() const;

            virtual bool failure();

            virtual MakeEnvCommand extend_command(const MakeEnvCommand &);

            virtual bool do_run_command(const std::string &);

        public:
            /**
             * Constructor.
             */
            EbuildMetadataCommand(const EbuildCommandParams &);

            /**
             * Return a pointer to our generated metadata. If operator() has not
             * yet been called, will be a zero pointer.
             */
            EbuildVersionMetadata::Pointer metadata() const
            {
                return _metadata;
            }
    };

    /**
     * An EbuildVariableCommand is used to fetch the value of an environment
     * variable for a particular ebuild in a PortageRepository.
     *
     * \ingroup grpebuildinterface
     */
    class EbuildVariableCommand :
        public EbuildCommand
    {
        private:
            std::string _result;
            const std::string _var;

        protected:
            virtual std::string commands() const;

            virtual MakeEnvCommand extend_command(const MakeEnvCommand &);

            virtual bool do_run_command(const std::string &);

            virtual bool failure();

        public:
            /**
             * Constructor.
             */
            EbuildVariableCommand(const EbuildCommandParams &, const std::string &);

            /**
             * Fetch our result.
             */
            std::string result() const
            {
                return _result;
            }
    };

    /**
     * An EbuildFetchCommand is used to download and verify the digests for a
     * particular ebuild in a PortageRepository. On failure it throws.
     *
     * \ingroup grpebuildinterface
     */
    class EbuildFetchCommand :
        public EbuildCommand
    {
        protected:
            /// Parameters for fetch.
            const EbuildFetchCommandParams fetch_params;

            virtual std::string commands() const;

            virtual bool failure() PALUDIS_ATTRIBUTE((noreturn));

            virtual MakeEnvCommand extend_command(const MakeEnvCommand &);

        public:
            /**
             * Constructor.
             */
            EbuildFetchCommand(const EbuildCommandParams &, const EbuildFetchCommandParams &);
    };

    /**
     * An EbuildInstallCommand is used to install an ebuild from a
     * PortageRepository. On failure it throws.
     *
     * \ingroup grpebuildinterface
     */
    class EbuildInstallCommand :
        public EbuildCommand
    {
        protected:
            /// Parameters for install.
            const EbuildInstallCommandParams install_params;

            virtual std::string commands() const;

            virtual bool failure() PALUDIS_ATTRIBUTE((noreturn));

            virtual MakeEnvCommand extend_command(const MakeEnvCommand &);

        public:
            /**
             * Constructor.
             */
            EbuildInstallCommand(const EbuildCommandParams &, const EbuildInstallCommandParams &);
    };

    /**
     * An EbuildUninstallCommand is used to uninstall a package in a VDBRepository.
     *
     * \ingroup grpebuildinterface
     */
    class EbuildUninstallCommand :
        public EbuildCommand
    {
        protected:
            /// Parameters for uninstall.
            const EbuildUninstallCommandParams uninstall_params;

            virtual std::string commands() const;

            virtual bool failure() PALUDIS_ATTRIBUTE((noreturn));

            virtual MakeEnvCommand extend_command(const MakeEnvCommand &);

        public:
            /**
             * Constructor.
             */
            EbuildUninstallCommand(const EbuildCommandParams &, const EbuildUninstallCommandParams &);
    };
}

#endif
