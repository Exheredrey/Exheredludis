/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh
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

#include <iostream>
#include <exception>
#include <paludis/util/log.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/instantiation_policy-impl.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/action_queue.hh>
#include "config.h"

#ifdef PALUDIS_ENABLE_THREADS
#  ifdef __linux__
#    include <sys/time.h>
#    include <sys/resource.h>
#    include <unistd.h>
#    include <sys/syscall.h>
#  endif
#endif

using namespace paludis;

#include <paludis/util/log-se.cc>

template class InstantiationPolicy<Log, instantiation_method::SingletonTag>;

namespace paludis
{
    template<>
    struct Implementation<Log>
    {
        LogLevel log_level;
        std::ostream * stream;
        std::string program_name;
        std::string previous_context;

        mutable ActionQueue action_queue;

        Implementation() :
            log_level(ll_qa),
            stream(&std::cerr),
            program_name("paludis"),
            action_queue(1, false, false)
        {
        }

        void message(const LogLevel l, const LogContext c, const std::string cs, const std::string s)
        {
            if (l >= log_level)
            {
                *stream << program_name << "@" << ::time(0) << ": ";
                do
                {
                    switch (l)
                    {
                        case ll_debug:
                            *stream << "[DEBUG] ";
                            continue;

                        case ll_qa:
                            *stream << "[QA] ";
                            continue;

                        case ll_warning:
                            *stream << "[WARNING] ";
                            continue;

                        case ll_silent:
                            throw InternalError(PALUDIS_HERE, "ll_silent used for a message");

                        case last_ll:
                            break;
                    }

                    throw InternalError(PALUDIS_HERE, "Bad value for log_level");

                } while (false);

                if (lc_context == c)
                {
                    if (previous_context == cs)
                        *stream << "(same context) " << s << std::endl;
                    else
                        *stream << cs << s << std::endl;
                    previous_context = cs;
                }
                else
                    *stream << s << std::endl;
            }
        }

        void set_log_level(const LogLevel l)
        {
            log_level = l;
        }

        void get_log_level(LogLevel & l) const
        {
            l = log_level;
        }

        void set_program_name(const std::string & s)
        {
            program_name = s;
        }

        void set_log_stream(std::ostream * const s)
        {
            stream = s;
        }
    };
}

Log::Log() :
    PrivateImplementationPattern<Log>(new Implementation<Log>)
{
}

Log::~Log()
{
}

void
Log::set_log_level(const LogLevel l)
{
    _imp->action_queue.enqueue(tr1::bind(tr1::mem_fn(&Implementation<Log>::set_log_level), _imp.get(), l));
}

LogLevel
Log::log_level() const
{
    LogLevel result(static_cast<LogLevel>(1337));
    _imp->action_queue.enqueue(tr1::bind(tr1::mem_fn(&Implementation<Log>::get_log_level), _imp.get(), tr1::ref(result)));
    _imp->action_queue.complete_pending();
    return result;
}

void
Log::_message(const LogLevel l, const LogContext c, const std::string & s)
{
    if (lc_context == c)
        _imp->action_queue.enqueue(tr1::bind(tr1::mem_fn(&Implementation<Log>::message), _imp.get(), l, c,
#ifdef PALUDIS_ENABLE_THREADS
#  ifdef __linux__
                    "In thread ID '" + stringify(syscall(SYS_gettid)) + "':\n  ... " +
#  else
#    warning "Don't know how to get a thread ID on your platform"
#  endif
#endif
                    Context::backtrace("\n  ... "), s));
    else
        _imp->action_queue.enqueue(tr1::bind(tr1::mem_fn(&Implementation<Log>::message), _imp.get(), l, c, "", s));
}

LogMessageHandler::LogMessageHandler(const LogMessageHandler & o) :
    _message(o._message),
    _log_level(o._log_level),
    _log_context(o._log_context)
{
}

void
Log::message(const LogLevel l, const LogContext c, const std::string & s)
{
    _message(l, c, s);
}

LogMessageHandler
Log::message(const LogLevel l, const LogContext c)
{
    return LogMessageHandler(this, l, c);
}

void
Log::set_log_stream(std::ostream * const s)
{
    _imp->action_queue.enqueue(tr1::bind(tr1::mem_fn(&Implementation<Log>::set_log_stream), _imp.get(), s));
}

void
Log::complete_pending() const
{
    _imp->action_queue.complete_pending();
}

void
Log::set_program_name(const std::string & s)
{
    _imp->action_queue.enqueue(tr1::bind(tr1::mem_fn(&Implementation<Log>::set_program_name), _imp.get(), s));
}

LogMessageHandler::LogMessageHandler(Log * const ll, const LogLevel l, const LogContext c) :
    _log(ll),
    _log_level(l),
    _log_context(c)
{
}

void
LogMessageHandler::_append(const std::string & s)
{
    _message.append(s);
}

LogMessageHandler::~LogMessageHandler()
{
    if (! std::uncaught_exception() && ! _message.empty())
        _log->_message(_log_level, _log_context, _message);
}

