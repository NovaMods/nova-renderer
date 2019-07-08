#pragma once

#include <EABase/config/eacompilertraits.h>
#include <cstdlib>
#include <iosfwd>

namespace bvestl {
	namespace polyalloc {
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
			constexpr explicit EA_FORCE_INLINE Bytes(std::size_t const count) noexcept : byte_count(count) {};
			constexpr EA_FORCE_INLINE Bytes(Bytes const& other) noexcept = default;
			constexpr Bytes& operator=(Bytes const& other) noexcept = default;

			/**
			 * \return Value in bytes.
			 */
			constexpr EA_FORCE_INLINE std::size_t b_count() const noexcept { return byte_count; }
			/**
			 * \return Rounded value in kilobytes.
			 */
			constexpr EA_FORCE_INLINE std::size_t k_count() const noexcept { return (byte_count + 512) / 1024; }
			/**
			 * \return Rounded value in megabytes.
			 */
			constexpr EA_FORCE_INLINE std::size_t m_count() const noexcept { return (byte_count + 524288) / 1048576; }
			/**
			 * \return Rounded value in gigabytes.
			 */
			constexpr EA_FORCE_INLINE std::size_t g_count() const noexcept { return (byte_count + 536870912) / 1073741824; }

#pragma region Comparison operators
			constexpr EA_FORCE_INLINE bool operator==(Bytes const& rhs) const noexcept { return byte_count == rhs.byte_count; }

			constexpr EA_FORCE_INLINE bool operator!=(Bytes const& rhs) const noexcept { return !(rhs == *this); }

			constexpr EA_FORCE_INLINE bool operator<(Bytes const& rhs) const noexcept { return byte_count < rhs.byte_count; }

			constexpr EA_FORCE_INLINE bool operator>(Bytes const& rhs) const noexcept { return rhs < *this; }

			constexpr EA_FORCE_INLINE bool operator<=(Bytes const& rhs) const noexcept { return !(rhs < *this); }

			constexpr EA_FORCE_INLINE bool operator>=(Bytes const& rhs) const noexcept { return !(*this < rhs); }
#pragma endregion

#pragma region Bytes arithmatic operators
			constexpr EA_FORCE_INLINE Bytes operator+(Bytes const addend) const noexcept {
				return Bytes(byte_count + addend.byte_count);
			}

			constexpr EA_FORCE_INLINE Bytes operator-(Bytes const subtrahend) const noexcept {
				return Bytes(byte_count - subtrahend.byte_count);
			}

			constexpr EA_FORCE_INLINE Bytes operator*(std::size_t const multiplicand) const noexcept {
				return Bytes(byte_count * multiplicand);
			}

			constexpr EA_FORCE_INLINE Bytes operator/(std::size_t const divisor) const noexcept {
				return Bytes(byte_count / divisor);
			}

			constexpr EA_FORCE_INLINE Bytes& operator+=(Bytes const addend) noexcept {
				byte_count += addend.byte_count;
				return *this;
			}

			constexpr EA_FORCE_INLINE Bytes& operator-=(Bytes const subtrahend) noexcept {
				byte_count -= subtrahend.byte_count;
				return *this;
			}
#pragma endregion

#pragma region std::size_t arithmatic operators
			constexpr EA_FORCE_INLINE Bytes operator+(std::size_t const addend) const noexcept {
				return Bytes(byte_count + addend);
			}

			constexpr EA_FORCE_INLINE Bytes operator-(std::size_t const subtrahend) const noexcept {
				return Bytes(byte_count - subtrahend);
			}

			constexpr EA_FORCE_INLINE Bytes& operator*=(std::size_t const multiplicand) noexcept {
				byte_count *= multiplicand;
				return *this;
			}

			constexpr EA_FORCE_INLINE Bytes& operator/=(std::size_t const divisor) noexcept {
				byte_count /= divisor;
				return *this;
			}
#pragma endregion

#pragma region Bytes bitwise operators
			constexpr EA_FORCE_INLINE Bytes operator~() const noexcept { return Bytes(~byte_count); }

			constexpr EA_FORCE_INLINE Bytes operator&(Bytes const& rhs) const noexcept {
				return Bytes(byte_count & rhs.byte_count);
			}

			constexpr EA_FORCE_INLINE Bytes operator|(Bytes const& rhs) const noexcept {
				return Bytes(byte_count | rhs.byte_count);
			}
#pragma endregion
		private:
			std::size_t byte_count;
		};

		/**
		 * A class to represent a value in kilobytes. Unlike \ref Bytes::k_count(), converting from a \ref Bytes to a \ref KBytes is lossless.
		 */
		class KBytes : public Bytes {
		public:
			/**
			 * Represents count * 1024 bytes.
			 *
			 * \param count Amount of kilobytes to represent.
			 */
			constexpr explicit EA_FORCE_INLINE KBytes(std::size_t const count) noexcept : Bytes(count * 1024) {};
			/**
			 * Losslessly copies from another \ref Bytes instance.
			 *
			 * \param b Instance to copy from.
			 */
			constexpr explicit EA_FORCE_INLINE KBytes(Bytes const b) noexcept : Bytes(b) {};
		};

		/**
		 * A class to represent a value in megabytes. Unlike \ref Bytes::m_count(), converting from a \ref Bytes to a \ref MBytes is lossless.
		 */
		class MBytes : public Bytes {
		public:
			/**
			 * Represents count * 1024^2 bytes.
			 *
			 * \param count Amount of megabytes to represent.
			 */
			constexpr explicit EA_FORCE_INLINE MBytes(std::size_t const count) noexcept : Bytes(count * 1048576) {};
			/**
			 * Losslessly copies from another \ref Bytes instance.
			 *
			 * \param b Instance to copy from.
			 */
			constexpr explicit EA_FORCE_INLINE MBytes(Bytes const b) noexcept : Bytes(b) {};
		};

		/**
		 * A class to represent a value in gigabytes. Unlike \ref Bytes::g_count(), converting from a \ref Bytes to a \ref GBytes is lossless.
		 */
		class GBytes : public Bytes {
		public:
			/**
			 * Represents count * 1024^3 bytes.
			 *
			 * \param count Amount of gigabytes to represent.
			 */
			constexpr explicit EA_FORCE_INLINE GBytes(std::size_t const count) noexcept : Bytes(count * 1073741824) {};
			/**
			 * Losslessly copies from another \ref Bytes instance.
			 *
			 * \param b Instance to copy from.
			 */
			constexpr explicit EA_FORCE_INLINE GBytes(Bytes const b) noexcept : Bytes(b) {};
		};

		constexpr EA_FORCE_INLINE Bytes operator*(std::size_t const multiplicand, Bytes const lhs) noexcept {
			return lhs * multiplicand;
		}

		constexpr EA_FORCE_INLINE Bytes operator/(std::size_t const dividend, Bytes const divisor) noexcept {
			return Bytes(dividend / divisor.b_count());
		}

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
			constexpr EA_FORCE_INLINE Bytes operator""_b(unsigned long long const value) { return Bytes(value); }
			constexpr EA_FORCE_INLINE Bytes operator""_kb(unsigned long long const value) { return KBytes(value); }
			constexpr EA_FORCE_INLINE Bytes operator""_mb(unsigned long long const value) { return MBytes(value); }
			constexpr EA_FORCE_INLINE Bytes operator""_gb(unsigned long long const value) { return GBytes(value); }
		} // namespace operators
	}
}
