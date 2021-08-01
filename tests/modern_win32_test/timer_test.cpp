//
// Copyright © 2021 Terry Moreland
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

#pragma warning(push,2)
#include <gtest/gtest.h>
#pragma warning(pop)

#include <chrono>
#include <modern_win32/threading/timer.h>
#include <modern_win32/threading/event.h>
#include "context.h"

using std::chrono::milliseconds;
using modern_win32::threading::manual_reset_event;
using modern_win32::threading::delayed_callback;
using modern_win32::threading::synchronization_timer;

#pragma warning(disable : 4455)
using std::literals::chrono_literals::operator ""ms;
using std::literals::chrono_literals::operator ""s;

class timer_test : public testing::TestWithParam<std::chrono::milliseconds>
{
    std::chrono::milliseconds value_{ 0ms };
public:
    void SetUp() override
    {
        value_ = GetParam();
    }

protected:
    [[nodiscard]]
    auto value() const noexcept -> std::chrono::milliseconds const&
    {
        return value_;
    }
};

class delayed_callback_test : public timer_test
{
};

class synchronization_timer_test : public timer_test
{
};

class foo
{
    int x_{};
public:
    explicit foo() = default;

    [[nodiscard]]
    int const& x() const noexcept
    {
        return x_;
    }
    [[nodiscard]]
    int& x() noexcept
    {
        return x_;
    }
};

TEST(delayed_callback_test, constructor__does_not_throw__when_state_is_trivial)
{
    ASSERT_NO_THROW({
        delayed_callback<int> delay([](int&) { /* ... */ }, 3);
    });
}

TEST(delayed_callback_test, constructor__does_not_throw__when_state_is_reference)
{
    foo bar{};

    ASSERT_NO_THROW({
        delayed_callback<std::reference_wrapper<foo>> delay([](std::reference_wrapper<foo>&) {  /* ... */ }, std::reference_wrapper(bar));
    });
}

TEST_P(delayed_callback_test, start__throws_invalid_argument__when_due_time_is_less_than_zero)
{
    try {
        delayed_callback<int> delay([](int&) { /* ... */ }, 3);
        delay.start(value(), 100ms);
        FAIL();
    } catch (std::invalid_argument const& e) {
        ASSERT_EQ(e.what(), std::string_view("due_time must be greater than or equal to zero"));
    }
}

TEST_P(delayed_callback_test, start__throws_invalid_argument__when_poll_period_is_less_than_zero)
{
    try {
        delayed_callback<int> delay([](int&) { /* ... */ }, 3);
        delay.start(0ms, value());
        FAIL();
    } catch (std::invalid_argument const& e) {
        ASSERT_EQ(e.what(), std::string_view("period must be greater than or equal to zero"));
    }
}

TEST(delayed_callback_test, start__begins_timer__when_arguments_are_greater_than_or_equal_to_zero)
{
    manual_reset_event callback_event{ false };
    bool called{ false };
    int const expected_state = 3;
    auto callback = [&called, &callback_event, expected_state](int& state) {
        if (state == expected_state) {
            called = true;
            std::ignore = callback_event.set();
        }
    };

    delayed_callback<int, decltype(callback)> timer(callback, expected_state);

    timer.start(10ms, 100ms);

    std::ignore = callback_event.wait_one(1s);

    ASSERT_TRUE(called);
}


TEST(synchronization_timer_test, constructor__does_not_throw__when_state_is_trivial)
{
    ASSERT_NO_THROW({
        synchronization_timer<int> timer([](int&) { /* ... */ }, 3);
    });
}

TEST(synchronization_timer_test, constructor__does_not_throw__when_state_is_reference)
{
    foo bar{};

    ASSERT_NO_THROW({
        synchronization_timer<std::reference_wrapper<foo>> timer([](std::reference_wrapper<foo>&) {  /* ... */ }, std::reference_wrapper(bar));
    });
}

TEST(synchronization_timer_test, start__begins_timer__when_arguments_are_greater_than_or_equal_to_zero)
{
    manual_reset_event callback_event{ false };
    bool called{ false };
    int const expected_state = 3;
    auto callback = [&called, &callback_event, expected_state](int& state) {
        if (state == expected_state) {
            called = true;
            std::ignore = callback_event.set();
        }
    };

    synchronization_timer<int, decltype(callback)> timer(callback, expected_state);

    timer.start(10ms, 100ms);

    std::ignore = callback_event.wait_one(1s);

    ASSERT_TRUE(called);
}

TEST_P(synchronization_timer_test, start__throws_invalid_argument__when_due_time_is_less_than_zero)
{
    try {
        synchronization_timer<int> timer([](int&) { /* ... */ }, 3);
        timer.start(value(), 100ms);
        FAIL();
    } catch (std::invalid_argument const& e) {
        ASSERT_EQ(e.what(), std::string_view("due_time must be greater than or equal to zero"));
    }
}

TEST_P(synchronization_timer_test, start__throws_invalid_argument__when_poll_period_is_less_than_zero)
{
    try {
        delayed_callback<int> delay([](int&) { /* ... */ }, 3);
        delay.start(0ms, value());
        FAIL();
    } catch (std::invalid_argument const& e) {
        ASSERT_EQ(e.what(), std::string_view("period must be greater than or equal to zero"));
    }
}

INSTANTIATE_TEST_SUITE_P(data_driven, delayed_callback_test, testing::Values(-50ms, -100ms));
INSTANTIATE_TEST_SUITE_P(data_driven, synchronization_timer_test, testing::Values(-50ms, -100ms));
