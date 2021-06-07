#pragma once

#include <stdexcept>
#include <string>

namespace redisfs {

    /**
     * @brief Base exception for redisfs errors.
     */
    class RedisFSError : std::runtime_error {
        public:
            explicit RedisFSError( const std::string & what );
    };


    class CorruptFilesystemError : RedisFSError {
        public:
            explicit CorruptFilesystemError( const std::string & what );
    };

}