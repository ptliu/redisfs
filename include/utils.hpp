#pragma once

namespace redisfs {

    /**
     * @brief SI prefixes for convenience.
     * 
     * @tparam T The type to use.
     */
    template<typename T>
    class Size {

        public:
        static constexpr T KILO = 1000;
        static constexpr T MEGA = KILO * KILO;
        static constexpr T GIGA = KILO * MEGA;
        static constexpr T TERA = KILO * GIGA;

        static constexpr T KIBI = 1024;
        static constexpr T MEBI = KIBI * KIBI;
        static constexpr T GIBI = KIBI * MEBI;
        static constexpr T TEBI = KIBI * GIBI;

    };

}