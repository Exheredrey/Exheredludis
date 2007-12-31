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

#include <cstdlib>
#include <paludis/util/system.hh>
#include <paludis/util/log.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/destringify.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/destringify.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/fd_output_stream.hh>
#include <paludis/util/pipe.hh>

#include <sys/utsname.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <unistd.h>
#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <map>
#include <iostream>
#include "config.h"

using namespace paludis;

template class WrappedForwardIterator<Command::ConstIteratorTag, const std::pair<const std::string, std::string> >;

namespace
{
    static int stdout_write_fd = -1;
    static int stdout_close_fd = -1;

    static int stderr_write_fd = -1;
    static int stderr_close_fd = -1;

    pid_t get_paludis_pid()
    {
        pid_t result(0);
        std::string str;

        if (((str = getenv_with_default("PALUDIS_PID", ""))).empty())
        {
            result = getpid();
            setenv("PALUDIS_PID", stringify(result).c_str(), 1);
        }
        else
            result = destringify<pid_t>(str);

        return result;
    }


    static pid_t paludis_pid PALUDIS_ATTRIBUTE((used)) = get_paludis_pid();
}

void
paludis::set_run_command_stdout_fds(const int w, const int c)
{
    stdout_write_fd = w;
    stdout_close_fd = c;
}

void
paludis::set_run_command_stderr_fds(const int w, const int c)
{
    stderr_write_fd = w;
    stderr_close_fd = c;
}

GetenvError::GetenvError(const std::string & key) throw () :
    Exception("Environment variable '" + key + "' not set")
{
}

RunCommandError::RunCommandError(const std::string & our_message) throw () :
    Exception(our_message)
{
}

std::string
paludis::getenv_with_default(const std::string & key, const std::string & def)
{
    const char * const e(std::getenv(key.c_str()));
    return e ? e : def;
}

std::string
paludis::getenv_or_error(const std::string & key)
{
    const char * const e(std::getenv(key.c_str()));
    if (! e)
        throw GetenvError(key);
    return e;
}

namespace
{
    /**
     * Fetch the kernel version, for paludis::kernel_version.
     *
     * \ingroup grpsystem
     */
    std::string get_kernel_version()
    {
        struct utsname u;
        if (0 != uname(&u))
            throw InternalError(PALUDIS_HERE, "uname call failed");
        return u.release;
    }
}

std::string
paludis::kernel_version()
{
    static const std::string result(get_kernel_version());
    return result;
}

namespace paludis
{
    template<>
    struct Implementation<Command>
    {
        std::string command;
        std::map<std::string, std::string> setenv_values;
        std::string chdir;
        bool echo_to_stderr;
        tr1::shared_ptr<uid_t> uid;
        tr1::shared_ptr<gid_t> gid;
        std::string stdout_prefix;
        std::string stderr_prefix;
        bool prefix_discard_blank_output;
        bool prefix_blank_lines;
        tr1::function<std::string (const std::string &)> pipe_command_handler;
        std::ostream * captured_stdout_stream;

        Implementation(const std::string & c,
                const std::map<std::string, std::string> & s = (std::map<std::string, std::string>()),
                const std::string & d = "", bool e = false,
                tr1::shared_ptr<uid_t> u = tr1::shared_ptr<uid_t>(),
                tr1::shared_ptr<gid_t> g = tr1::shared_ptr<gid_t>(),
                const std::string & p = "", const std::string & q = "",
                const bool b = false, const bool bb = false,
                const tr1::function<std::string (const std::string &)> & h = tr1::function<std::string (const std::string &)>(),
                std::ostream * cs = 0) :
            command(c),
            setenv_values(s),
            chdir(d),
            echo_to_stderr(e),
            uid(u),
            gid(g),
            stdout_prefix(p),
            stderr_prefix(q),
            prefix_discard_blank_output(b),
            prefix_blank_lines(bb),
            pipe_command_handler(h),
            captured_stdout_stream(cs)
        {
        }
    };
}

Command::Command(const std::string & s) :
    PrivateImplementationPattern<Command>(new Implementation<Command>(s))
{
}

Command::Command(const char * const s) :
    PrivateImplementationPattern<Command>(new Implementation<Command>(s))
{
}

Command::Command(const Command & other) :
    PrivateImplementationPattern<Command>(new Implementation<Command>(other._imp->command,
                other._imp->setenv_values, other._imp->chdir, other._imp->echo_to_stderr,
                other._imp->uid, other._imp->gid, other._imp->stdout_prefix, other._imp->stderr_prefix,
                other._imp->prefix_discard_blank_output,
                other._imp->prefix_blank_lines, other._imp->pipe_command_handler, other._imp->captured_stdout_stream))
{
}

const Command &
Command::operator= (const Command & other)
{
    if (this != &other)
    {
        _imp.reset(new Implementation<Command>(other._imp->command, other._imp->setenv_values,
                    other._imp->chdir, other._imp->echo_to_stderr,
                    tr1::shared_ptr<uid_t>(),
                    tr1::shared_ptr<gid_t>(),
                    other._imp->stdout_prefix,
                    other._imp->stderr_prefix,
                    other._imp->prefix_discard_blank_output,
                    other._imp->prefix_blank_lines,
                    other._imp->pipe_command_handler, other._imp->captured_stdout_stream));
        if (other.uid() && other.gid())
            with_uid_gid(*other.uid(), *other.gid());
    }

    return *this;
}

Command::~Command()
{
}

Command &
Command::with_chdir(const FSEntry & c)
{
    _imp->chdir = stringify(c);
    return *this;
}

Command &
Command::with_setenv(const std::string & k, const std::string & v)
{
    _imp->setenv_values.insert(std::make_pair(k, v));
    return *this;
}

Command &
Command::with_uid_gid(const uid_t u, const gid_t g)
{
    _imp->uid.reset(new uid_t(u));
    _imp->gid.reset(new gid_t(g));
    return *this;
}

Command &
Command::with_captured_stdout_stream(std::ostream * const c)
{
    _imp->captured_stdout_stream = c;
    return *this;
}

Command &
Command::with_sandbox()
{
#if HAVE_SANDBOX
    if (! getenv_with_default("PALUDIS_DO_NOTHING_SANDBOXY", "").empty())
        Log::get_instance()->message(ll_debug, lc_no_context,
                "PALUDIS_DO_NOTHING_SANDBOXY is set, not using sandbox");
    else if (! getenv_with_default("SANDBOX_ACTIVE", "").empty())
        Log::get_instance()->message(ll_warning, lc_no_context,
                "Already inside sandbox, not spawning another sandbox instance");
    else
        _imp->command = "sandbox " + _imp->command;
#endif

    return *this;
}

tr1::shared_ptr<const uid_t>
Command::uid() const
{
    return _imp->uid;
}

tr1::shared_ptr<const gid_t>
Command::gid() const
{
    return _imp->gid;
}

int
paludis::run_command(const Command & cmd)
{
    Context context("When running command '" + stringify(cmd.command()) + "':");

    std::string extras;

    if (! cmd.chdir().empty())
        extras.append(" [chdir " + cmd.chdir() + "]");

    for (Command::ConstIterator s(cmd.begin_setenvs()), s_end(cmd.end_setenvs()) ; s != s_end ; ++s)
        extras.append(" [setenv " + s->first + "=" + s->second + "]");

    if (cmd.gid() && *cmd.gid() != getgid())
        extras.append(" [setgid " + stringify(*cmd.gid()) + "]");

    if (cmd.uid() && *cmd.uid() != getuid())
        extras.append(" [setuid " + stringify(*cmd.uid()) + "]");

    std::string command(cmd.command());

    if ((! cmd.stdout_prefix().empty()) || (! cmd.stderr_prefix().empty()))
        command = getenv_with_default("PALUDIS_OUTPUTWRAPPER_DIR", LIBEXECDIR "/paludis/utils") + "/outputwrapper --stdout-prefix '"
            + cmd.stdout_prefix() + "' --stderr-prefix '" + cmd.stderr_prefix() + "' "
            + (cmd.prefix_discard_blank_output() ? " --discard-blank-output " : "")
            + (cmd.prefix_blank_lines() ? " --wrap-blanks " : "")
            + " -- " + command;

    cmd.echo_to_stderr();
    Log::get_instance()->message(ll_debug, lc_no_context, "execl /bin/sh -c " + command
            + " " + extras);

    tr1::shared_ptr<Pipe> internal_command_reader(new Pipe), pipe_command_reader, pipe_command_response, captured_stdout;
    if (cmd.pipe_command_handler())
    {
        pipe_command_reader.reset(new Pipe);
        pipe_command_response.reset(new Pipe);
    }
    if (cmd.captured_stdout_stream())
        captured_stdout.reset(new Pipe);

    pid_t child(fork());
    if (0 == child)
    {
        pid_t child_child(fork());
        if (0 == child_child)
        {
            /* The pid that does the exec */
            try
            {
                if (cmd.pipe_command_handler())
                {
                    close(pipe_command_reader->read_fd());
                    pipe_command_reader->clear_read_fd();

                    close(pipe_command_response->write_fd());
                    pipe_command_response->clear_write_fd();
                }

                if (cmd.captured_stdout_stream())
                {
                    close(captured_stdout->read_fd());
                    captured_stdout->clear_read_fd();
                }

                close(internal_command_reader->read_fd());
                internal_command_reader->clear_read_fd();
                close(internal_command_reader->write_fd());
                internal_command_reader->clear_write_fd();

                if (! cmd.chdir().empty())
                    if (-1 == chdir(stringify(cmd.chdir()).c_str()))
                        throw RunCommandError("chdir failed: " + stringify(strerror(errno)));

                for (Command::ConstIterator s(cmd.begin_setenvs()), s_end(cmd.end_setenvs()) ; s != s_end ; ++s)
                    setenv(s->first.c_str(), s->second.c_str(), 1);

                setenv("PATH_NOT_CLOBBERED_BY_SANDBOX", getenv_with_default("PATH", "").c_str(), 1);

                if (cmd.pipe_command_handler())
                {
                    setenv("PALUDIS_PIPE_COMMAND_WRITE_FD", stringify(pipe_command_reader->write_fd()).c_str(), 1);
                    setenv("PALUDIS_PIPE_COMMAND_READ_FD", stringify(pipe_command_response->read_fd()).c_str(), 1);
                }

                if (cmd.captured_stdout_stream())
                {
                    if (-1 == dup2(captured_stdout->write_fd(), 1))
                        throw RunCommandError("dup2 failed: " + stringify(strerror(errno)));
                }
                else if (-1 != stdout_write_fd)
                {
                    if (-1 == dup2(stdout_write_fd, 1))
                        throw RunCommandError("dup2 failed: " + stringify(strerror(errno)));

                    if (-1 != stdout_close_fd)
                        close(stdout_close_fd);
                }

                if (-1 != stderr_write_fd)
                {
                    if (-1 == dup2(stderr_write_fd, 2))
                        throw RunCommandError("dup2 failed: " + stringify(strerror(errno)));

                    if (-1 != stderr_close_fd)
                        close(stderr_close_fd);
                }

                if (cmd.gid() && *cmd.gid() != getgid())
                {
                    gid_t g(*cmd.gid());

                    if (0 != ::setgid(*cmd.gid()))
                        std::cerr << "setgid(" << *cmd.uid() << ") failed for exec of '" << command << "': "
                            << strerror(errno) << std::endl;
                    else if (0 != ::setgroups(1, &g))
                        std::cerr << "setgroups failed for exec of '" << command << "': " << strerror(errno) << std::endl;
                }

                if (cmd.uid() && *cmd.uid() != getuid())
                    if (0 != ::setuid(*cmd.uid()))
                        std::cerr << "setuid(" << *cmd.uid() << ") failed for exec of '" << command << "': "
                            << strerror(errno) << std::endl;

                execl("/bin/sh", "sh", "-c", command.c_str(), static_cast<char *>(0));
                throw RunCommandError("execl /bin/sh -c '" + command + "' failed:"
                        + stringify(strerror(errno)));
            }
            catch (const Exception & e)
            {
                std::cerr << "exec of '" << command << "' failed due to exception '" << e.message()
                    << "' (" << e.what() << ")" << std::endl;
                exit(123);
            }
            catch (...)
            {
                std::cerr << "exec of '" << command << "' failed due to unknown exception" << std::endl;
                exit(124);
            }
        }
        else if (-1 == child_child)
        {
            std::cerr << "fork failed: " + stringify(strerror(errno)) + "'" << std::endl;
            exit(125);
        }
        else
        {
            /* The pid that waits for the exec pid and then writes to the done pipe */
            if (cmd.pipe_command_handler())
            {
                close(pipe_command_reader->read_fd());
                pipe_command_reader->clear_read_fd();
                close(pipe_command_reader->write_fd());
                pipe_command_reader->clear_write_fd();

                close(pipe_command_response->read_fd());
                pipe_command_response->clear_read_fd();
                close(pipe_command_response->write_fd());
                pipe_command_response->clear_write_fd();
            }

            if (cmd.captured_stdout_stream())
            {
                close(captured_stdout->read_fd());
                captured_stdout->clear_read_fd();
                close(captured_stdout->write_fd());
                captured_stdout->clear_write_fd();
            }

            close(internal_command_reader->read_fd());
            internal_command_reader->clear_read_fd();

            int status(-1);

            stdout_write_fd = -1;
            stdout_close_fd = -1;
            stderr_write_fd = -1;
            stderr_close_fd = -1;

            int ret(-1);
            if (-1 == waitpid(child_child, &status, 0))
                std::cerr << "wait failed: " + stringify(strerror(errno)) + "'" << std::endl;
            else
                ret = (WIFSIGNALED(status) ? WTERMSIG(status) + 128 : WEXITSTATUS(status));

            {
                FDOutputStream stream(internal_command_reader->write_fd());
                stream << "EXIT " << ret << std::endl;
            }
        }

        _exit(0);
    }
    else if (-1 == child)
        throw RunCommandError("fork failed: " + stringify(strerror(errno)));
    else
    {
        /* Our original pid */
        if (cmd.pipe_command_handler())
        {
            close(pipe_command_reader->write_fd());
            pipe_command_reader->clear_write_fd();
            close(pipe_command_response->read_fd());
            pipe_command_response->clear_read_fd();
        }

        if (cmd.captured_stdout_stream())
        {
            close(captured_stdout->write_fd());
            captured_stdout->clear_write_fd();
        }

        close(internal_command_reader->write_fd());
        internal_command_reader->clear_write_fd();

        std::string pipe_command_buffer, internal_command_buffer;
        while (true)
        {
            fd_set read_fds;
            FD_ZERO(&read_fds);
            int max_fd(0);

            if (cmd.pipe_command_handler())
            {
                FD_SET(pipe_command_reader->read_fd(), &read_fds);
                max_fd = std::max(max_fd, pipe_command_reader->read_fd());
            }

            if (cmd.captured_stdout_stream())
            {
                FD_SET(captured_stdout->read_fd(), &read_fds);
                max_fd = std::max(max_fd, captured_stdout->read_fd());
            }

            FD_SET(internal_command_reader->read_fd(), &read_fds);
            max_fd = std::max(max_fd, internal_command_reader->read_fd());

            timespec tv;
            tv.tv_sec = 5;
            tv.tv_nsec = 0;

            int retval(pselect(max_fd + 1, &read_fds, 0, 0, &tv, 0));
            if (-1 == retval)
                throw RunCommandError("select failed: " + stringify(strerror(errno)));
            else if (0 == retval)
            {
                Log::get_instance()->message(ll_debug, lc_context) << "Waiting for child " << child << " to finish";
                continue;
            }
            else
            {
                char buf[1024];

                if (cmd.captured_stdout_stream() && FD_ISSET(captured_stdout->read_fd(), &read_fds))
                {
                    int r;
                    if (((r = read(captured_stdout->read_fd(), buf, 1024))) > 0)
                    {
                        *cmd.captured_stdout_stream() << std::string(buf, r);
                        /* don't look at the other FDs yet to avoid a partial read from being snipped
                         * when capturing output */
                        continue;
                    }
                }

                if (cmd.pipe_command_handler() && FD_ISSET(pipe_command_reader->read_fd(), &read_fds))
                {
                    int r;
                    if (((r = read(pipe_command_reader->read_fd(), buf, 1024))) > 0)
                        pipe_command_buffer.append(std::string(buf, r));
                }

                if (FD_ISSET(internal_command_reader->read_fd(), &read_fds))
                {
                    int r;
                    if (((r = read(internal_command_reader->read_fd(), buf, 1024))) > 0)
                        internal_command_buffer.append(std::string(buf, r));
                }
            }

            if (! pipe_command_buffer.empty())
                Log::get_instance()->message(ll_debug, lc_context) << "pipe_command_buffer is '" << pipe_command_buffer << "'";
            if (! internal_command_buffer.empty())
                Log::get_instance()->message(ll_debug, lc_context) << "internal_command_buffer is '" << internal_command_buffer << "'";

            while (! pipe_command_buffer.empty())
            {
                std::string::size_type n_p(pipe_command_buffer.find('\0'));
                if (std::string::npos == n_p)
                    break;

                std::string op(pipe_command_buffer.substr(0, n_p));
                pipe_command_buffer.erase(0, n_p + 1);

                std::string response;
                if (cmd.pipe_command_handler())
                {
                    response = cmd.pipe_command_handler()(op);
                    Log::get_instance()->message(ll_debug, lc_context) << "Pipe command op '" << op << "' response '"
                        << response << "'";
                }
                else
                    Log::get_instance()->message(ll_warning, lc_context) << "Pipe command op '" << op <<
                        "' was requested but no handler defined. This is probably a bug...";

                ssize_t n(0);
                while (! response.empty())
                {
                    n = write(pipe_command_response->write_fd(), response.c_str(), response.length());
                    if (-1 == n)
                        throw InternalError(PALUDIS_HERE, "write failed: " + stringify(strerror(errno)));
                    else
                        response.erase(0, n);
                }

                char c(0);
                n = write(pipe_command_response->write_fd(), &c, 1);
                if (1 != n)
                    throw InternalError(PALUDIS_HERE, "write failed: " + stringify(strerror(errno)));
            }

            while (! internal_command_buffer.empty())
            {
                std::string::size_type n_p(internal_command_buffer.find('\n'));
                if (std::string::npos == n_p)
                    break;

                std::string op(internal_command_buffer.substr(0, n_p));
                internal_command_buffer.erase(0, n_p + 1);
                if (0 == op.compare(0, 4, "EXIT"))
                {
                    op.erase(0, 4);
                    int status(-1);
                    Log::get_instance()->message(ll_debug, lc_context) << "Got exit op '" << op << "'";
                    if (-1 == waitpid(child, &status, 0))
                        std::cerr << "wait failed: " + stringify(strerror(errno)) + "'" << std::endl;
                    return destringify<int>(strip_leading(strip_trailing(op, " \r\n\t"), " \r\n\t"));
                }
                else
                    throw InternalError(PALUDIS_HERE, "unknown op '" + op + "' on internal_command_buffer");
            }
        }
    }

    throw InternalError(PALUDIS_HERE, "should never be reached");
}

std::string
Command::command() const
{
    return _imp->command;
}

std::string
Command::chdir() const
{
    return _imp->chdir;
}

Command::ConstIterator
Command::begin_setenvs() const
{
    return ConstIterator(_imp->setenv_values.begin());
}

Command::ConstIterator
Command::end_setenvs() const
{
    return ConstIterator(_imp->setenv_values.end());
}

void
Command::echo_to_stderr() const
{
    if (! _imp->echo_to_stderr)
        return;

    std::cerr << command().c_str() << std::endl;
}

Command &
Command::with_echo_to_stderr()
{
    _imp->echo_to_stderr = true;
    return *this;
}

Command &
Command::with_prefix_discard_blank_output()
{
    _imp->prefix_discard_blank_output = true;
    return *this;
}

Command &
Command::with_prefix_blank_lines()
{
    _imp->prefix_blank_lines = true;
    return *this;
}

Command &
Command::with_stdout_prefix(const std::string & s)
{
    _imp->stdout_prefix = s;
    return *this;
}

Command &
Command::with_stderr_prefix(const std::string & s)
{
    _imp->stderr_prefix = s;
    return *this;
}

Command &
Command::with_pipe_command_handler(const tr1::function<std::string (const std::string &)> & f)
{
    _imp->pipe_command_handler = f;
    return *this;
}

std::string
Command::stdout_prefix() const
{
    return _imp->stdout_prefix;
}

std::string
Command::stderr_prefix() const
{
    return _imp->stderr_prefix;
}

bool
Command::prefix_discard_blank_output() const
{
    return _imp->prefix_discard_blank_output;
}

bool
Command::prefix_blank_lines() const
{
    return _imp->prefix_blank_lines;
}

const tr1::function<std::string (const std::string &)> &
Command::pipe_command_handler() const
{
    return _imp->pipe_command_handler;
}

std::ostream *
Command::captured_stdout_stream() const
{
    return _imp->captured_stdout_stream;
}

std::string
paludis::get_user_name(const uid_t u)
{
    const struct passwd * const p(getpwuid(u));
    if (p)
        return stringify(p->pw_name);
    else
    {
        Context c("When getting user name for uid '" + stringify(u) + "':");
        Log::get_instance()->message(ll_warning, lc_context, "getpwuid("
                + stringify(u) + ") returned null");
        return stringify(u);
    }
}

std::string
paludis::get_group_name(const gid_t u)
{
    const struct group * const p(getgrgid(u));
    if (p)
        return stringify(p->gr_name);
    else
    {
        Context c("When getting group name for gid '" + stringify(u) + "':");
        Log::get_instance()->message(ll_warning, lc_context, "getgrgid("
                + stringify(u) + ") returned null");
        return stringify(u);
    }
}

