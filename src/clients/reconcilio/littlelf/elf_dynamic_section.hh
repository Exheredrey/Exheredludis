#ifndef ELFDYNAMICSECTION_HH_
#define ELFDYNAMICSECTION_HH_

#include "elf_sections.hh"

#include <paludis/util/clone.hh>
#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/tr1_memory.hh>
#include <paludis/util/visitor.hh>
#include <paludis/util/wrapped_forward_iterator-fwd.hh>

#include <string>
#include <iosfwd>

template <typename ElfType_> class DynamicEntry;
template <typename ElfType_> class DynamicEntryUnknown;
template <typename ElfType_> class DynamicEntryValue;
template <typename ElfType_> class DynamicEntryPointer;
template <typename ElfType_> class DynamicEntryString;
template <typename ElfType_> class DynamicEntryFlag;

template <typename ElfType_>
struct DynamicEntryVisitorTypes :
    paludis::VisitorTypes<
        DynamicEntryVisitorTypes<ElfType_>,
        DynamicEntry<ElfType_>,
        DynamicEntryUnknown<ElfType_>,
        DynamicEntryValue<ElfType_>,
        DynamicEntryPointer<ElfType_>,
        DynamicEntryString<ElfType_>,
        DynamicEntryFlag<ElfType_>
        >
{
};

template <typename ElfType_>
class DynamicEntriesVisitor :
    public paludis::Visitor<DynamicEntryVisitorTypes<ElfType_> >
{
    public:
        virtual void visit(DynamicEntryUnknown<ElfType_> &) {}
        virtual void visit(DynamicEntryString<ElfType_> &)  {}
        virtual void visit(DynamicEntryPointer<ElfType_> &) {}
        virtual void visit(DynamicEntryValue<ElfType_> &)   {}
        virtual void visit(DynamicEntryFlag<ElfType_> &)    {}
};

template <typename ElfType_>
class DynamicEntry :
    public virtual paludis::AcceptInterface<DynamicEntryVisitorTypes<ElfType_> >,
    public virtual paludis::Cloneable<DynamicEntry<ElfType_> >
{
    private:
        std::string _tag_name;

    public:
        DynamicEntry(const std::string &);
        ~DynamicEntry();
        virtual void initialize(const typename ElfType_::DynamicEntry & entry) = 0;

        std::string tag_name() const
        {
            return _tag_name;
        }
};

template <typename ElfType_>
class DynamicEntryUnknown :
    public virtual DynamicEntry<ElfType_>,
    public paludis::AcceptInterfaceVisitsThis<DynamicEntryVisitorTypes<ElfType_> , DynamicEntryUnknown<ElfType_> >,
    public paludis::CloneUsingThis<DynamicEntry<ElfType_>, DynamicEntryUnknown<ElfType_> >
{
    public:
        DynamicEntryUnknown();
        virtual ~DynamicEntryUnknown();
        virtual void initialize(const typename ElfType_::DynamicEntry &);
};

template <typename ElfType_>
class DynamicEntryFlag :
    public virtual DynamicEntry<ElfType_>,
    public paludis::AcceptInterfaceVisitsThis<DynamicEntryVisitorTypes<ElfType_> , DynamicEntryFlag<ElfType_> >,
    public paludis::CloneUsingThis<DynamicEntry<ElfType_>, DynamicEntryFlag<ElfType_> >
{
    public:
        DynamicEntryFlag(const std::string &);
        ~DynamicEntryFlag();
        virtual void initialize(const typename ElfType_::DynamicEntry &);
};

template <typename ElfType_>
class DynamicEntryValue :
    public virtual DynamicEntry<ElfType_>,
    public paludis::AcceptInterfaceVisitsThis<DynamicEntryVisitorTypes<ElfType_> , DynamicEntryValue<ElfType_> >,
    public paludis::CloneUsingThis<DynamicEntry<ElfType_>, DynamicEntryValue<ElfType_> >
{
    private:
        typename ElfType_::DynamicValue _value;

    public:
        DynamicEntryValue(const std::string &);
        virtual ~DynamicEntryValue();
        virtual void initialize(const typename ElfType_::DynamicEntry & entry);

        typename ElfType_::DynamicValue operator() () const
        {
            return _value;
        }
};

template <typename ElfType_>
class DynamicEntryPointer :
    public virtual DynamicEntry<ElfType_>,
    public paludis::AcceptInterfaceVisitsThis<DynamicEntryVisitorTypes<ElfType_> , DynamicEntryPointer<ElfType_> >,
    public paludis::CloneUsingThis<DynamicEntry<ElfType_>, DynamicEntryPointer<ElfType_> >
{
    private:
        typename ElfType_::DynamicPointer _pointer;

    public:
        DynamicEntryPointer(const std::string &);
        virtual ~DynamicEntryPointer();
        virtual void initialize(const typename ElfType_::DynamicEntry &);

        typename ElfType_::DynamicPointer operator() () const
        {
            return _pointer;
        }
};

namespace littlelf_internals
{
    template <typename ElfType_> class DynEntriesStringResolvingVisitor;
}

template <typename ElfType_>
class DynamicEntryString :
    public virtual DynamicEntry<ElfType_>,
    public paludis::AcceptInterfaceVisitsThis<DynamicEntryVisitorTypes<ElfType_> , DynamicEntryString<ElfType_> >,
    public paludis::CloneUsingThis<DynamicEntry<ElfType_>, DynamicEntryString<ElfType_> >
{
    friend class littlelf_internals::DynEntriesStringResolvingVisitor<ElfType_>;

    private:
        typename ElfType_::DynamicValue _value;
        std::string _str;

    public:
        DynamicEntryString(const std::string &);
        virtual ~DynamicEntryString();
        virtual void initialize(const typename ElfType_::DynamicEntry &);

        std::string operator() () const
        {
            return _str;
        }

    private:
        void resolve_string(std::string str)
        {
            _str = str;
        }

        typename ElfType_::DynamicValue get_string_index() const
        {
            return _value;
        }
};

template <typename ElfType_>
class DynamicEntries :
    public paludis::InstantiationPolicy<DynamicEntries<ElfType_>, paludis::instantiation_method::SingletonTag>,
    private paludis::PrivateImplementationPattern<DynamicEntries<ElfType_> >
{
    using paludis::PrivateImplementationPattern<DynamicEntries>::_imp;
    friend class paludis::InstantiationPolicy<DynamicEntries, paludis::instantiation_method::SingletonTag>;

    public:
        void register_type(typename ElfType_::DynamicTag, paludis::tr1::shared_ptr<DynamicEntry<ElfType_> >);

        paludis::tr1::shared_ptr<DynamicEntry<ElfType_> > get_entry(typename ElfType_::DynamicTag) const;
        bool has_entry(typename ElfType_::DynamicTag) const;

    private:
        DynamicEntries();
        ~DynamicEntries();
};

template <typename ElfType_>
class DynamicSection :
    public Section<ElfType_>,
    public paludis::AcceptInterfaceVisitsThis<SectionVisitorTypes<ElfType_> , DynamicSection<ElfType_> >,
    private paludis::PrivateImplementationPattern<DynamicSection<ElfType_> >
{
    using paludis::PrivateImplementationPattern<DynamicSection>::_imp;

    public:
        DynamicSection(const typename ElfType_::SectionHeader &, std::istream &, bool);
        virtual ~DynamicSection();

        virtual std::string get_type() const;

        void resolve_entry_names(Section<ElfType_> &);

        struct EntryIteratorTag;
        typedef paludis::WrappedForwardIterator<EntryIteratorTag, DynamicEntry<ElfType_> > EntryIterator;
        EntryIterator entry_begin() const;
        EntryIterator entry_end() const;
};

#endif /*ELFDYNAMICSECTION_HH_*/
