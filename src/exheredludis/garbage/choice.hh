/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_CHOICE_HH
#define PALUDIS_GUARD_PALUDIS_CHOICE_HH 1

#include <paludis/choice-fwd.hh>
#include <paludis/permitted_choice_value_parameter_values-fwd.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/pimp.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/named_value.hh>
#include <paludis/util/wrapped_value.hh>
#include <paludis/util/set.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <memory>
#include <string>

/** \file
 * Declarations for choice-related classes.
 *
 * \ingroup g_choices
 * \since 0.32
 *
 * \section Examples
 *
 * - None at this time.
 */

namespace paludis
{
    namespace n
    {
        typedef Name<struct name_consider_added_or_changed> consider_added_or_changed;
        typedef Name<struct name_contains_every_value> contains_every_value;
        typedef Name<struct name_hidden> hidden;
        typedef Name<struct name_hide_description> hide_description;
        typedef Name<struct name_human_name> human_name;
        typedef Name<struct name_prefix> prefix;
        typedef Name<struct name_raw_name> raw_name;
        typedef Name<struct name_show_with_no_prefix> show_with_no_prefix;
    }

    /**
     * Thrown if a ChoicePrefixName is given an invalid value.
     *
     * \since 0.32
     * \ingroup g_choices
     * \ingroup g_exceptions
     */
    class PALUDIS_VISIBLE ChoicePrefixNameError :
        public NameError
    {
        public:
            ChoicePrefixNameError(const std::string &) noexcept;
    };

    /**
     * Thrown if a ChoiceNameWithPrefix is given an invalid value.
     *
     * \since 0.32
     * \ingroup g_choices
     * \ingroup g_exceptions
     */
    class PALUDIS_VISIBLE ChoiceNameWithPrefixError :
        public NameError
    {
        public:
            ChoiceNameWithPrefixError(const std::string &) noexcept;
    };

    /**
     * Thrown if a UnprefixedChoiceName is given an invalid value.
     *
     * \since 0.32
     * \ingroup g_choices
     * \ingroup g_exceptions
     */
    class PALUDIS_VISIBLE UnprefixedChoiceNameError :
        public NameError
    {
        public:
            UnprefixedChoiceNameError(const std::string &) noexcept;
    };

    template <>
    struct WrappedValueTraits<ChoicePrefixNameTag>
    {
        typedef std::string UnderlyingType;
        typedef void ValidationParamsType;
        typedef ChoicePrefixNameError ExceptionType;

        static bool validate(const std::string &) PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    extern template class PALUDIS_VISIBLE WrappedValue<ChoicePrefixNameTag>;

    template <>
    struct WrappedValueTraits<UnprefixedChoiceNameTag>
    {
        typedef std::string UnderlyingType;
        typedef void ValidationParamsType;
        typedef UnprefixedChoiceNameError ExceptionType;

        static bool validate(const std::string &) PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    extern template class PALUDIS_VISIBLE WrappedValue<UnprefixedChoiceNameTag>;

    template <>
    struct WrappedValueTraits<ChoiceNameWithPrefixTag>
    {
        typedef std::string UnderlyingType;
        typedef void ValidationParamsType;
        typedef ChoiceNameWithPrefixError ExceptionType;

        static bool validate(const std::string &) PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    extern template class PALUDIS_VISIBLE WrappedValue<ChoiceNameWithPrefixTag>;

    /**
     * Choices holds a collection of configurable values for a PackageID.
     *
     * A PackageID may have a choices_key holding a Choices object. This Choices object
     * will hold a number of Choice objects. Examples of things held by Choice objects for
     * ebuilds include USE, USE_EXPAND values (linguas, video_cards and so on), ARCH and
     * build_options.
     *
     * \ingroup g_choices
     * \since 0.32
     */
    class PALUDIS_VISIBLE Choices
    {
        private:
            Pimp<Choices> _imp;

        public:
            ///\name Basic operations
            ///\{

            Choices();
            ~Choices();

            ///\}

            /**
             * Add a new Choice to our collection.
             */
            void add(const std::shared_ptr<const Choice> &);

            ///\name Iterate over Choice children
            ///\{

            struct ConstIteratorTag;
            typedef WrappedForwardIterator<ConstIteratorTag, const std::shared_ptr<const Choice> > ConstIterator;
            ConstIterator begin() const PALUDIS_ATTRIBUTE((warn_unused_result));
            ConstIterator end() const PALUDIS_ATTRIBUTE((warn_unused_result));

            ///\since 0.44
            ConstIterator find(const ChoicePrefixName &) const PALUDIS_ATTRIBUTE((warn_unused_result));

            ///\}

            /**
             * Find a ChoiceValue that has a particular prefix and name.
             *
             * Returns a zero pointer for no match.
             *
             * This is a convenient way of getting a particular use flag's details. Calling this
             * method with, say, "nls" or "ruby" will get the value for that flag without having
             * to hunt around in all the subkeys manually. Prefixes work too, e.g. "linguas_en" for
             * 0-based EAPIs or "linguas:en" for exheres EAPIs.
             */
            const std::shared_ptr<const ChoiceValue> find_by_name_with_prefix(
                    const ChoiceNameWithPrefix &) const PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Do we have a Choice subkey with contains_every_value true and a prefix matching
             * this name?
             *
             * 0-based EAPIs don't require things like userland_GNU in IUSE. So if you're looking
             * for a flag and don't find it, check this method before issuing a QA notice.
             */
            bool has_matching_contains_every_value_prefix(const ChoiceNameWithPrefix &) const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    /**
     * Named parameters for Choice::Choice.
     *
     * \since 0.38
     * \ingroup g_choices
     */
    struct ChoiceParams
    {
        NamedValue<n::consider_added_or_changed, bool> consider_added_or_changed;
        NamedValue<n::contains_every_value, bool> contains_every_value;
        NamedValue<n::hidden, bool> hidden;
        NamedValue<n::hide_description, bool> hide_description;
        NamedValue<n::human_name, std::string> human_name;
        NamedValue<n::prefix, ChoicePrefixName> prefix;
        NamedValue<n::raw_name, std::string> raw_name;
        NamedValue<n::show_with_no_prefix, bool> show_with_no_prefix;
    };

    /**
     * An individual choice in a Choices collection.
     *
     * Examples of a choice include USE, individual USE_EXPAND values (linguas, video_cards etc)
     * and build_options.
     *
     * \ingroup g_choices
     * \since 0.32
     */
    class PALUDIS_VISIBLE Choice
    {
        private:
            Pimp<Choice> _imp;

        public:
            ///\name Basic operations
            ///\{

            ///\since 0.38
            Choice(const ChoiceParams &);
            ~Choice();

            ///\}

            /**
             * Add a new ChoiceValue.
             */
            void add(const std::shared_ptr<const ChoiceValue> &);

            ///\name Properties
            ///\{

            /**
             * Our raw name, for example 'USE' or 'LINGUAS'.
             */
            const std::string raw_name() const PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * A human-readable name (often the same as raw_name).
             */
            const std::string human_name() const PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * The prefix for our ChoiceValue children.
             *
             * An empty string for USE and ARCH, 'linguas' for LINGUAS etc.
             */
            const ChoicePrefixName prefix() const PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * If true, pretend that we contain every possible value and that any value not listed
             * as a child exists and is not enabled.
             *
             * For pesky 0-based EAPIs that don't require things like userland_GNU in IUSE, and that
             * don't have a comprehensive list of possible values.
             */
            bool contains_every_value() const PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * If true, this option should not usually be shown visually to a user.
             */
            bool hidden() const PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * If true, this option should not have a description be shown
             * visually to the user.
             */
            bool hide_description() const PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * If true, hint that we're better not displaying our prefix to the user.
             *
             * This is used by --pretend --install and --query to avoid showing a Use:
             * prefix before a list of use flag names.
             */
            bool show_with_no_prefix() const PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * If false, do not consider flags in this section for 'added' or 'changed'
             * detection.
             *
             * Used by build_options.
             */
            bool consider_added_or_changed() const PALUDIS_ATTRIBUTE((warn_unused_result));

            ///\}

            ///\name Iterate over ChoiceValue children
            ///\{

            struct ConstIteratorTag;
            typedef WrappedForwardIterator<ConstIteratorTag, const std::shared_ptr<const ChoiceValue> > ConstIterator;
            ConstIterator begin() const PALUDIS_ATTRIBUTE((warn_unused_result));
            ConstIterator end() const PALUDIS_ATTRIBUTE((warn_unused_result));

            ///\}
    };

    /**
     * An individial Value in a ChoiceValue.
     *
     * For example, the 'nls' flag in 'USE'.
     *
     * Some choice values have an associated parameter. For example,
     * build_options:jobs=4.
     *
     * \ingroup g_choices
     * \since 0.32
     */
    class PALUDIS_VISIBLE ChoiceValue
    {
        public:
            ///\name Basic operations
            ///\{

            virtual ~ChoiceValue() = 0;

            ///\}

            ///\name Properties
            ///\{

            /**
             * Our name, without an prefix (for example, 'nls' or 'en').
             */
            virtual const UnprefixedChoiceName unprefixed_name() const PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Our name, with prefix if there is one (for example, 'nls' or 'linguas_en').
             */
            virtual const ChoiceNameWithPrefix name_with_prefix() const PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Is this flag enabled?
             */
            virtual bool enabled() const PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Would this flag be enabled by default (i.e. before
             * considering any overrides from the Environment)?
             */
            virtual bool enabled_by_default() const PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Does this flag have to be explicitly accepted when doing matching?
             */
            virtual bool presumed() const PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Is this flag locked (forced or masked)?
             */
            virtual bool locked() const PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * The flag's description, or an empty string.
             */
            virtual const std::string description() const PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * The origin of this choice.
             *
             * \since 0.66
             *
             * Use this to avoid showing things like LINGUAS values that aren't listed
             * in IUSE but that end up as a ChoiceValue anyway.
             */
            virtual ChoiceOrigin origin() const PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * This flag's parameter, or an empty string if it doesn't have one.
             *
             * \since 0.40
             */
            virtual const std::string parameter() const PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Permitted values for this flag's parameter, or null if there is no parameter.
             *
             * \since 0.59
             */
            virtual const std::shared_ptr<const PermittedChoiceValueParameterValues> permitted_parameter_values() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            ///\}
    };

    extern template class Pimp<Choices>;
    extern template class Pimp<Choice>;

    extern template class PALUDIS_VISIBLE Set<UnprefixedChoiceName>;
    extern template class PALUDIS_VISIBLE WrappedForwardIterator<Set<UnprefixedChoiceName>::ConstIteratorTag, const UnprefixedChoiceName>;
    extern template class PALUDIS_VISIBLE WrappedOutputIterator<Set<UnprefixedChoiceName>::InserterTag, UnprefixedChoiceName>;

    extern template class PALUDIS_VISIBLE WrappedForwardIterator<Choices::ConstIteratorTag, const std::shared_ptr<const Choice> >;
    extern template class PALUDIS_VISIBLE WrappedForwardIterator<Choice::ConstIteratorTag, const std::shared_ptr<const ChoiceValue> >;
}

#endif
