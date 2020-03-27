#pragma once

#include <ostream>

#include <rx/core/types.h>

namespace nova::mem {
    /**
     * \addtogroup allocation
     * \{
     */
    /**
     * \brief Represents a certain amount of bytes
     *
     * A byte type that offers easy conversion between various bases as well as type safety regarding arguments to
     * functions having the expected units. Functions returning an integer count return the nearest integer to the real
     * value, rounding as needed.
     *
     * Converting between the byte subclasses offers no accuracy loss.
     *
     * You can use the user-defined literals `_b`, `_kb`, `_mb`, and `_gb`.
     */
    class Bytes {
    public:
        // ReSharper disable once CppNonExplicitConvertingConstructor
        constexpr Bytes(rx_size const count) noexcept : byte_count(count){}

        constexpr Bytes(Bytes const& other) noexcept = default;
        constexpr Bytes& operator=(Bytes const& other) noexcept = default;

        /**
         * \return Value in bytes.
         */
        [[nodiscard]] constexpr rx_size b_count() const noexcept { return byte_count; }
        /**
         * \return Rounded value in kilobytes.
         */
        [[nodiscard]] constexpr rx_size k_count() const noexcept { return (byte_count + 512) / 1024; }
        /**
         * \return Rounded value in megabytes.
         */
        [[nodiscard]] constexpr rx_size m_count() const noexcept { return (byte_count + 524288) / 1048576; }
        /**
         * \return Rounded value in gigabytes.
         */
        [[nodiscard]] constexpr rx_size g_count() const noexcept { return (byte_count + 536870912) / 1073741824; }

#pragma region Comparison operators
        constexpr bool operator==(Bytes const& rhs) const noexcept { return byte_count == rhs.byte_count; }

        constexpr bool operator!=(Bytes const& rhs) const noexcept { return !(rhs == *this); }

        constexpr bool operator<(Bytes const& rhs) const noexcept { return byte_count < rhs.byte_count; }

        constexpr bool operator>(Bytes const& rhs) const noexcept { return rhs < *this; }

        constexpr bool operator<=(Bytes const& rhs) const noexcept { return !(rhs < *this); }

        constexpr bool operator>=(Bytes const& rhs) const noexcept { return !(*this < rhs); }
#pragma endregion

#pragma region Bytes arithmatic operators
        constexpr Bytes operator+(Bytes const addend) const noexcept { return Bytes(byte_count + addend.byte_count); }

        constexpr Bytes operator-(Bytes const subtrahend) const noexcept { return Bytes(byte_count - subtrahend.byte_count); }

        constexpr Bytes operator*(rx_size const multiplicand) const noexcept { return Bytes(byte_count * multiplicand); }

        constexpr Bytes operator/(rx_size const divisor) const noexcept { return Bytes(byte_count / divisor); }

        constexpr Bytes operator%(rx_size const divisor) const noexcept { return Bytes(byte_count % divisor); }

        constexpr Bytes& operator+=(Bytes const addend) noexcept {
            byte_count += addend.byte_count;
            return *this;
        }

        constexpr Bytes& operator-=(Bytes const subtrahend) noexcept {
            byte_count -= subtrahend.byte_count;
            return *this;
        }

        constexpr Bytes operator%(Bytes const divisor) const noexcept { return Bytes(byte_count % divisor.byte_count); }

#pragma endregion

#pragma region rx_size arithmatic operators
        constexpr Bytes operator+(rx_size const addend) const noexcept { return Bytes(byte_count + addend); }

        constexpr Bytes operator-(rx_size const subtrahend) const noexcept { return Bytes(byte_count - subtrahend); }

        constexpr Bytes& operator*=(rx_size const multiplicand) noexcept {
            byte_count *= multiplicand;
            return *this;
        }

        constexpr Bytes& operator/=(rx_size const divisor) noexcept {
            byte_count /= divisor;
            return *this;
        }

        constexpr Bytes& operator%=(rx_size const divisor) noexcept {
            byte_count %= divisor;
            return *this;
        }
#pragma endregion

#pragma region Bytes bitwise operators
        constexpr Bytes operator~() const noexcept { return Bytes(~byte_count); }

        constexpr Bytes operator&(Bytes const& rhs) const noexcept { return Bytes(byte_count & rhs.byte_count); }

        constexpr Bytes operator|(Bytes const& rhs) const noexcept { return Bytes(byte_count | rhs.byte_count); }
#pragma endregion
    private:
        rx_size byte_count;
    };

    /**
     * A class to represent a value in kilobytes. Unlike \ref Bytes::k_count(), converting from a \ref Bytes to a \ref KBytes is
     * lossless.
     */
    class KBytes : public Bytes {
    public:
        /**
         * Represents count * 1024 bytes.
         *
         * \param count Amount of kilobytes to represent.
         */
        constexpr explicit KBytes(rx_size const count) noexcept : Bytes(count * 1024){}
        /**
         * Losslessly copies from another \ref Bytes instance.
         *
         * \param b Instance to copy from.
         */
        constexpr explicit KBytes(Bytes const b) noexcept : Bytes(b){}
    };

    /**
     * A class to represent a value in megabytes. Unlike \ref Bytes::m_count(), converting from a \ref Bytes to a \ref MBytes is
     * lossless.
     */
    class MBytes : public Bytes {
    public:
        /**
         * Represents count * 1024^2 bytes.
         *
         * \param count Amount of megabytes to represent.
         */
        constexpr explicit MBytes(rx_size const count) noexcept : Bytes(count * 1048576){}
        /**
         * Losslessly copies from another \ref Bytes instance.
         *
         * \param b Instance to copy from.
         */
        constexpr explicit MBytes(Bytes const b) noexcept : Bytes(b){}
    };

    /**
     * A class to represent a value in gigabytes. Unlike \ref Bytes::g_count(), converting from a \ref Bytes to a \ref GBytes is
     * lossless.
     */
    class GBytes : public Bytes {
    public:
        /**
         * Represents count * 1024^3 bytes.
         *
         * \param count Amount of gigabytes to represent.
         */
        constexpr explicit GBytes(rx_size const count) noexcept : Bytes(count * 1073741824){}
        /**
         * Losslessly copies from another \ref Bytes instance.
         *
         * \param b Instance to copy from.
         */
        constexpr explicit GBytes(Bytes const b) noexcept : Bytes(b){}
    };

    constexpr Bytes operator*(rx_size const multiplicand, Bytes const lhs) noexcept { return lhs * multiplicand; }

    constexpr Bytes operator/(rx_size const dividend, Bytes const divisor) noexcept { return Bytes(dividend / divisor.b_count()); }

    /**
     * Print the amount of bytes within a \ref Bytes class. Prints as "XXb".
     *
     * \param os Stream to print on.
     * \param b Byte value to print.
     * \return Input stream.
     */
    std::ostream& operator<<(std::ostream& os, Bytes b);
    /**
     * Rounds and prints the amount of kilobytes within a \ref Bytes class. Prints as "XXkb"
     *
     * \param os Stream to print on.
     * \param b Byte value to print.
     * \return Input stream.
     */
    std::ostream& operator<<(std::ostream& os, KBytes b);
    /**
     * Rounds and prints the amount of megabytes within a \ref Bytes class. Prints as "XXkb"
     *
     * \param os Stream to print on.
     * \param b Byte value to print.
     * \return Input stream.
     */
    std::ostream& operator<<(std::ostream& os, MBytes b);
    /**
     * Rounds and prints the amount of gigabytes within a \ref Bytes class. Prints as "XXkb"
     *
     * \param os Stream to print on.
     * \param b Byte value to print.
     * \return Input stream.
     */
    std::ostream& operator<<(std::ostream& os, GBytes b);

    namespace operators {
        constexpr Bytes operator""_b(unsigned long long const value) { return Bytes(value); }
        constexpr Bytes operator""_kb(unsigned long long const value) { return KBytes(value); }
        constexpr Bytes operator""_mb(unsigned long long const value) { return MBytes(value); }
        constexpr Bytes operator""_gb(unsigned long long const value) { return GBytes(value); }
    } // namespace operators
} // namespace nova::mem
