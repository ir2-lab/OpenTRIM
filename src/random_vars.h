#ifndef _RANDOM_VARS_H_
#define _RANDOM_VARS_H_

#include <random>
#include <array>
#include <cstdint>

/**
 * \defgroup RNG Random numbers
 * @brief Classes for generating random quantities needed in the Monte-Carlo simulation.
 *
 * @{
 *
 * @ingroup MC
 *
 * @}
 *
 *
 *
 */

/**
 * @brief Xoshiro256+ random number generator
 *
 * Adopted from the original by David Blackman and Sebastiano Vigna (vigna@acm.org) 2018
 *
 * License: http://creativecommons.org/publicdomain/zero/1.0/
 * 
 * Original implementation: http://prng.di.unimi.it/xoshiro256plus.c
 *
 * From the authors:
 * > This is xoshiro256+ 1.0, our best and fastest generator for floating-point
 * > numbers. We suggest to use its upper bits for floating-point
 * > generation, as it is slightly faster than xoshiro256++/xoshiro256**. It
 * > passes all tests we are aware of except for the lowest three bits,
 * > which might fail linearity tests (and just those), so if low linear
 * > complexity is not considered an issue (as it is usually the case) it
 * > can be used to generate 64-bit outputs, too.
 * > 
 * > We suggest to use a sign test to extract a random Boolean value, and
 * > right shifts to extract subsets of bits.
 * > 
 * > The state must be seeded so that it is not everywhere zero. If you have
 * > a 64-bit seed, we suggest to seed a splitmix64 generator and use its
 * > output to fill s. 
 *
 *  The C++ implementation here  qualifies as a std::uniform_random_bit_generator
 *  and can be used with std lib funtions.
 *
 * Properties:
 *   - Output: 64 bits
 *   - Period: 2^256 - 1
 *   - Footprint: 32 bytes
 *   - Version: 1.0
 * 
 *   @ingroup RNG
 */

class Xoshiro256Plus
{
    static constexpr std::uint64_t RotL(const std::uint64_t x, const int s) noexcept
    {
        return (x << s) | (x >> (64 - s));
    }

    /**
     * Computes Stafford variant 13 of the 64-bit mixing function for
     * MurmurHash3. This is a 64-bit hashing function with excellent avalanche
     * statistics.
     * http://zimbry.blogspot.com/2011/09/better-bit-mixing-improving-on.html
     *
     * <p> Note that if the argument {@code z} is 0, the result is 0.
     *
     * @param z any long value
     *
     * @return the result of hashing z
     */
    static constexpr std::uint64_t mixStafford13(std::uint64_t z) {
        z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
        z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
        return z ^ (z >> 31);
    }

public:

    using state_type	= std::array<std::uint64_t, 4>;
    using result_type	= std::uint64_t;

    /**
     * @brief Xoshiro256Plus default constructor
     *
     * The argument is used to initialize the state by calling seed().
     *
     * If no seed is given then a default seed (1234567890ULL) is used.
     *
     * Instances of @ref Xoshiro256Plus
     * created with the same seed in the same
     * program generate identical sequences of values.
     *
     * @param a_seed the seed used to initialize the state
     */
    explicit Xoshiro256Plus(std::uint64_t a_seed = DefaultSeed) noexcept
    { seed(a_seed); }

    explicit constexpr Xoshiro256Plus(state_type state) noexcept
        : m_state(state) {}

    Xoshiro256Plus(const Xoshiro256Plus& other) noexcept
        : m_state(other.m_state)
    {}

    /**
     * @brief Set the internal state by a seed s
     *
     * s is used to initialize a std::mt19937_64 object, which is
     * then used to produce the internal state data.
     *
     * @param s the seed used initialize the state
     */
    void randomize(result_type s)
    {
        std::mt19937_64 mt(s);

        for (auto& state : m_state)
        {
            state = static_cast<std::uint64_t>(mt());
        }
    }

    /**
     * @brief Set the internal state by a seed s
     *
     * Sets the internal state using the
     * specified value as seed.
     *
     * Instances of @ref Xoshiro256Plus
     *  created with the same seed generate identical sequences of values.
     *
     * @param s the initial seed
     */
    void seed(result_type s)
    {
        // Using a value with irregularly spaced 1-bits to xor the seed
        // argument tends to improve "pedestrian" seeds such as 0 or
        // other small integers.  We may as well use SILVER_RATIO_64.
        //
        // The x values are then filled in as if by a SplitMix PRNG with
        // GOLDEN_RATIO_64 as the gamma value and Stafford13 as the mixer.
        m_state[0] = mixStafford13(s ^= SILVER_RATIO_64);
        m_state[1] = mixStafford13(s += GOLDEN_RATIO_64);
        m_state[2] = mixStafford13(s += GOLDEN_RATIO_64);
        m_state[3] = mixStafford13(s + GOLDEN_RATIO_64);
    }

    /// Advances the state and returns the generated value
    constexpr result_type operator()() noexcept
    {
        const std::uint64_t result = m_state[0] + m_state[3];
        const std::uint64_t t = m_state[1] << 17;
        m_state[2] ^= m_state[0];
        m_state[3] ^= m_state[1];
        m_state[1] ^= m_state[2];
        m_state[0] ^= m_state[3];
        m_state[2] ^= t;
        m_state[3] = RotL(m_state[3], 45);
        return result;
    }

    /**
     * @brief Advances the state by 2^128 steps
     *
     * Can be used to generate 2^128 non-overlapping subsequences for parallel computations.
     */
    constexpr void jump() noexcept
    {
        constexpr std::uint64_t JUMP[] = {
            0x180ec6d33cfd0aba,
            0xd5a61266f0c9392c,
            0xa9582618e03fc9aa,
            0x39abdc4529b1661c
        };

        std::uint64_t s0 = 0;
        std::uint64_t s1 = 0;
        std::uint64_t s2 = 0;
        std::uint64_t s3 = 0;

        for (std::uint64_t jump : JUMP)
        {
            for (int b = 0; b < 64; ++b)
            {
                if (jump & UINT64_C(1) << b)
                {
                    s0 ^= m_state[0];
                    s1 ^= m_state[1];
                    s2 ^= m_state[2];
                    s3 ^= m_state[3];
                }
                operator()();
            }
        }

        m_state[0] = s0;
        m_state[1] = s1;
        m_state[2] = s2;
        m_state[3] = s3;
    }

    /**
     * @brief Advances the state by 2^192 steps
     *
     * It can be used to generate 2^64 starting points,
     * from each of which jump() will generate 2^64 non-overlapping
     * subsequences for parallel distributed computations.
     */
    constexpr void longJump() noexcept
    {
        constexpr std::uint64_t LONG_JUMP[] = {
            0x76e15d3efefdcbbf,
            0xc5004e441c522fb3,
            0x77710069854ee241,
            0x39109bb02acbe635
        };

        std::uint64_t s0 = 0;
        std::uint64_t s1 = 0;
        std::uint64_t s2 = 0;
        std::uint64_t s3 = 0;

        for (std::uint64_t jump : LONG_JUMP)
        {
            for (int b = 0; b < 64; ++b)
            {
                if (jump & UINT64_C(1) << b)
                {
                    s0 ^= m_state[0];
                    s1 ^= m_state[1];
                    s2 ^= m_state[2];
                    s3 ^= m_state[3];
                }
                operator()();
            }
        }

        m_state[0] = s0;
        m_state[1] = s1;
        m_state[2] = s2;
        m_state[3] = s3;
    }

    /// Smallest output value (0)
    [[nodiscard]]
    static constexpr result_type min() noexcept
    {
        return std::numeric_limits<result_type>::lowest();
    }

    /// Smallest output value (2^64 - 1)
    [[nodiscard]]
    static constexpr result_type max() noexcept
    {
        return std::numeric_limits<result_type>::max();
    }

    /// @brief Returns the internal state
    [[nodiscard]]
    constexpr state_type state() const noexcept
    {
        return m_state;
    }

    /// @brief  Set the internal state to a new value
    constexpr void state(const state_type state) noexcept
    {
        m_state = state;
    }

    /// Returns true if the two objects have equal states
    [[nodiscard]]
    friend bool operator ==(const Xoshiro256Plus& lhs, const Xoshiro256Plus& rhs) noexcept
    {
        return (lhs.m_state == rhs.m_state);
    }

    /// Return true if the two objects have unequal states
    [[nodiscard]]
    friend bool operator !=(const Xoshiro256Plus& lhs, const Xoshiro256Plus& rhs) noexcept
    {
        return (lhs.m_state != rhs.m_state);
    }

private:
    static constexpr result_type DefaultSeed = 1234567890ULL;

    /**
     * The first 64 bits of the golden ratio (1+sqrt(5))/2, forced to be odd.
     * Useful for producing good Weyl sequences or as an arbitrary nonzero odd
     * value.
     */
    static constexpr result_type GOLDEN_RATIO_64 = 0x9e3779b97f4a7c15ULL;

    /**
     * The first 64 bits of the silver ratio 1+sqrt(2), forced to be odd. Useful
     * for producing good Weyl sequences or as an arbitrary nonzero odd value.
     */
    static constexpr result_type SILVER_RATIO_64 = 0x6A09E667F3BCC909ULL;

    state_type m_state;
};

inline constexpr float toFloat(const std::uint32_t i) noexcept
{
    return (i >> 8) * 0x1.0p-24f;
}

inline constexpr float toFloat(const std::uint64_t i) noexcept
{
    return (i >> 40) * 0x1.0p-24f;
}

inline constexpr double toDouble(const std::uint64_t i) noexcept
{
    return (i >> 11) * 0x1.0p-53;
}

/**
 * @brief The random_vars class is used for generating random quantities needed in the simulation
 *
 * It is based on the xoshiro128+ random number generator.
 *
 * @ingroup RNG
 *
 */
class random_vars : public Xoshiro256Plus
{
    typedef Xoshiro256Plus rng_engine;

    std::normal_distribution<float> N_;

public:
    /// default constructor
    random_vars() : rng_engine()
    {}
    /// Quasi copy constructor, using an already defined engine
    explicit random_vars(rng_engine& e) : rng_engine(e.state())
    {}
    /// copy constructor
    random_vars(const random_vars& other)
        : Xoshiro256Plus(other)
    {}

    /// Single precision uniform random values in [0, 1)

    /**
     * @brief Return a random 32bit float in [0, 1)
     *
     * The function uses the upper 24 bits of the output
     * from xoshiro128+ divided by 2^24.
     *
     * This creates a 32bit float uniformly distributed in [0, 1)
     *
     * The value of 1 is never returned. The maximum value that can be returned
     * is 1.f - std::limits<float>::epsilon()/2
     *
     */
    float u01s_ropen() {
        return toFloat((*this)());
    }
    /// Same as u01ropen()
    float u01s() { return u01s_ropen(); }
    /// Return random value in (0, 1]
    float u01s_lopen() {
        return 1.f - u01s_ropen();
    }
    /// Return random value in (0, 1)
    float u01s_open() {
        float u;
        do u = u01s_ropen(); while(u==0.f);
        return u;
    }

    /// Double precision uniform random values in [0, 1)

    /**
     * @brief Return a random 64bit float in [0, 1)
     *
     * The function uses the upper 53 bits of the output
     * from xoshiro128+ divided by 2^53.
     *
     * This creates a 64bit float uniformly distributed in [0, 1)
     *
     * The value of 1 is never returned. The maximum value that can be returned
     * is 1.f - std::limits<double>::epsilon()/2
     *
     */
    double u01d_ropen() {
        return toDouble((*this)());
    }
    /// Same as u01d_ropen()
    float u01d() { return u01d_ropen(); }
    /// Return double precision random value in (0, 1]
    float u01d_lopen() {
        return 1.0 - u01d_ropen();
    }
    /// Return random value in (0, 1)
    float u01d_open() {
        double u;
        do u = u01d_ropen(); while(u==0.0);
        return u;
    }

    /// Return a random value distributed as N(0,1)
    float normal()
    {
        return N_(*this);
    }
    /**
     * @brief Generate a random azimuthal direction
     *
     * Equivalent to:
     * @code{.cpp}
     * phi = 2*pi*u; nx = sin(phi); ny = cos(phi);
     * @endcode
     *
     * The employed algorithm samples (nx, ny) without calling
     * trigonometric functions.
     *
     * @param nx sin(phi)
     * @param ny cos(phi)
     */
    void random_azimuth_dir(float& nx, float& ny)
    {
        float s1,s2,r2;
        do
        {
            nx = 2.f*u01s() - 1.f;
            ny = u01s();
            s1 = nx*nx;
            s2 = ny*ny;
            r2 = s1 + s2;
        } while (r2>1.f || r2==0.f);
        ny = 2*nx*ny/r2;
        nx = (s1-s2)/r2;
    }
};

#endif // RANDOM_VARS_H
