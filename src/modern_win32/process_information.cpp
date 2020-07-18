//
// Copyright � 2020 Terry Moreland
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), 
// to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, 
// and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
// WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
// 
    
#include <modern_win32/process_information.h>
#include <modern_win32/wait_for.h>
#include <memory>

namespace modern_win32
{

process_information::process_information()
    : m_process_thread_id{0UL}
    , m_process{}
{
}
/// <summary>Initializes an open process_information class based on <paramref name="process_information"/></summary>
process_information::process_information(native_handle_type const& process_information)
    : m_process_thread_id{process_information.dwThreadId}
    , m_process{process_information.hProcess}
    , m_process_thread_handle{process_information.hThread}
{
}
process_information::process_information(process_information&& other) noexcept 
{
    auto [process_id, process_thread_id, process_handle, process_thread_handle] = other.release();
    static_cast<void>(process_id);
    m_process_thread_id = process_thread_id;
    static_cast<void>(m_process.reset(process_handle));
    static_cast<void>(m_process_thread_handle.reset(process_thread_handle));
}
process_information& process_information::operator=(process_information&& other) noexcept 
{
    if (*this == other)
        return *this;

    std::swap(*this, other);
    return *this;
}
process_information::~process_information() 
{
    close(); // done to match unique_handle but not really needed, default would be fine
}

process_information::native_handle_type process_information::native_handle() const 
{
    native_handle_type process_information;
    process_information.hProcess = m_process.native_handle();
    process_information.hThread = m_process_thread_handle.native_handle();

     process_information.dwProcessId = static_cast<bool>(m_process)
         ?  m_process.get_process_id().value_or(0UL)
         : 0UL;
    process_information.dwThreadId = m_process_thread_id;
    return process_information;
}
process_information::native_process_handle_type process_information::get_native_process_handle() const noexcept
{
    return m_process.native_handle();
}
process_information::native_process_thread_handle_type process_information::get_native_process_thread_handle() const noexcept
{
    return m_process_thread_handle.native_handle();
}
std::optional<process_id_type> process_information::get_process_id() const 
{
    return m_process.get_process_id();
}
process_information::native_process_thread_id process_information::get_native_process_thread_id() const noexcept
{
    return m_process_thread_id;
}
process_handle& process_information::get_process_handle() noexcept
{
    return m_process.get();
}
process_thread_handle& process_information::get_process_thread_handle() noexcept
{
    return m_process_thread_handle;
}

bool process_information::is_running() const 
{
    return m_process.is_running();
}

void process_information::wait_for_exit() const
{
    m_process.wait_for_exit();
}
bool process_information::wait_for_exit(std::chrono::milliseconds const& timeout) const
{
    return m_process.wait_for_exit(timeout);
}

std::optional<process_information::exit_code_type> process_information::get_exit_code() const
{
    return m_process.get_exit_code();
}

bool process_information::reset(deconstruct_type&& deconstructed)
{
    if (*this == deconstructed)
        return static_cast<bool>(*this);

    close();
    auto [process_id, process_thread_id, process_handle, process_thread_handle] = std::move(deconstructed);
    static_cast<void>(process_id);
    m_process_thread_id = process_thread_id;
    static_cast<void>(m_process.reset(process_handle));
    static_cast<void>(m_process_thread_handle.reset(process_thread_handle));

    return static_cast<bool>(*this);
}
bool process_information::reset(native_handle_type const& handle)
{
    if (*this == handle)
        return static_cast<bool>(*this);

    close(); // no check for open needed, handled by the handle classes
    m_process_thread_id = handle.dwThreadId;
    static_cast<void>(m_process.reset(handle.hProcess));
    static_cast<void>(m_process_thread_handle.reset(handle.hThread));

    return static_cast<bool>(*this);
}
process_information::deconstruct_type process_information::release()
{
    auto deconstructed = std::make_tuple(m_process.get_process_id().value_or(0UL), m_process_thread_id, m_process.release(), m_process_thread_handle.release());
    m_process_thread_id = 0UL;
    return deconstructed;
}

bool operator==(process_information const& lhs, process_information const& rhs)
{
    return &lhs == &rhs ||
        lhs.m_process_thread_id == rhs.m_process_thread_id &&
        lhs.m_process == rhs.m_process &&
        lhs.m_process_thread_handle == rhs.m_process_thread_handle;
}
bool operator!=(process_information const& lhs, process_information const& rhs)
{
    return !(lhs == rhs);
}
bool operator==(process_information const& lhs, PROCESS_INFORMATION const& rhs)
{
    return lhs.m_process_thread_id == rhs.dwThreadId &&
        lhs.m_process.native_handle() == rhs.hProcess &&
        lhs.m_process_thread_handle.native_handle() == rhs.hThread;
}
bool operator!=(process_information const& lhs, PROCESS_INFORMATION const& rhs)
{
    return !(lhs == rhs);
}
bool operator==(PROCESS_INFORMATION const& lhs, process_information const& rhs)
{
    return operator==(rhs, lhs);
}
bool operator!=(PROCESS_INFORMATION const& lhs, process_information const& rhs)
{
    return !(lhs == rhs);
}
bool operator==(process_information const& lhs, process_information::deconstruct_type const& rhs)
{
    return lhs.m_process_thread_id == std::get<1>(rhs) &&
        lhs.m_process.native_handle() == std::get<2>(rhs) &&
        lhs.m_process_thread_handle.native_handle() == std::get<3>(rhs);
}
bool operator!=(process_information const& lhs, process_information::deconstruct_type const& rhs)
{
    return !(lhs == rhs);
}
bool operator==(process_information::deconstruct_type const& lhs, process_information const& rhs)
{
    return (rhs == lhs);
}
bool operator!=(process_information::deconstruct_type const& lhs, process_information const& rhs)
{
    return !(lhs == rhs);
}

process_information::operator bool() const noexcept
{
    return static_cast<bool>(m_process) && static_cast<bool>(m_process_thread_handle);
}

void swap(process_information& lhs, process_information& rhs) noexcept
{
    using std::swap;
    swap(lhs.m_process_thread_id, rhs.m_process_thread_id);
    swap(lhs.m_process, rhs.m_process);
    swap(lhs.m_process_thread_handle, rhs.m_process_thread_handle);
}

void process_information::close()
{
    if (*this)
    {
        m_process_thread_id = 0UL;
        static_cast<void>(m_process.reset());
        static_cast<void>(m_process_thread_handle.reset());
    }
}

}