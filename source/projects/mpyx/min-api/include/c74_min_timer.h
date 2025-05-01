/// @file
///	@ingroup 	minapi
///	@copyright	Copyright 2018 The Min-API Authors. All rights reserved.
///	@license	Use of this source code is governed by the MIT License found in the License.md file.

#pragma once

namespace c74::min {

/// Options that control the behavior of the timer.
enum class timer_options
{
    deliver_on_scheduler, ///< The default behavior delivers events on Max's scheduler thread
    defer_delivery ///< Defers events from the scheduler to Max's main thread
};

template <timer_options options = timer_options::deliver_on_scheduler>
class timer;

class timer_base;

static const char* timer_impl_name = "min_timer_impl";

// important! this class is used by all min-based externals that use min's timer class. If you make significant changes to the
// timer_impl object we're registering here, consider changing the timer_impl_name to avoid conflicts with other externals that
// use an older version of min-api.
struct timer_impl
{
    max::t_object m_obj;
    timer_base* m_owner;
};

static max::t_class* s_timer_impl_class = nullptr;

extern "C" void timer_tick_callback(timer_impl* an_owner); // defined in c74_min_impl.h
extern "C" void timer_qfn_callback(timer_impl* a_timer); // defined in c74_min_impl.h

class timer_base
{
  public:
    ~timer_base()
    {
        max::object_free(m_timer_impl);
        max::object_free(m_instance);
        if (m_qelem) {
            max::qelem_free(m_qelem);
        }
    }

    // Timers cannot be copied.
    // If they are then the ownership of the internal t_clock becomes ambiguous.

    timer_base(const timer_base&) = delete;
    timer_base& operator=(const timer_base& value) = delete;

    /// Set the timer to fire after a specified delay.
    /// When the timer fires its function will be executed.
    /// @param	duration_in_ms	The length of the delay (from "now") before the timer fires.
    void delay(const double duration_in_ms)
    {
        max::clock_fdelay(m_instance, duration_in_ms);
    }

    /// Stop a timer that has been previously set using the delay() call.
    void stop()
    {
        if (m_instance) {
            max::clock_unset(m_instance);
        }
    }

    /// Execute the timer's function immediately / synchronously.
    void tick()
    {
        atoms a;
        m_function(a, -1);
    }

    /// Execute the timer's function immediately / synchronously.
    void operator()()
    {
        tick();
    }

    /// Determine if the timer executes it's function by deferring.
    /// @return		True if the timer defers to the main thread. Otherwise false.
    bool should_defer()
    {
        return m_qelem;
    }

    /// post information about the timer to the console
    // also serves the purpose of eliminating warnings about m_owner being unused
    void post()
    {
        std::cout << m_instance << &m_function << m_owner << std::endl;
    }

    void tick_callback()
    {
        if (should_defer()) {
            defer();
        }
        else {
            tick();
        }
    }

    void qfn_callback()
    {
        tick();
    }

  protected:
    timer_base(object_base* an_owner, timer_options options, const function a_function)
        : m_owner{ an_owner }
        , m_function{ a_function }
    {

        if (!s_timer_impl_class) {
            // the timer_impl Max object is potentially already registered by another min based object
            if (const auto registered_class = max::class_findbyname(const_cast<max::t_symbol*>(max::CLASS_NOBOX), max::gensym(timer_impl_name))) {
                s_timer_impl_class = registered_class;
            }
            else {
                // important! this class is used by all min-based externals that use min's timer class. If you make significant changes to the
                // timer_impl object we're registering here, consider changing the timer_impl_name to avoid conflicts with other externals that
                // use an older version of min-api.
                s_timer_impl_class = max::class_new(timer_impl_name, (max::method)0, (max::method)0, sizeof(timer_impl), (max::method)0, 0);
                max::class_register(max::CLASS_NOBOX, s_timer_impl_class);
            }
        }

        m_timer_impl = (timer_impl*)max::object_alloc(s_timer_impl_class);
        m_timer_impl->m_owner = this;
        const auto s = max::scheduler_fromobject(an_owner->maxobj());
        max::object_obex_storeflags(m_timer_impl, max::gensym("#S"), reinterpret_cast<max::t_object*>(s), max::OBJ_FLAG_REF);

        m_instance = max::clock_new(m_timer_impl, reinterpret_cast<max::method>(timer_tick_callback));
        if (options == timer_options::defer_delivery) {
            m_qelem = max::qelem_new(m_timer_impl, reinterpret_cast<max::method>(timer_qfn_callback));
        }
    }

  private:
    object_base* m_owner;
    function m_function;
    max::t_clock* m_instance{ nullptr };
    max::t_qelem* m_qelem{ nullptr };
    timer_impl* m_timer_impl;

    friend void timer_tick_callback(timer_impl* an_owner);

    void defer()
    {
        max::qelem_set(m_qelem);
    }
};

/// The timer class allows you to schedule a function to be called in the future using Max's scheduler.
/// Note: the name `timer` was chosen instead of `clock` because of the use of the type is `clock` is ambiguous on
/// the Mac OS when not explicitly specifying the `c74::min` namespace.
/// @tparam	options		Optional argument to alter the delivery from the scheduler thread to the main thread.
///
/// @seealso	#time_value
/// @seealso	#queue
/// @seealso	#fifo
template <timer_options options>
class timer : public timer_base
{
  public:
    /// Create a timer.
    /// @param	an_owner	The owning object for the timer. Typically you will pass `this`.
    /// @param	a_function	A function to be executed when the timer is called.
    ///				Typically the function is defined using a C++ lambda with the #MIN_FUNCTION signature.

    timer(object_base* an_owner, const function a_function)
        : timer_base(an_owner, options, a_function)
    {
    }

    // Timers cannot be copied.
    // If they are then the ownership of the internal t_clock becomes ambiguous.
    timer(const timer&) = delete;
    timer& operator=(const timer& value) = delete;
};

} // namespace c74::min
